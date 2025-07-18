/**
 * @file bh1750.c
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Driver para o sensor de luminosidade I2C BH1750.
 *
 * @details
 * Este arquivo implementa as funções para inicializar e ler dados
 * do sensor de luz ambiente BH1750. Ele gerencia a comunicação I2C,
 * incluindo o envio de comandos de operação e a interpretação dos
 * dados brutos recebidos do sensor para convertê-los em Lux.
 */

// Bibliotecas do SDK do Raspberry Pi Pico
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Cabeçalho do próprio módulo, para garantir consistência com a API declarada.
#include "bh1750.h"

// =================================================================================
// --- Parâmetros de Hardware e Protocolo ---
// =================================================================================

/**
 * @brief Endereço I2C padrão de 7 bits do sensor BH1750.
 * @details Este valor (0x23) é definido pelo fabricante. Ele pode ser alterado
 * para 0x5C se o pino ADDR do sensor for conectado a VCC, permitindo que
 * dois sensores BH1750 coexistam no mesmo barramento I2C.
 */
#define BH1750_ADDR 0x23

// --- Comandos de Operação do Sensor BH1750 ---
// Estes valores (opcodes) são definidos no datasheet do sensor.
// São declarados como 'static const' para que o compilador possa otimizá-los
// e para que possamos obter seus endereços com '&' de forma segura, o que é
// necessário para as funções de escrita I2C do SDK do Pico.

/** @brief Comando para ligar o oscilador interno do sensor. */
static const uint8_t CMD_POWER_ON = 0x01;
/** @brief Comando para iniciar medições contínuas no modo de alta resolução (1 lux). */
static const uint8_t CMD_CONTINUOUS_HIGH_RES = 0x10;

/**
 * @brief Implementação da função que inicializa o sensor BH1750.
 */
void bh1750_iniciar(i2c_inst_t *i2c) {
    // 1. Envia o comando para ligar o sensor (Power On).
    // O sensor sai do modo de baixo consumo e se prepara para receber outros comandos.
    i2c_write_blocking(i2c, BH1750_ADDR, &CMD_POWER_ON, 1, false);
    sleep_ms(10); // Pequena espera para estabilização.

    // 2. Configura o sensor para o modo de medição contínua de alta resolução.
    // Neste modo, o sensor fará medições constantemente (a cada ~120ms).
    // Isso simplifica a leitura, pois não precisamos enviar um comando a cada vez,
    // apenas solicitar os dados já medidos.
    i2c_write_blocking(i2c, BH1750_ADDR, &CMD_CONTINUOUS_HIGH_RES, 1, false);
    sleep_ms(10); // Pequena espera.
}

/**
 * @brief Implementação da função que lê e converte o valor para Lux.
 */
float bh1750_ler_lux(i2c_inst_t *i2c) {
    // Array para armazenar os 2 bytes de dados brutos recebidos do sensor.
    uint8_t raw_data[2];

    // Como o sensor está em modo contínuo, só precisamos ler o valor mais recente.
    // A função `i2c_read_blocking` solicita 2 bytes do endereço do sensor.
    int bytes_read = i2c_read_blocking(i2c, BH1750_ADDR, raw_data, 2, false);

    // Verifica se a leitura foi bem-sucedida. Se `bytes_read` for menor que 2,
    // significa que a comunicação falhou (ex: o sensor não respondeu - NACK).
    if (bytes_read < 2) {
        return -1.0f; // Retorna um valor de erro facilmente identificável.
    }

    // Os dados chegam em formato Big Endian (primeiro o byte mais significativo).
    // Combina os dois bytes em um único valor inteiro de 16 bits.
    // (Ex: raw_data[0] = 0x1A, raw_data[1] = 0x2B -> raw_value = 0x1A2B)
    uint16_t raw_value = (raw_data[0] << 8) | raw_data[1];

    // Converte o valor bruto para Lux.
    // De acordo com o datasheet do BH1750, para os modos de alta resolução,
    // o resultado deve ser dividido por 1.2 para se obter o valor em Lux.
    return (float)raw_value / 1.2f;
}