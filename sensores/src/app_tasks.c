/**
 * @file app_tasks.c
 * @version 1.1
 * @date 14/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Implementação das tarefas principais da aplicação para o Pico W.
 *
 * Este arquivo contém a lógica de implementação para todas as funções
 * declaradas em `app_tasks.h`. As responsabilidades são divididas em
 * funções modulares para melhorar a legibilidade e a manutenção do código.
 */

// Inclui o próprio cabeçalho para garantir consistência com os protótipos.
#include "app_tasks.h"

// Includes de bibliotecas padrão e do SDK do Pico
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"

// Includes dos outros módulos do projeto
#include "configura_geral.h"
#include "bh1750.h"
#include "aht10.h"
#include "mqtt_lwip.h"

// Variável global para guardar os dados do AHT10
static aht10_data_t aht10_dados;

/**
 * @brief Implementação da configuração da comunicação serial.
 */
void configurar_serial() {
    // Inicializa todos os periféricos de I/O padrão (USB, UART).
    stdio_init_all();

    // Pausa a execução até que o PC se conecte à porta serial USB.
    // Isso é vital para não perder as primeiras mensagens de log durante a inicialização.
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Projeto Aquisição de dados Sensor\n");
}

/**
 * @brief Implementação da conexão Wi-Fi.
 */
bool conectar_wifi() {
    printf("Inicializando hardware e conexões...\n");

    // Inicializa o chip CYW43439, responsável pelo Wi-Fi e Bluetooth.
    if (cyw43_arch_init()) {
        printf("ERRO: Falha ao inicializar Wi-Fi\n");
        return false;
    }
    // Habilita o modo "Station" (STA), que permite ao Pico se conectar a um roteador.
    cyw43_arch_enable_sta_mode();

    printf("Conectando à rede Wi-Fi '%s'...\n", WIFI_SSID);
    // Tenta conectar à rede com um tempo limite de 30 segundos.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi\n");
        return false;
    }
    printf("Conectado com sucesso ao Wi-Fi!\n");
    return true;
}

/**
 * @brief Implementação da configuração dos periféricos.
 */
void configurar_perifericos() {
    printf("Inicializando periféricos...\n");
    
    // --- Configuração do I2C ---

     // --- CONFIGURAÇÃO DO I2C0 PARA O AHT10 ---
    i2c_init(I2C0_PORT, 100 * 1000);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);
    printf("Barramento I2C0 inicializado nos pinos %d (SDA) e %d (SCL).\n", I2C0_SDA_PIN, I2C0_SCL_PIN);

    // Inicialização do Sensor AHT10
    if (aht10_init(I2C0_PORT)) {
        printf("Sensor de umidade/temperatura AHT10 pronto para uso.\n");
    } else {
        printf("ERRO: Falha ao inicializar o sensor AHT10.\n");
    }


    // Inicializa o barramento I2C 'i2c1' com uma velocidade de 100 kHz.
    i2c_init(I2C1_PORT, 100 * 1000); // Usando a constante de configura_geral.h
    // Associa os pinos GPIO 2 e 3 às funções de SDA (Dados) e SCL (Clock) do I2C.
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    // Habilita os resistores de pull-up internos nos pinos do I2C.
    // Isso é necessário para manter as linhas em estado alto quando não estão sendo usadas.
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    printf("Barramento I2C inicializado nos pinos 2 (SDA) e 3 (SCL).\n");

    // --- Inicialização do Sensor ---
    // Chama a função de inicialização da biblioteca do sensor BH1750.
    bh1750_iniciar(I2C1_PORT);
    printf("Sensor de luminosidade BH1750 pronto para uso.\n");

    
}

/**
 * @brief Implementação da conexão inicial ao broker MQTT.
 */
void conectar_mqtt_inicial() {
    printf("Inicializando módulo MQTT...\n");
    // Inicia o cliente MQTT, passando as configurações definidas em `configura_geral.h`.
    iniciar_mqtt_cliente(MQTT_BROKER_IP, MQTT_BROKER_PORT, DEVICE_ID);
    printf("Cliente MQTT iniciado. Aguardando conexão com o broker...\n");

    // Loop de espera simples: tenta por 10 segundos (20 * 500ms).
    for (int i = 0; i < 20 && !cliente_mqtt_esta_conectado(); i++) {
        sleep_ms(500);
    }

    if (cliente_mqtt_esta_conectado()) {
        printf("Conexão MQTT estabelecida com sucesso!\n\n");
    } else {
        printf("[AVISO] Não foi possível conectar ao broker MQTT inicialmente. A reconexão será tentada no loop principal.\n\n");
    }
}

/**
 * @brief 3. IMPLEMENTAÇÃO DO CICLO COM LEITURA COMBINADA E PUBLICAÇÃO JSON.
 */
void processar_ciclo_operacional() {
    // Buffers para o tópico e o payload JSON.
    char topico_json[128];
    char payload_json[256]; // Aumentar buffer para conter todos os dados

    // --- Leitura dos Sensores ---
    
    // Leitura do AHT10 (Temperatura e Umidade)
    if (aht10_read_data(I2C0_PORT, &aht10_dados)) {
        printf("AHT10 -> Temperatura: %.2f C, Umidade: %.2f %%\n", aht10_dados.temperature, aht10_dados.humidity);
    } else {
        printf("Falha na leitura do AHT10. Usando valores padrão.\n");
        // Atribuir valores padrão em caso de falha para não quebrar o JSON
        aht10_dados.temperature = 0.0;
        aht10_dados.humidity = 0.0;
    }
    
    // Leitura do BH1750 (Luminosidade)
    float lux = bh1750_ler_lux(I2C1_PORT);
    printf("BH1750 -> Luminosidade: %.2f Lux\n", lux);

    // --- Montagem do Payload JSON ---
    // Formata os dados dos sensores em uma única string JSON.
    snprintf(payload_json, sizeof(payload_json),
             "{\"temperatura\":%.2f, \"umidade\":%.2f, \"luminosidade\":%.2f}",
             aht10_dados.temperature, aht10_dados.humidity, lux);

    // --- Publicação via MQTT ---
    
    // Monta o nome completo do tópico para a publicação JSON.
    snprintf(topico_json, sizeof(topico_json), "%s/%s", DEVICE_ID, TOPICO_PUBLICACAO_JSON);

    printf("Publicando em '%s': %s\n", topico_json, payload_json);
    publicar_mensagem_mqtt(topico_json, payload_json);
}

/**
 * @brief Implementação da lógica de reconexão MQTT.
 */
void gerenciar_reconexao_mqtt() {
    printf("[AVISO] Cliente MQTT desconectado. Tentando reconectar...\n");
    // Simplesmente chama a função de inicialização novamente para tentar restabelecer a conexão.
    iniciar_mqtt_cliente(MQTT_BROKER_IP, MQTT_BROKER_PORT, DEVICE_ID);
}