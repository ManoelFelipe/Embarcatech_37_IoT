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
#include "mqtt_lwip.h"

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
    // Inicializa o barramento I2C 'i2c1' com uma velocidade de 100 kHz.
    i2c_init(i2c1, 100 * 1000);
    // Associa os pinos GPIO 2 e 3 às funções de SDA (Dados) e SCL (Clock) do I2C.
    gpio_set_function(2, GPIO_FUNC_I2C); // SDA
    gpio_set_function(3, GPIO_FUNC_I2C); // SCL
    // Habilita os resistores de pull-up internos nos pinos do I2C.
    // Isso é necessário para manter as linhas em estado alto quando não estão sendo usadas.
    gpio_pull_up(2);
    gpio_pull_up(3);
    printf("Barramento I2C inicializado nos pinos 2 (SDA) e 3 (SCL).\n");

    // --- Inicialização do Sensor ---
    // Chama a função de inicialização da biblioteca do sensor BH1750.
    bh1750_iniciar();
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
 * @brief Implementação do ciclo de leitura e publicação.
 */
void processar_ciclo_operacional() {
    // Buffers locais para armazenar o tópico e o payload da mensagem MQTT.
    char topico_completo[128];
    char payload_lux[20];

    // Realiza a leitura da luminosidade a partir do sensor BH1750.
    float lux = bh1750_ler_lux();
    printf("Luminosidade: %.2f Lux\n", lux);

    // Monta a string do tópico completo de forma segura para evitar estouro de buffer.
    // Ex: "Sensores/dados/Sensor_Luz"
    snprintf(topico_completo, sizeof(topico_completo), "%s/%s", DEVICE_ID, TOPICO_PUBLICACAO_LUZ);
    
    // Converte o valor de ponto flutuante (float) para uma string.
    // Ex: 150.75
    snprintf(payload_lux, sizeof(payload_lux), "%.2f", lux);

    // Chama a função da biblioteca MQTT para publicar a mensagem no broker.
    publicar_mensagem_mqtt(topico_completo, payload_lux);
}

/**
 * @brief Implementação da lógica de reconexão MQTT.
 */
void gerenciar_reconexao_mqtt() {
    printf("[AVISO] Cliente MQTT desconectado. Tentando reconectar...\n");
    // Simplesmente chama a função de inicialização novamente para tentar restabelecer a conexão.
    iniciar_mqtt_cliente(MQTT_BROKER_IP, MQTT_BROKER_PORT, DEVICE_ID);
}