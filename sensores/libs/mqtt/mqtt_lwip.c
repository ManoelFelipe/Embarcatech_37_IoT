/**
 * @file mqtt_lwip.c
 * @version 1.1
 * @date 14/07/2025
 * @brief Módulo encapsulado para o cliente MQTT usando a pilha lwIP.
 *
 * Este arquivo gerencia a conexão com o broker MQTT e fornece uma interface
 * simplificada para publicar mensagens. Ele é projetado para ser usado
 * de forma modular pelo restante da aplicação.
 */

// Bibliotecas da pilha de rede lwIP
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

// Bibliotecas padrão do C
#include <string.h>
#include <stdio.h>

// =================================================================================
// --- Variáveis Estáticas do Módulo ---
// (static significa que estas variáveis só são visíveis dentro deste arquivo)
// =================================================================================

/** @brief Ponteiro global para a instância do cliente MQTT da biblioteca lwIP. */
static mqtt_client_t *client;

/** @brief Flag para controle de fluxo. Evita o envio de uma nova mensagem antes da confirmação da anterior (essencial para QoS > 0). */
static bool publicacao_em_andamento = false;


// =================================================================================
// --- Funções de Callback Internas ---
// (Funções chamadas pela biblioteca lwIP em resposta a eventos de rede)
// =================================================================================

/**
 * @brief Callback executado quando o status da conexão com o broker muda.
 * @param client Ponteiro para o cliente MQTT.
 * @param arg Argumento opcional passado durante a conexão (não utilizado aqui).
 * @param status Status da conexão (ex: MQTT_CONNECT_ACCEPTED).
 */
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("[MQTT] Conectado ao broker! (Modo Apenas Publicação)\n");
    } else {
        // Imprime um código de erro que pode ser consultado na documentação da lwIP.
        printf("[MQTT] Falha na conexão: código %d\n", status);
    }
}

/**
 * @brief Callback executado após uma tentativa de publicação com QoS > 0.
 * A lwIP chama esta função quando o broker confirma o recebimento da mensagem.
 * @param arg Argumento opcional (não utilizado aqui).
 * @param err Resultado da publicação (ERR_OK se bem-sucedida).
 */
static void mqtt_pub_request_cb(void *arg, err_t err) {
    // Libera a trava para permitir que a próxima mensagem seja enviada.
    publicacao_em_andamento = false;
    if (err != ERR_OK) {
        printf("[MQTT] Falha na publicação, erro: %d\n", err);
    }
}


// =================================================================================
// --- Funções Públicas (Implementação da API) ---
// =================================================================================

/**
 * @brief Publica uma mensagem em um tópico MQTT.
 */
void publicar_mensagem_mqtt(const char *topico, const char *mensagem) {
    // Verificações de segurança: não faz nada se o cliente não existe, não está conectado
    // ou se uma publicação anterior ainda está aguardando confirmação.
    if (!client || !mqtt_client_is_connected(client) || publicacao_em_andamento) {
        return;
    }
    
    err_t err;
    // Chama a função da lwIP para publicar a mensagem.
    // Parâmetros: cliente, tópico, payload, tamanho do payload, QoS, retain, callback, arg do callback.
    // QoS 1: Garante que a mensagem seja entregue pelo menos uma vez.
    // Retain 0: A mensagem não será retida no broker para novos inscritos.
    err = mqtt_publish(client, topico, mensagem, strlen(mensagem), 1, 0, mqtt_pub_request_cb, NULL);
    
    if (err == ERR_OK) {
        // Ativa a trava para aguardar a confirmação do callback `mqtt_pub_request_cb`.
        publicacao_em_andamento = true;
    } else {
         printf("[MQTT] Erro ao tentar publicar: %d\n", err);
    }
}

/**
 * @brief Inicializa o cliente MQTT e tenta se conectar ao broker.
 */
void iniciar_mqtt_cliente(const char* broker_ip, int broker_port, const char* device_id) {
    // Cria uma nova instância do cliente MQTT.
    client = mqtt_client_new();
    
    // Converte o endereço IP de string para o formato numérico da lwIP.
    ip_addr_t broker_addr;
    ipaddr_aton(broker_ip, &broker_addr);

    // Monta um Client ID único para o dispositivo.
    char client_id[32];
    snprintf(client_id, sizeof(client_id), "%s_client", device_id);
    
    // Estrutura com informações de conexão do cliente.
    struct mqtt_connect_client_info_t ci = { .client_id = client_id };
    
    // Dispara a tentativa de conexão. A função retorna imediatamente.
    // O resultado da conexão será reportado no callback 'mqtt_connection_cb'.
    mqtt_client_connect(client, &broker_addr, broker_port, mqtt_connection_cb, 0, &ci);
}

/**
 * @brief Verifica de forma segura se o cliente MQTT está conectado.
 */
bool cliente_mqtt_esta_conectado(void) {
    // A verificação `client != NULL` é crucial para evitar um crash
    // caso esta função seja chamada antes de `iniciar_mqtt_cliente()`.
    if (client == NULL) {
        return false;
    }
    // Retorna o status de conexão mantido pela biblioteca lwIP.
    return mqtt_client_is_connected(client);
}