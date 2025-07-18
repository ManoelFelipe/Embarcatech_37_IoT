/**
 * @file app_tasks.h
 * @version 1.0
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Arquivo de cabeçalho para as tarefas principais da aplicação.
 *
 * @details
 * Este arquivo define a interface pública para o módulo de tarefas da aplicação (`app_tasks.c`).
 * Ele declara os protótipos de todas as funções de alto nível que podem ser chamadas
 * por outros módulos, como o `main.c`. Funciona como um "contrato", garantindo que
 * a comunicação entre os diferentes arquivos do projeto seja clara e organizada.
 * Também centraliza constantes importantes para o comportamento da aplicação.
 */

#ifndef APP_TASKS_H
#define APP_TASKS_H

// Inclui o cabeçalho de tipos booleanos padrão do C, que define `true` e `false`.
// Essencial para funções que retornam um estado de sucesso ou falha.
#include <stdbool.h>

// =================================================================================
// --- Constantes da Aplicação ---
// =================================================================================
/**
 * @brief Define o intervalo do loop principal em milissegundos.
 * @details Esta constante determina a frequência com que o ciclo principal de
 * leitura e publicação de dados será executado.
 * - Um valor MAIOR (ex: 5000) significa leituras menos frequentes, economizando
 * energia e reduzindo o tráfego de rede.
 * - Um valor MENOR (ex: 500) significa leituras mais frequentes, fornecendo dados
 * em tempo mais real, ao custo de maior consumo e tráfego.
 * Usar uma constante definida aqui facilita o ajuste fino do comportamento do
 * dispositivo sem precisar procurar por "números mágicos" espalhados pelo código.
 */
#define INTERVALO_LOOP_MS 1000

// =================================================================================
// --- Protótipos das Funções (Interface Pública) ---
// =================================================================================
/**
 * @brief Inicializa a comunicação serial via USB para fins de depuração.
 * @details Esta função configura o `stdio` para usar a porta USB e bloqueia
 * a execução até que um monitor serial (como o do VS Code ou PuTTY) seja
 * conectado. Isso é vital para garantir que as mensagens de log da fase de
 * inicialização do sistema não sejam perdidas.
 */
void configurar_serial();

/**
 * @brief Inicializa o hardware Wi-Fi e tenta se conectar à rede especificada.
 * @details Realiza a inicialização do chip CYW43 e tenta a conexão com as
 * credenciais definidas em `configura_geral.h`.
 * @return `true` se a conexão for estabelecida com sucesso.
 * @return `false` se ocorrer uma falha na inicialização do hardware ou se
 * a conexão com o ponto de acesso falhar (ex: senha incorreta, sinal fraco).
 */
bool conectar_wifi();

/**
 * @brief Configura e inicializa os periféricos de hardware.
 * @details Prepara os barramentos de comunicação, como o I2C, e inicializa
 * os sensores (BH1750, AHT10) conectados a eles, deixando-os prontos para a leitura.
 */
void configurar_perifericos();

/**
 * @brief Realiza a primeira tentativa de conexão com o broker MQTT.
 * @details Esta função não bloqueia a execução indefinidamente. Ela tenta se
 * conectar por um curto período e, se não conseguir, permite que a execução
 * continue. A responsabilidade de gerenciar a reconexão é delegada ao
 * loop principal, que chamará `gerenciar_reconexao_mqtt()` quando necessário.
 */
void conectar_mqtt_inicial();

/**
 * @brief Executa um ciclo completo de operação: ler, formatar e publicar.
 * @details Esta função é chamada repetidamente pelo `main.c` quando o dispositivo
 * está conectado ao broker MQTT. Ela lê os valores dos sensores, os formata
 * em uma string JSON e publica essa string em um tópico MQTT predefinido.
 */
void processar_ciclo_operacional();

/**
 * @brief Gerencia a tentativa de reconexão com o broker MQTT.
 * @details Esta função é chamada no loop principal sempre que for detectado
 * que a conexão com o broker MQTT foi perdida. Sua responsabilidade é
 * executar os passos necessários para tentar restabelecer a comunicação.
 */
void gerenciar_reconexao_mqtt();


#endif // APP_TASKS_H