/**
 * @file bh1750.h
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Define a interface pública (API) para o módulo do sensor de luz BH1750.
 *
 * @details
 * Este arquivo de cabeçalho declara as funções que podem ser chamadas
 * por outras partes do sistema para interagir com o sensor de luminosidade.
 * Ele abstrai os detalhes de baixo nível do hardware, como os endereços e
 * comandos I2C, fornecendo um conjunto de funções simples e de alto nível
 * para inicializar o sensor e obter leituras de luz em Lux.
 */

#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @brief Inicializa o sensor BH1750 no barramento I2C especificado.
 * @details Envia os comandos necessários para ligar o sensor e configurá-lo
 * para o modo de medição contínua de alta resolução.
 *
 * @param i2c_port Ponteiro para a instância do barramento I2C a ser usada (ex: i2c0, i2c1).
 */
void bh1750_iniciar(i2c_inst_t *i2c_port);

/**
 * @brief Lê o valor de luminosidade (em Lux) do sensor.
 * @details Realiza a leitura dos dados brutos do sensor via I2C e os converte
 * para a unidade de medida padrão, Lux, utilizando o fator de conversão
 * especificado no datasheet do componente.
 *
 * @param i2c_port Ponteiro para a instância I2C onde o sensor está conectado.
 * @return O valor de luminosidade em Lux como um número de ponto flutuante (float).
 * Retorna um valor negativo (-1.0f) em caso de falha na leitura.
 */
float bh1750_ler_lux(i2c_inst_t *i2c_port);

#endif // BH1750_H