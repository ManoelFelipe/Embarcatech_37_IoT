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

/**
 * @brief Inicializa o sensor BH1750, enviando o comando para ligá-lo (power on).
 *
 * Esta função envia um comando essencial para "acordar" o sensor e prepará-lo
 * para receber comandos de medição.
 *
 * @note A inicialização do barramento I2C (`i2c_init()`) deve ser feita
 * externamente (por exemplo, no `app_tasks.c` ou `main.c`) antes de chamar esta função.
 * Isso permite que o mesmo barramento I2C seja compartilhado com outros sensores.
 */
void bh1750_iniciar(void);

/**
 * @brief Solicita uma nova medição ao sensor, aguarda a conversão e retorna o valor em Lux.
 *
 * O processo completo envolve enviar um comando de medição, aguardar o tempo
 * de conversão especificado pelo datasheet do sensor, ler os bytes de dados brutos
 * e, por fim, convertê-los para a unidade de medida Lux.
 *
 * @return float O valor da luminosidade em Lux.
 * @retval -1.0f em caso de erro de comunicação com o sensor via I2C.
 */
float bh1750_ler_lux(void);

#endif // BH1750_H