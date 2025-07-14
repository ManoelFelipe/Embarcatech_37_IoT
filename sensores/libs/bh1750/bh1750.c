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
#define I2C_PORT i2c1
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

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

/** @brief Comando para ligar o oscilador interno do sensor. */
const uint8_t BH1750_CMD_POWER_ON = 0x01;

/** @brief Comando para iniciar uma medição no modo de alta resolução (precisão de 1 lux). */
const uint8_t BH1750_CMD_HIRES1 = 0x20;


/**
 * @brief Envia os comandos iniciais para ligar e configurar o sensor BH1750.
 */
void bh1750_iniciar(void) {
    // Envia o comando para "acordar" o sensor. O último parâmetro 'false'
    // indica para a função I2C liberar o barramento após a escrita (enviar um STOP).
    i2c_write_blocking(I2C_PORT, BH1750_ADDR, &BH1750_CMD_POWER_ON, 1, false);
    // Uma pequena pausa para garantir que o sensor esteja pronto para o próximo comando.
    sleep_ms(10);
}

/**
 * @brief Realiza uma leitura completa do sensor e converte o valor para Lux.
 */
float bh1750_ler_lux(void) {
    // Array para armazenar os 2 bytes de dados brutos recebidos do sensor.
    uint8_t raw_data[2];

    // 1. Envia o comando para o sensor iniciar uma nova medição.
    i2c_write_blocking(I2C_PORT, BH1750_ADDR, &BH1750_CMD_HIRES1, 1, false);
    
    // 2. Aguarda o tempo necessário para a conversão interna do sensor.
    // Este valor (180ms) vem do datasheet e é o tempo máximo para o modo de alta resolução.
    sleep_ms(180); 
    
    // 3. Lê os 2 bytes do resultado da medição.
    // A função retorna o número de bytes lidos com sucesso.
    int bytes_read = i2c_read_blocking(I2C_PORT, BH1750_ADDR, raw_data, 2, false);
    
    // Verifica se a leitura foi bem-sucedida. Se menos de 2 bytes foram lidos, houve um erro.
    if (bytes_read < 2) {
        return -1.0f; // Retorna um valor de erro definido.
    }
    
    // 4. Combina os dois bytes de 8 bits em um único valor inteiro de 16 bits.
    // Isso é feito usando operações de deslocamento de bits (bit-shifting).
    // `raw_data[0]` é o byte mais significativo (MSB) e `raw_data[1]` o menos significativo (LSB).
    uint16_t raw_value = (raw_data[0] << 8) | raw_data[1];
    
    // 5. Converte o valor bruto para Lux.
    // O fator de conversão 1.2 também é especificado no datasheet do BH1750.
    // A conversão para (float) garante que a divisão seja de ponto flutuante.
    return (float)raw_value / 1.2f;
}