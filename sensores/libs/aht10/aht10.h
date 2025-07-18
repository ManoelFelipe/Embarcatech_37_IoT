/**
 * @file aht10.h
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Define a interface pública (API) para o driver do sensor de umidade e temperatura AHT10.
 *
 * @details
 * Este arquivo de cabeçalho declara as funções e estruturas de dados necessárias
 * para interagir com o sensor AHT10. Ele abstrai os detalhes da comunicação I2C
 * e dos comandos do sensor, fornecendo uma interface simples para inicializar
 * o dispositivo e ler os dados de temperatura e umidade.
 */

#ifndef AHT10_H
#define AHT10_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

/**
 * @brief Endereço I2C padrão do sensor AHT10.
 * @details Este é o endereço de 7 bits do dispositivo na comunicação I2C,
 * definido pelo fabricante no datasheet do componente.
 */
#define AHT10_ADDR 0x38

/**
 * @brief Estrutura para armazenar os dados lidos do sensor AHT10.
 * @details Agrupa os valores de temperatura e umidade em uma única estrutura
 * para facilitar o retorno e o manuseio dos dados pela aplicação.
 */
typedef struct {
    /**
     * @var float temperature
     * @brief Armazena o valor da temperatura em Graus Celsius (°C).
     */
    float temperature;

    /**
     * @var float humidity
     * @brief Armazena o valor da umidade relativa do ar em porcentagem (%).
     */
    float humidity;
} aht10_data_t;

/**
 * @brief Inicializa o sensor AHT10 no barramento I2C especificado.
 * @details Envia os comandos de inicialização necessários para que o sensor
 * saia do estado de repouso e esteja pronto para realizar medições.
 *
 * @param i2c_port Ponteiro para a instância do barramento I2C a ser usada (ex: i2c0, i2c1).
 * @return `true` se a inicialização for bem-sucedida.
 * @return `false` se houver uma falha na comunicação I2C.
 */
bool aht10_init(i2c_inst_t* i2c_port);

/**
 * @brief Dispara uma medição e lê os dados de temperatura e umidade do sensor.
 * @details Esta função executa o ciclo completo de leitura: envia o comando para
 * iniciar uma medição, aguarda o tempo necessário para a conversão, lê os
* dados brutos e os converte para os valores finais de temperatura e umidade.
 *
 * @param i2c_port Ponteiro para a instância do barramento I2C onde o sensor está conectado.
 * @param data Ponteiro para uma estrutura `aht10_data_t` onde os dados lidos serão armazenados.
 * @return `true` se a leitura e a conversão forem bem-sucedidas.
 * @return `false` se houver falha na comunicação ou se o sensor indicar um erro (ex: não calibrado).
 */
bool aht10_read_data(i2c_inst_t* i2c_port, aht10_data_t* data);

#endif // AHT10_H