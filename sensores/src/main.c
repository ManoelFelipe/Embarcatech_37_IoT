/**
 * @file main.c
 * @version 1.1
 * @date 14/07/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Ponto de entrada principal do projeto de Aquisição de Dados.
 *
 * Orquestra a inicialização e o loop principal da aplicação, delegando
 * as tarefas de baixo nível para os módulos específicos implementados
 * em `app_tasks.c`. Esta abordagem torna o fluxo do programa claro e legível.
 */

// Inclui a biblioteca padrão do Pico e SDK
#include <stdio.h>
#include "pico/stdlib.h"
// Inclui o cabeçalho da biblioteca MQTT para a verificação de conexão
#include "mqtt_lwip.h"
// Inclui o cabeçalho com nossas funções de aplicação de alto nível
#include "app_tasks.h"

/**
 * @brief Função principal do programa. Ponto de entrada da execução.
 *
 * O fluxo de execução é o seguinte:
 * 1. Configura a comunicação serial para depuração.
 * 2. Conecta à rede Wi-Fi. Se falhar, o programa encerra.
 * 3. Configura os periféricos de hardware (I2C, sensores).
 * 4. Tenta a conexão inicial com o broker MQTT.
 * 5. Entra em um loop infinito (superloop) que gerencia o estado da aplicação.
 *
 * @return int. Teoricamente retorna 0 em caso de sucesso, mas em um sistema
 * embarcado, a função main() nunca deve retornar.
 */
int main() {
    // 1. Fase de Configuração Inicial
    configurar_serial();

    // Tenta conectar ao Wi-Fi. Se não conseguir, não há como continuar.
    if (!conectar_wifi()) {
        printf("Falha crítica de conexão. O programa será encerrado.\n");
        return -1;
    }
    
    // 2. Fase de Inicialização dos Módulos
    configurar_perifericos();
    conectar_mqtt_inicial();

    // 3. Fase de Operação (Loop Infinito)
    // Este é o "superloop", padrão em muitos sistemas embarcados.
    printf("--- Iniciando loop principal de operação ---\n");
    while (1) {
        // Verifica o estado da conexão MQTT a cada ciclo.
        if (cliente_mqtt_esta_conectado()) {
            // Se conectado, executa a tarefa principal (ler e publicar).
            processar_ciclo_operacional();
        } else {
            // Se desconectado, tenta restabelecer a conexão.
            gerenciar_reconexao_mqtt();
        }
        
        // Pausa a execução pelo intervalo definido, controlando a frequência das operações.
        // Isso economiza processamento e evita sobrecarregar o broker com mensagens.
        sleep_ms(INTERVALO_LOOP_MS); 
    }

    // Este ponto do código nunca deve ser alcançado em um sistema embarcado.
    return 0;
}