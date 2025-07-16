/**
 * @file bh1750.c
 * @version 1.1
 * @date 14/07/2025
 * @brief Driver para o sensor de luminosidade I2C BH1750.
 *
 * Este arquivo implementa as funções para inicializar e ler dados
 * do sensor de luz ambiente BH1750. Ele gerencia a comunicação I2C,
 * incluindo o envio de comandos de operação e a interpretação dos
 * dados brutos recebidos do sensor.
 */

// Bibliotecas do SDK do Raspberry Pi Pico
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Cabeçalho do próprio módulo
#include "bh1750.h"

// =================================================================================
// --- Parâmetros de Hardware e Protocolo ---
// =================================================================================

/**
 * @brief Define qual dos dois barramentos I2C do Pico será usado.
 * O RP2040 possui i2c0 e i2c1.
 */
//#define I2C_PORT i2c1

/**
 * @brief Endereço padrão do sensor BH1750 na comunicação I2C.
 * Este valor (0x23) é definido pelo fabricante e pode ser alterado
 * (geralmente para 0x5C) dependendo do estado do pino ADDR do sensor.
 */
#define BH1750_ADDR 0x23

// --- Comandos de Operação do Sensor BH1750 ---
// Estes valores são definidos no datasheet do sensor.
// São declarados como 'const' para que possamos obter seu endereço com '&',
// o que é necessário para as funções de escrita I2C do SDK.

// Comandos de Operação do Sensor BH1750
static const uint8_t CMD_POWER_ON = 0x01;
static const uint8_t CMD_CONTINUOUS_HIGH_RES = 0x10; // Modo contínuo, alta resolução 1

/**
 * @brief Envia os comandos iniciais para ligar e configurar o sensor BH1750.
 */

void bh1750_iniciar(i2c_inst_t *i2c) {
    // 1. Envia comando para ligar o sensor
    i2c_write_blocking(i2c, BH1750_ADDR, &CMD_POWER_ON, 1, false);
    sleep_ms(10); // Pequena espera

    // 2. Configura para o modo de medição contínua de alta resolução.
    // Neste modo, o sensor fará medições constantemente, não precisamos
    // enviar um comando a cada leitura.
    i2c_write_blocking(i2c, BH1750_ADDR, &CMD_CONTINUOUS_HIGH_RES, 1, false);
    sleep_ms(10);
}

/**
 * @brief Realiza uma leitura completa do sensor e converte o valor para Lux.
 */
float bh1750_ler_lux(i2c_inst_t *i2c) {
    uint8_t raw_data[2];

    // Como o sensor está em modo contínuo, só precisamos ler o valor mais recente.
    int bytes_read = i2c_read_blocking(i2c, BH1750_ADDR, raw_data, 2, false);

    // Verifica se a leitura foi bem-sucedida.
    if (bytes_read < 2) {
        return -1.0f; // Retorna um valor de erro.
    }

    // Combina os dois bytes em um valor de 16 bits.
    uint16_t raw_value = (raw_data[0] << 8) | raw_data[1];

    // Converte o valor bruto para Lux usando o fator do datasheet.
    return (float)raw_value / 1.2f;
}