/**
 * @file aht10.c
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Implementação do driver para o sensor de umidade e temperatura AHT10.
 *
 * @details
 * Este arquivo contém a lógica de baixo nível para se comunicar com o sensor
 * AHT10 via I2C. Ele implementa as funções declaradas em `aht10.h`,
 * gerenciando o envio de comandos e a interpretação dos bytes de dados
 * recebidos do sensor, conforme especificado no datasheet do componente.
 */

#include "aht10.h"

// --- Comandos do AHT10 (conforme o datasheet) ---
/**
 * @brief Comando para inicializar o sensor. Deve ser enviado após o power-on.
 * Esta sequência de bytes configura o sensor para o modo de operação normal.
 */
const uint8_t CMD_INIT[] = {0xE1, 0x08, 0x00};

/**
 * @brief Comando para disparar uma nova medição de temperatura e umidade.
 */
const uint8_t CMD_MEASURE[] = {0xAC, 0x33, 0x00};

/**
 * @brief Implementação da função de inicialização do sensor AHT10.
 */
bool aht10_init(i2c_inst_t* i2c) {
    // Envia o comando de inicialização (calibração) para o sensor.
    // A função `i2c_write_blocking` envia a sequência de bytes de forma síncrona.
    int ret = i2c_write_blocking(i2c, AHT10_ADDR, CMD_INIT, sizeof(CMD_INIT), false);

    // Se `ret` for negativo, ocorreu um erro na comunicação I2C (ex: NACK).
    if (ret < 0) return false;

    // Aguarda um curto período para garantir que o sensor processe o comando de inicialização.
    sleep_ms(20);
    return true;
}

/**
 * @brief Implementação da função de leitura de dados do AHT10.
 */
bool aht10_read_data(i2c_inst_t* i2c, aht10_data_t* data) {
    // 1. Envia o comando para o sensor iniciar uma nova medição.
    int ret = i2c_write_blocking(i2c, AHT10_ADDR, CMD_MEASURE, sizeof(CMD_MEASURE), false);
    if (ret < 0) return false; // Falha ao enviar o comando.

    // 2. Aguarda o tempo de medição. O datasheet indica que leva aproximadamente 75ms.
    // Usamos 80ms para ter uma margem de segurança.
    sleep_ms(80);

    // 3. Lê os 6 bytes de dados de resposta do sensor.
    // O primeiro byte é o status, seguido pelos dados de umidade e temperatura.
    uint8_t buf[6];
    ret = i2c_read_blocking(i2c, AHT10_ADDR, buf, sizeof(buf), false);
    if (ret < 6) return false; // Não conseguiu ler todos os 6 bytes esperados.

    // 4. Checa o byte de status (primeiro byte recebido) para validar a medição.
    // Conforme o datasheet, para uma leitura válida:
    // - O bit 7 (Busy indicator) deve ser 0 (indicando que a medição terminou).
    // - O bit 3 (Calibration enabled) deve ser 1 (indicando que o sensor está calibrado).
    // A máscara 0x88 verifica ambos os bits. O resultado esperado é 0x08.
    if ((buf[0] & 0x88) != 0x08) {
        return false; // O sensor está ocupado ou não está calibrado.
    }

    // 5. Calcula os valores de umidade e temperatura com base nas fórmulas do datasheet.
    // Os dados brutos de 20 bits para umidade e temperatura estão distribuídos pelos bytes de 1 a 5.

    // Extrai o dado bruto da umidade (20 bits).
    uint32_t raw_humidity = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
    // Aplica a fórmula de conversão: Umidade (%) = (dado_bruto / 2^20) * 100
    data->humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;

    // Extrai o dado bruto da temperatura (20 bits).
    uint32_t raw_temp = (((uint32_t)buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    // Aplica a fórmula de conversão: Temperatura (°C) = ((dado_bruto / 2^20) * 200) - 50
    data->temperature = (((float)raw_temp / 1048576.0f) * 200.0f) - 50.0f;

    return true; // Leitura bem-sucedida.
}