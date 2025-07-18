/**
 * @file mqtt_lwip.c
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Módulo encapsulado para o cliente MQTT usando a pilha lwIP.
 *
 * @details
 * Este arquivo gerencia a conexão com o broker MQTT e fornece uma interface
 * simplificada para publicar mensagens. Ele é projetado para ser usado
 * de forma modular pelo restante da aplicação, escondendo a complexidade
 * dos callbacks e da API assíncrona da biblioteca MQTT da lwIP.
 */

// Bibliotecas da pilha de rede lwIP
#include "lwip/apps/mqtt.h" // Funções e estruturas do cliente MQTT
#include "lwip/ip_addr.h"   // Funções para manipulação de endereços IP

// Bibliotecas padrão do C
#include <string.h> // Para strlen()
#include <stdio.h>  // Para printf()

// =================================================================================
// --- Variáveis Estáticas do Módulo ---
// (O qualificador 'static' restringe a visibilidade destas variáveis a este arquivo,
//  criando um encapsulamento e evitando conflitos de nomes globais)
// =================================================================================

/** @brief Ponteiro global para a instância do cliente MQTT da biblioteca lwIP.
 * Armazena o estado da conexão e outras informações do cliente.
 */
static mqtt_client_t *client;

/** @brief Flag para controle de fluxo da publicação.
 * @details Quando usamos QoS 1 ou 2, a publicação é um processo de duas etapas:
 * 1. Enviamos a mensagem.
 * 2. O broker envia uma confirmação (PUBACK).
 * Esta flag é usada como uma "trava" para evitar o envio de uma nova mensagem
 * antes de recebermos a confirmação da anterior, garantindo a ordem.
 */
static bool publicacao_em_andamento = false;


// =================================================================================
// --- Funções de Callback Internas ---
// (Estas são funções que NÓS escrevemos, mas que são chamadas pela biblioteca lwIP
//  em resposta a eventos de rede específicos. É a base da programação assíncrona).
// =================================================================================

/**
 * @brief Callback executado pela lwIP quando o status da conexão com o broker muda.
 * @param client Ponteiro para a instância do cliente MQTT.
 * @param arg Argumento opcional passado durante a chamada de conexão (não utilizado aqui).
 * @param status O novo status da conexão (ex: MQTT_CONNECT_ACCEPTED).
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado ao broker! (Modo Apenas Publicação)\n");
        // Nota: Se precisássemos nos inscrever em tópicos, faríamos isso aqui,
        // pois a conexão acaba de ser estabelecida.
    } else {
        // Imprime um código de erro que pode ser consultado na documentação da lwIP
        // para diagnosticar problemas de conexão (ex: ID de cliente duplicado, etc.).
        printf("[MQTT] Falha na conexão: código %d\n", status);
    }
}

/**
 * @brief Callback executado pela lwIP após uma tentativa de publicação com QoS > 0.
 * @details A lwIP chama esta função quando o broker confirma o recebimento da mensagem
 * (enviando um pacote PUBACK para QoS 1) ou quando a tentativa falha.
 * @param arg Argumento opcional passado na chamada de `mqtt_publish` (não utilizado aqui).
 * @param err Resultado da publicação (`ERR_OK` se bem-sucedida).
 */
static void mqtt_pub_request_cb(void *arg, err_t err) {
    // Libera a trava para permitir que a próxima mensagem seja enviada.
    publicacao_em_andamento = false;
    if (err != ERR_OK) {
        printf("[MQTT] Falha na confirmação da publicação, erro: %d\n", err);
    }
}


// =================================================================================
// --- Funções Públicas (Implementação da API declarada em mqtt_lwip.h) ---
// =================================================================================

/**
 * @brief Implementação da função que publica uma mensagem MQTT.
 */
void publicar_mensagem_mqtt(const char *topico, const char *mensagem) {
    // Verificações de segurança (guard clauses): não faz nada se...
    // ...o cliente não foi inicializado (`!client`),
    // ...o cliente não está conectado (`!mqtt_client_is_connected`),
    // ...ou uma publicação anterior ainda está aguardando confirmação.
    if (!client || !mqtt_client_is_connected(client) || publicacao_em_andamento) {
        return;
    }

    err_t err;
    // Chama a função da lwIP para publicar a mensagem.
    // Parâmetros:
    // client: A instância do nosso cliente.
    // topico: O tópico de destino.
    // mensagem: O payload (conteúdo) da mensagem.
    // strlen(mensagem): O tamanho do payload.
    // 1: Quality of Service (QoS) 1 - Garante que a mensagem seja entregue pelo menos uma vez.
    // 0: Retain flag - A mensagem não será retida no broker para novos inscritos.
    // mqtt_pub_request_cb: O callback a ser chamado quando a confirmação chegar.
    // NULL: Argumento opcional para o callback.
    err = mqtt_publish(client, topico, mensagem, strlen(mensagem), 1, 0, mqtt_pub_request_cb, NULL);

    if (err == ERR_OK) {
        // A chamada foi bem-sucedida. Ativa a trava para aguardar a confirmação
        // que virá através do callback `mqtt_pub_request_cb`.
        publicacao_em_andamento = true;
    } else {
         printf("[MQTT] Erro ao tentar enfileirar publicação: %d\n", err);
    }
}

/**
 * @brief Implementação da função que inicializa o cliente MQTT.
 */
void iniciar_mqtt_cliente(const char* broker_ip, int broker_port, const char* device_id) {
    // Aloca memória para uma nova instância do cliente MQTT.
    client = mqtt_client_new();

    // Converte o endereço IP de string (ex: "192.168.1.104") para o formato numérico da lwIP.
    ip_addr_t broker_addr;
    ipaddr_aton(broker_ip, &broker_addr);

    // Monta um Client ID único para o dispositivo, concatenando o ID do dispositivo com "_client".
    // É uma boa prática para depuração no broker.
    char client_id[32];
    snprintf(client_id, sizeof(client_id), "%s_client", device_id);

    // Estrutura com informações de conexão do cliente.
    // O único campo obrigatório é o client_id. Outros campos como
    // username/password ou last will testament (LWT) poderiam ser definidos aqui.
    struct mqtt_connect_client_info_t ci = { .client_id = client_id };

    // Dispara a tentativa de conexão. Esta função é assíncrona e retorna imediatamente.
    // O resultado da conexão (sucesso ou falha) será reportado no futuro, através
    // de uma chamada da lwIP à nossa função `mqtt_connection_cb`.
    mqtt_client_connect(client, &broker_addr, broker_port, mqtt_connection_cb, 0, &ci);
}

/**
 * @brief Implementação da função que verifica o estado da conexão MQTT.
 */
bool cliente_mqtt_esta_conectado(void) {
    // A verificação `client != NULL` é crucial para evitar um crash (null pointer dereference)
    // caso esta função seja chamada antes de `iniciar_mqtt_cliente()` ter sido executada.
    if (client == NULL) {
        return false;
    }
    // Retorna o status de conexão mantido internamente pela biblioteca lwIP.
    return mqtt_client_is_connected(client);
}