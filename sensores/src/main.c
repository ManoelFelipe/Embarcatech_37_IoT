/**
 * @file main.c
 * @version 1.0
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Ponto de entrada principal do projeto de Aquisição de Dados.
 *
 * @details
 * Este arquivo serve como o orquestrador geral da aplicação embarcada. Sua responsabilidade
 * é inicializar os sistemas essenciais em uma ordem lógica e, em seguida, entrar em um
 * loop infinito (superloop) que mantém o dispositivo em operação contínua.
 * A lógica de alto nível é mantida aqui para clareza, enquanto as implementações
 * detalhadas de cada tarefa são delegadas aos módulos em `app_tasks.c`.
 */

// Inclui a biblioteca padrão do Pico para funções essenciais como printf e sleep_ms.
#include <stdio.h>
#include "pico/stdlib.h"
// Inclui o cabeçalho da biblioteca MQTT para a verificação de conexão.
// Esta inclusão permite chamar a função `cliente_mqtt_esta_conectado()`.
#include "mqtt_lwip.h"
// Inclui o cabeçalho com nossas funções de aplicação de alto nível.
// Este é o "contrato" que `main.c` usa para chamar as funções implementadas em `app_tasks.c`.
#include "app_tasks.h"

/**
 * @brief Função principal do programa. Ponto de entrada da execução.
 *
 * @details
 * O fluxo de execução é estruturado em três fases distintas para garantir uma
 * inicialização robusta e uma operação estável:
 *
 * 1.  Fase de Configuração Inicial: Prepara a comunicação serial para depuração
 * e estabelece a conexão com a rede Wi-Fi, que é um pré-requisito crítico
 * para a operação do dispositivo. Se a conexão Wi-Fi falhar, o programa
 * é encerrado, pois não pode cumprir sua função principal.
 *
 * 2.  Fase de Inicialização dos Módulos: Com a conectividade de rede garantida,
 * a função configura os periféricos de hardware (como os barramentos I2C e
 * os sensores conectados a eles) e realiza a primeira tentativa de conexão
 * com o broker MQTT.
 *
 * 3.  Fase de Operação (Superloop): O programa entra em um loop infinito,
 * que é o padrão de arquitetura para sistemas embarcados que devem rodar
 * continuamente. Dentro deste loop, o estado da conexão MQTT é verificado
 * periodicamente. Se conectado, os dados dos sensores são lidos e publicados.
 * Se desconectado, uma rotina de reconexão é acionada. Uma pausa (`sleep_ms`)
 * é utilizada para controlar a frequência das operações, economizando
 * recursos e evitando sobrecarga da rede e do broker.
 *
 * @return int. Em teoria, retornaria 0 em caso de sucesso. No entanto, em um
 * sistema embarcado como este, a função main() é projetada para nunca retornar.
 * O retorno de um valor diferente de zero (como -1) indica uma falha crítica
 * durante a inicialização.
 */
int main() {
    // --- 1. Fase de Configuração Inicial ---

    // Inicializa a comunicação serial via USB e aguarda a conexão de um terminal.
    // Essencial para não perder as mensagens de log iniciais durante o boot.
    configurar_serial();

    // Tenta conectar à rede Wi-Fi. A conectividade é indispensável para as
    // etapas seguintes (comunicação MQTT). Se falhar, o programa não pode continuar.
    if (!conectar_wifi()) {
        printf("Falha crítica de conexão Wi-Fi. O programa será encerrado.\n");
        // Em um sistema embarcado real, isso poderia acionar um LED de erro ou
        // um mecanismo de watchdog para reiniciar o dispositivo.
        return -1;
    }

    // --- 2. Fase de Inicialização dos Módulos ---

    // Configura os barramentos I2C e inicializa os sensores (BH1750, AHT10).
    configurar_perifericos();
    // Realiza a primeira tentativa de conexão ao broker MQTT.
    // A lógica é não-bloqueante; se falhar, a reconexão será tratada no loop principal.
    conectar_mqtt_inicial();

    // --- 3. Fase de Operação (Loop Infinito) ---

    // Imprime uma mensagem para indicar que a fase de inicialização foi concluída.
    printf("--- Iniciando loop principal de operação ---\n");

    // Este é o "superloop", o coração de um firmware embarcado.
    // Ele garante que o microcontrolador execute continuamente suas tarefas.
    while (1) {
        // A cada iteração do loop, a primeira tarefa é verificar o estado da conexão MQTT.
        // Isso torna o sistema resiliente a falhas de rede ou do broker.
        if (cliente_mqtt_esta_conectado()) {
            // Se a conexão estiver ativa, o dispositivo executa sua tarefa principal:
            // ler os dados dos sensores, formatá-los em JSON e publicá-los.
            processar_ciclo_operacional();
        } else {
            // Se a conexão foi perdida, a tarefa principal é tentar restabelecê-la.
            gerenciar_reconexao_mqtt();
        }

        // Pausa a execução pelo intervalo definido em `app_tasks.h`.
        // Este `sleep` é crucial para:
        // 1. Reduzir o consumo de energia e o aquecimento do processador.
        // 2. Controlar a frequência de envio de dados, evitando sobrecarregar o broker MQTT.
        // 3. Criar um ciclo de operação previsível e temporizado.
        sleep_ms(INTERVALO_LOOP_MS);
    }

    // Este ponto do código nunca deve ser alcançado em uma aplicação embarcada
    // projetada para rodar indefinidamente. Se alcançado, indica um erro na lógica do programa.
    return 0;
}