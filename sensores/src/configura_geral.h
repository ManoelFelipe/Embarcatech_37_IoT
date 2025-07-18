/**
 * @file configura_geral.h
 * @version 1.0
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Centraliza todas as configurações e credenciais do Projeto Aquisição de Dados.
 *
 * @details
 * Este arquivo de cabeçalho funciona como um painel de controle para o projeto.
 * Ele contém todas as definições que são específicas do ambiente de implantação,
 * como credenciais de rede (Wi-Fi), endereços de servidor (MQTT Broker),
 * identificadores de dispositivo e configurações de hardware (pinos GPIO).
 *
 * O objetivo principal é isolar estas informações "voláteis" do código fonte principal
 * (`main.c`, `app_tasks.c`), facilitando a configuração do projeto para diferentes
 * cenários (ex: mudar de uma rede Wi-Fi de desenvolvimento para uma de produção)
 * sem a necessidade de alterar a lógica da aplicação.
 *
 * @warning ESTE ARQUIVO CONTÉM INFORMAÇÕES SENSÍVEIS (CREDENCIAS DE REDE).
 * Nunca envie este arquivo com credenciais reais para repositórios públicos
 * como o GitHub. Para projetos sérios, a melhor prática é usar um arquivo de
 * configuração local ignorado pelo sistema de controle de versão (ex: via `.gitignore`)
 * ou utilizar mecanismos de provisionamento mais seguros.
 */

#ifndef CONFIGURA_GERAL_H
#define CONFIGURA_GERAL_H

// =================================================================================
// --- Configurações da Rede Wi-Fi ---
// =================================================================================
/**
 * @brief SSID (Service Set Identifier) ou nome da sua rede Wi-Fi.
 * @details Substitua o valor entre aspas pelo nome exato da rede sem fio
 * à qual o Pico W deve se conectar.
 */
#define WIFI_SSID "RENASCENCA_Cozinha_multilaser_"
//#define WIFI_SSID "Manoel_A50"
//#define WIFI_SSID "MARAMAKER"

/**
 * @brief Senha da sua rede Wi-Fi.
 * @details Substitua o valor entre aspas pela senha correspondente ao SSID definido acima.
 */
#define WIFI_PASSWORD "12345678"
//#define WIFI_PASSWORD "manel86rj"
//#define WIFI_PASSWORD "m@r@m@ker"

// =================================================================================
// --- Configurações do Broker MQTT ---
// =================================================================================
/**
 * @brief Endereço IP do servidor (broker) MQTT na sua rede local.
 * @details O broker MQTT é o servidor central que atua como intermediário,
 * recebendo as mensagens publicadas pelos sensores e as distribuindo para
 * todos os clientes inscritos nos tópicos correspondentes (ex: um dashboard).
 * Você deve substituir este valor pelo endereço IP do seu broker na sua rede.
 */
#define MQTT_BROKER_IP   "192.168.1.104"

/**
 * @brief Porta de comunicação do broker MQTT.
 * @details A porta padrão para comunicação MQTT não criptografada é 1883.
 * A porta 4004, usada aqui, é uma configuração personalizada do seu broker local.
 * Verifique a configuração do seu servidor MQTT para saber qual porta utilizar.
 */
#define MQTT_BROKER_PORT 4004

// =================================================================================
// --- Identificação do Dispositivo ---
// =================================================================================
/**
 * @brief ID único para este dispositivo.
 * @details Este identificador é usado como parte do tópico MQTT para diferenciar
 * a origem dos dados. Isso é crucial quando múltiplos dispositivos (ex: vários
 * "Pico Sensores") estão publicando dados no mesmo broker.
 * Permite que os clientes saibam exatamente qual dispositivo enviou a mensagem.
 */
#define DEVICE_ID "Sensores"

// =================================================================================
// --- Definições de Tópicos MQTT ---
// =================================================================================
/**
 * @brief Sub-tópico onde os dados consolidados em formato JSON serão publicados.
 * @details O tópico final será composto pelo DEVICE_ID e este valor, formando
 * algo como "Sensores/dados/json". Usar uma estrutura de tópicos hierárquica
 * (com barras) é uma boa prática em MQTT para organizar as mensagens em canais
 * temáticos dentro do broker, facilitando a filtragem e o gerenciamento.
 */
#define TOPICO_PUBLICACAO_JSON "dados/json"

// =================================================================================
// --- Configurações de Hardware (Pinos I2C) ---
// =================================================================================
/**
 * @brief Mapeamento de pinos para os periféricos I2C.
 * @details Esta seção define quais pinos do Pico W serão usados para a comunicação
 * I2C (Inter-Integrated Circuit), um protocolo comum para conectar sensores.
 * O Pico W possui múltiplos barramentos I2C (i2c0, i2c1), permitindo a conexão
 * de vários dispositivos, mesmo que tenham o mesmo endereço.
 */

// --- Configuração da Porta I2C 0 para o sensor AHT10 (Temperatura e Umidade) ---
#define I2C0_PORT i2c0       ///< Identificador do periférico I2C0.
#define I2C0_SDA_PIN 0       ///< Pino GPIO_0 será usado como SDA (Serial Data) para o I2C0.
#define I2C0_SCL_PIN 1       ///< Pino GPIO_1 será usado como SCL (Serial Clock) para o I2C0.

// --- Configuração da Porta I2C 1 para o sensor BH1750 (Luminosidade) ---
#define I2C1_PORT i2c1       ///< Identificador do periférico I2C1.
#define I2C1_SDA_PIN 2       ///< Pino GPIO_2 será usado como SDA (Serial Data) para o I2C1.
#define I2C1_SCL_PIN 3       ///< Pino GPIO_3 será usado como SCL (Serial Clock) para o I2C1.

#endif // CONFIGURA_GERAL_H