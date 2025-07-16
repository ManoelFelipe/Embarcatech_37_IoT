/**
 * @file configura_geral.h
 * @version 1.1
 * @date 14/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Centraliza todas as configurações e credenciais do Projeto Aquisição de Dados.
 *
 * Este arquivo de cabeçalho contém todas as definições que são específicas do ambiente
 * de implantação, como credenciais de rede, endereços de servidor e identificadores
 * únicos de dispositivo. O objetivo é isolar informações sensíveis e facilitar a
 * configuração do projeto para diferentes cenários sem alterar a lógica principal do código.
 *
 * @warning ESTE ARQUIVO CONTÉM INFORMAÇÕES SENSÍVEIS.
 * Nunca compartilhe este arquivo com credenciais reais preenchidas em repositórios
 * públicos como o GitHub. É uma boa prática usar variáveis de ambiente ou um arquivo
 * local ignorado pelo Git para armazenar senhas em um ambiente de produção.
 */

#ifndef CONFIGURA_GERAL_H
#define CONFIGURA_GERAL_H

// =================================================================================
// --- Configurações da Rede Wi-Fi ---
// =================================================================================
/**
 * @brief SSID (Service Set Identifier) ou nome da sua rede Wi-Fi.
 * Substitua pelo nome exato da rede à qual o Pico W deve se conectar.
 */
#define WIFI_SSID "RENASCENCA_Cozinha_multilaser_"

/**
 * @brief Senha da sua rede Wi-Fi.
 * Substitua pela senha correspondente ao SSID acima.
 */
#define WIFI_PASSWORD "12345678"

// =================================================================================
// --- Configurações do Broker MQTT ---
// =================================================================================
/**
 * @brief Endereço IP do servidor (broker) MQTT na sua rede local.
 * O broker é o servidor central que recebe as mensagens dos sensores e as
 * distribui para os clientes inscritos.
 */
#define MQTT_BROKER_IP   "192.168.1.104"

/**
 * @brief Porta de comunicação do broker MQTT.
 * A porta padrão para MQTT não criptografado é 1883. A porta 4004 é uma
 * configuração personalizada do seu broker.
 */
#define MQTT_BROKER_PORT 4004

// =================================================================================
// --- Identificação do Dispositivo ---
// =================================================================================
/**
 * @brief ID único para este dispositivo.
 * É usado como parte do tópico MQTT para identificar a origem dos dados.
 * Isso é crucial quando múltiplos dispositivos publicam no mesmo broker.
 */
#define DEVICE_ID "Sensores"

// =================================================================================
// --- Definições de Tópicos MQTT ---
// =================================================================================
/**
 * @brief Sub-tópico onde os dados de luminosidade serão publicados.
 * O tópico final será composto pelo DEVICE_ID e este valor.
 * Exemplo: "Sensores/dados/Sensor_Luz".
 * Tópicos organizam as mensagens em canais temáticos dentro do broker.
 */
//#define TOPICO_PUBLICACAO_LUZ  "dados/Sensor_Luz"
#define TOPICO_PUBLICACAO_JSON "dados/json"

// =================================================================================
// --- Configurações de Hardware (Pinos I2C) ---
// =================================================================================

// --- Configuração da Porta I2C 0 para o sensor AHT10 ---
#define I2C0_PORT i2c0
#define I2C0_SDA_PIN 0
#define I2C0_SCL_PIN 1

// --- Configuração da Porta I2C 1 para o sensor BH1750 ---
// (Os pinos 2 e 3 são usados por padrão no arquivo app_tasks.c)
#define I2C1_PORT i2c1
#define I2C1_SDA_PIN 2
#define I2C1_SCL_PIN 3

#endif // CONFIGURA_GERAL_H