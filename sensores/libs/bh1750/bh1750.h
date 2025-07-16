/**
 * @file bh1750.h
 * @version 1.1
 * @date 14/07/2025
 * @brief Define a interface pública (API) para o módulo do sensor de luz BH1750.
 *
 * Este arquivo de cabeçalho declara as funções que podem ser chamadas
 * por outras partes do sistema para interagir com o sensor de luminosidade.
 * Ele abstrai os detalhes de baixo nível do hardware, fornecendo um
 * conjunto de comandos simples e de alto nível.
 */

#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @brief Inicializa o sensor BH1750 no barramento I2C especificado.
 * @param i2c_port Ponteiro para a instância I2C (ex: i2c0, i2c1).
 */
void bh1750_iniciar(i2c_inst_t *i2c_port);

/**
 * @brief Lê o valor de luminosidade (em Lux) do sensor.
 * @param i2c_port Ponteiro para a instância I2C onde o sensor está conectado.
 * @return O valor de luminosidade em Lux como um float.
 */
float bh1750_ler_lux(i2c_inst_t *i2c_port);

#endif // BH1750_H