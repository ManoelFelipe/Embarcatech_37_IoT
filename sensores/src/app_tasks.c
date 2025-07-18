/**
 * @file app_tasks.c
 * @version 1.0
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Implementação das tarefas principais da aplicação para o Pico W.
 *
 * @details
 * Este arquivo contém a lógica de implementação para todas as funções
 * declaradas em `app_tasks.h`. As responsabilidades são divididas em
 * funções modulares para melhorar a legibilidade, a manutenibilidade e o
 * reuso do código. Aqui são tratados os detalhes de baixo nível da
 * interação com o hardware e os protocolos de comunicação.
 */

// Inclui o próprio cabeçalho para garantir que a implementação esteja
// consistente com os protótipos de função declarados. É uma boa prática de programação.
#include "app_tasks.h"

// Includes de bibliotecas padrão e do SDK do Pico
#include <stdio.h>
#include "pico/stdlib.h"      // Funções padrão do Pico (GPIO, stdio, etc.)
#include "pico/cyw43_arch.h"  // Funções específicas para o chip Wi-Fi CYW43439
#include "hardware/i2c.h"     // Funções para controle do periférico I2C

// Includes dos outros módulos do projeto
#include "configura_geral.h" // Acesso a todas as constantes de configuração (credenciais, pinos)
#include "bh1750.h"          // Funções específicas para o sensor de luminosidade BH1750
#include "aht10.h"           // Funções específicas para o sensor de temperatura/umidade AHT10
#include "mqtt_lwip.h"       // Funções para a comunicação com o broker MQTT

// Variável global estática para armazenar os dados lidos do sensor AHT10.
// O qualificador 'static' restringe a visibilidade desta variável apenas a este arquivo,
// encapsulando o estado e evitando acessos indesejados de outros módulos.
static aht10_data_t aht10_dados;

/**
 * @brief Implementação da configuração da comunicação serial.
 * @details Inicializa o sistema de I/O padrão do Pico, que geralmente é a porta serial USB.
 * Em seguida, entra em um loop de espera que bloqueia a execução até que um
 * computador se conecte à porta serial. Isso é crucial para depuração, pois
 * garante que nenhuma mensagem de log emitida durante a inicialização seja perdida.
 */
void configurar_serial() {
    // Inicializa todos os periféricos de I/O padrão (que incluem USB-CDC para serial).
    stdio_init_all();

    // Pausa a execução até que o PC estabeleça a conexão com a porta serial USB.
    // Sem isso, as primeiras chamadas `printf` poderiam ocorrer antes do monitor serial
    // estar pronto para recebê-las.
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Projeto Aquisição de dados Sensor\n");
}

/**
 * @brief Implementação da conexão Wi-Fi.
 * @details Esta função orquestra a inicialização do chip Wi-Fi (CYW43439)
 * e a tentativa de conexão com a rede definida em `configura_geral.h`.
 * @return true se a conexão for bem-sucedida, false caso contrário.
 */
bool conectar_wifi() {
    printf("Inicializando hardware e conexões...\n");

    // Inicializa o módulo CYW43, que controla o hardware de rádio (Wi-Fi e Bluetooth).
    if (cyw43_arch_init()) {
        printf("ERRO: Falha ao inicializar Wi-Fi\n");
        return false;
    }
    // Configura o chip para operar no modo "Station" (STA), ou seja,
    // como um cliente que se conecta a um ponto de acesso (roteador).
    cyw43_arch_enable_sta_mode();

    printf("Conectando à rede Wi-Fi '%s'...\n", WIFI_SSID);
    // Tenta se conectar à rede usando o SSID, a senha e o tipo de autenticação definidos.
    // A função aguarda até 30 segundos (30000 ms) pela conexão.
    // Se o tempo esgotar ou ocorrer outro erro, a função retorna um valor diferente de zero.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("ERRO: Falha ao conectar ao Wi-Fi\n");
        return false;
    }
    printf("Conectado com sucesso ao Wi-Fi!\n");
    return true;
}

/**
 * @brief Implementação da configuração dos periféricos de hardware.
 * @details Inicializa os barramentos I2C e os sensores conectados a eles.
 * Este projeto utiliza duas portas I2C distintas para evitar conflitos de
 * endereço ou para organizar melhor o hardware, uma para cada sensor.
 */
void configurar_perifericos() {
    printf("Inicializando periféricos...\n");

    // --- Configuração do I2C0 para o sensor AHT10 ---
    // Inicializa o periférico I2C0 com uma velocidade de 100 kHz (padrão).
    i2c_init(I2C0_PORT, 100 * 1000);
    // Define a função dos pinos GPIO especificados em `configura_geral.h` para I2C.
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    // Habilita os resistores de pull-up internos para os pinos I2C.
    // Essencial para manter as linhas em nível lógico alto quando o barramento está ocioso.
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);
    printf("Barramento I2C0 inicializado nos pinos %d (SDA) e %d (SCL).\n", I2C0_SDA_PIN, I2C0_SCL_PIN);

    // Inicialização do Sensor AHT10 no barramento I2C0.
    if (aht10_init(I2C0_PORT)) {
        printf("Sensor de umidade/temperatura AHT10 pronto para uso.\n");
    } else {
        printf("ERRO: Falha ao inicializar o sensor AHT10.\n");
    }


    // --- Configuração do I2C1 para o sensor BH1750 ---
    // Inicializa o periférico I2C1, também a 100 kHz.
    i2c_init(I2C1_PORT, 100 * 1000);
    // Associa os pinos GPIO definidos às funções de SDA (Dados) e SCL (Clock) do I2C1.
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    // Habilita os pull-ups também para este barramento.
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    printf("Barramento I2C1 inicializado nos pinos %d (SDA) e %d (SCL).\n", I2C1_SDA_PIN, I2C1_SCL_PIN);

    // Inicialização do Sensor BH1750 no barramento I2C1.
    bh1750_iniciar(I2C1_PORT);
    printf("Sensor de luminosidade BH1750 pronto para uso.\n");
}

/**
 * @brief Implementação da conexão inicial ao broker MQTT.
 * @details Inicia o cliente MQTT com as credenciais do servidor e aguarda um
 * curto período para que a conexão seja estabelecida. A função não bloqueia
 * indefinidamente, permitindo que o programa continue e tente a reconexão
 * mais tarde, caso a primeira tentativa falhe.
 */
void conectar_mqtt_inicial() {
    printf("Inicializando módulo MQTT...\n");
    // Chama a função da biblioteca MQTT para configurar e iniciar o processo de conexão.
    // Passa o IP do broker, a porta e um ID de dispositivo, todos de `configura_geral.h`.
    iniciar_mqtt_cliente(MQTT_BROKER_IP, MQTT_BROKER_PORT, DEVICE_ID);
    printf("Cliente MQTT iniciado. Aguardando conexão com o broker...\n");

    // Loop de espera simples: tenta por no máximo 10 segundos (20 iterações * 500ms).
    // Verifica o status da conexão a cada meio segundo.
    for (int i = 0; i < 20 && !cliente_mqtt_esta_conectado(); i++) {
        sleep_ms(500);
    }

    // Informa o resultado da tentativa de conexão inicial.
    if (cliente_mqtt_esta_conectado()) {
        printf("Conexão MQTT estabelecida com sucesso!\n\n");
    } else {
        printf("[AVISO] Não foi possível conectar ao broker MQTT inicialmente. A reconexão será tentada no loop principal.\n\n");
    }
}

/**
 * @brief Realiza um ciclo de operação: lê todos os sensores, formata os dados
 * em JSON e os publica via MQTT.
 * @details Esta é a principal função de trabalho do dispositivo. Ela consolida os dados de
 * múltiplos sensores em uma única mensagem estruturada (JSON) para otimizar a
 * comunicação e facilitar o processamento no lado do servidor.
 */
void processar_ciclo_operacional() {
    // Buffers (arrays de caracteres) para construir a string do tópico e do payload.
    char topico_json[128];
    char payload_json[256]; // Buffer aumentado para garantir espaço para todos os dados.

    // --- Leitura dos Sensores ---

    // Leitura do AHT10 (Temperatura e Umidade).
    // Os dados lidos são armazenados na variável global estática 'aht10_dados'.
    if (aht10_read_data(I2C0_PORT, &aht10_dados)) {
        printf("AHT10 -> Temperatura: %.2f C, Umidade: %.2f %%\n", aht10_dados.temperature, aht10_dados.humidity);
    } else {
        printf("Falha na leitura do AHT10. Usando valores padrão.\n");
        // Em caso de falha na leitura, atribui valores padrão (0.0) para que
        // a estrutura do JSON não seja corrompida e a aplicação continue funcionando.
        aht10_dados.temperature = 0.0;
        aht10_dados.humidity = 0.0;
    }

    // Leitura do BH1750 (Luminosidade).
    float lux = bh1750_ler_lux(I2C1_PORT);
    printf("BH1750 -> Luminosidade: %.2f Lux\n", lux);

    // --- Montagem do Payload JSON ---
    // A função `snprintf` é usada para formatar os dados dos sensores em uma
    // única string JSON de forma segura, evitando estouro de buffer.
    snprintf(payload_json, sizeof(payload_json),
             "{\"temperatura\":%.2f, \"umidade\":%.2f, \"luminosidade\":%.2f}",
             aht10_dados.temperature, aht10_dados.humidity, lux);

    // --- Publicação via MQTT ---

    // Monta o nome completo do tópico para a publicação, combinando o ID do dispositivo
    // com o sub-tópico definido em `configura_geral.h`. Ex: "Sensores/dados/json".
    snprintf(topico_json, sizeof(topico_json), "%s/%s", DEVICE_ID, TOPICO_PUBLICACAO_JSON);

    printf("Publicando em '%s': %s\n", topico_json, payload_json);
    // Chama a função da biblioteca MQTT para enviar a mensagem (payload) para o tópico especificado.
    publicar_mensagem_mqtt(topico_json, payload_json);
}

/**
 * @brief Implementação da lógica de reconexão MQTT.
 * @details Esta função é chamada quando o loop principal detecta que a conexão MQTT
 * foi perdida. A estratégia atual é simples: tentar reiniciar o cliente MQTT
 * do zero. Em sistemas mais complexos, poderia haver lógicas mais avançadas,
 * como backoff exponencial (aumentar o tempo de espera a cada falha).
 */
void gerenciar_reconexao_mqtt() {
    printf("[AVISO] Cliente MQTT desconectado. Tentando reconectar...\n");
    // Simplesmente chama a função de inicialização novamente para tentar
    // restabelecer a conexão com o broker.
    iniciar_mqtt_cliente(MQTT_BROKER_IP, MQTT_BROKER_PORT, DEVICE_ID);
}