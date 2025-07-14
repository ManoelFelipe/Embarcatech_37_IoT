/**
 * @file app_tasks.h
 * @version 1.1
 * @date 14/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Arquivo de cabeçalho para as tarefas principais da aplicação.
 *
 * Este arquivo declara os protótipos das funções que encapsulam as principais
 * responsabilidades da aplicação, como inicialização de hardware, conectividade
 * e o loop operacional. Ele define a interface pública para o módulo `app_tasks.c`,
 * permitindo que outros arquivos, como o `main.c`, utilizem estas funcionalidades
 * de forma organizada.
 */

#ifndef APP_TASKS_H
#define APP_TASKS_H

// Inclui o cabeçalho de tipos booleanos padrão do C (true, false).
#include <stdbool.h>

// =================================================================================
// --- Constantes da Aplicação ---
// =================================================================================
/**
 * @brief Define o intervalo do loop principal em milissegundos.
 * Usar uma constante definida aqui facilita o ajuste do tempo de ciclo
 * do dispositivo sem precisar procurar por "números mágicos" no código.
 * Este valor determina a frequência de leitura do sensor e publicação dos dados.
 */
#define INTERVALO_LOOP_MS 1000

// =================================================================================
// --- Protótipos das Funções (Interface Pública) ---
// =================================================================================

/**
 * @brief Inicializa a comunicação serial via USB.
 * @note Esta função bloqueia a execução até que um monitor serial (como o do
 * VS Code ou PuTTY) seja conectado. Isso garante que as mensagens de inicialização
 * não sejam perdidas.
 */
void configurar_serial();

/**
 * @brief Inicializa o hardware Wi-Fi e tenta se conectar à rede especificada.
 * @return `true` se a conexão for bem-sucedida.
 * @return `false` se ocorrer uma falha na inicialização ou na conexão.
 */
bool conectar_wifi();

/**
 * @brief Configura e inicializa periféricos de hardware, como o barramento I2C e sensores.
 */
void configurar_perifericos();

/**
 * @brief Inicia a conexão com o broker MQTT e aguarda um tempo para estabilizar.
 * @note Esta função não bloqueia indefinidamente, ela tenta por um período
 * e continua a execução mesmo que a conexão inicial falhe, permitindo
 * que a lógica de reconexão no loop principal assuma.
 */
void conectar_mqtt_inicial();

/**
 * @brief Executa um ciclo completo de operação quando o dispositivo está online.
 * Isto inclui ler o valor do sensor de luminosidade, formatar a mensagem
 * e publicá-la no tópico MQTT apropriado.
 */
void processar_ciclo_operacional();

/**
 * @brief Gerencia a tentativa de reconexão com o broker MQTT.
 * Esta função é chamada no loop principal sempre que a conexão MQTT é perdida.
 */
void gerenciar_reconexao_mqtt();


#endif // APP_TASKS_H