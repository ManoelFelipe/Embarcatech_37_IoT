/**
 * @file mqtt_lwip.h
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Define a interface pública (API) para o módulo cliente MQTT.
 *
 * @details
 * Este arquivo de cabeçalho declara as funções que podem ser chamadas
 * por outras partes do sistema (como o `app_tasks.c`) para interagir com o cliente MQTT.
 * O objetivo é abstrair a complexidade da biblioteca lwIP, oferecendo
 * uma interface simples e focada nas necessidades do projeto: inicializar,
 * conectar, publicar e verificar o estado da conexão.
 */

#ifndef MQTT_LWIP_H
#define MQTT_LWIP_H

// Incluído para permitir o uso do tipo de dados 'bool' (true/false) nativamente,
// o que torna o código mais legível e explícito em funções que retornam status.
#include <stdbool.h>

/**
 * @brief Inicializa o cliente MQTT e dispara a tentativa de conexão com o broker.
 *
 * @details Esta função é assíncrona; ela configura o cliente e solicita a
 * conexão, mas retorna imediatamente, sem esperar que a conexão seja de fato
 * estabelecida. A pilha de rede lwIP gerencia a tentativa de conexão em
 * segundo plano. O status da conexão pode ser verificado posteriormente
 * com a função `cliente_mqtt_esta_conectado()`.
 *
 * @param broker_ip String C (`const char*`) contendo o endereço IP do broker MQTT.
 * @param broker_port O número da porta do broker (ex: 1883, 4004).
 * @param device_id ID único do cliente para se identificar ao broker. É importante
 * que cada dispositivo conectado tenha um ID exclusivo.
 */
void iniciar_mqtt_cliente(const char* broker_ip, int broker_port, const char* device_id);

/**
 * @brief Publica uma mensagem em um tópico MQTT específico.
 *
 * @details A função encapsula a chamada de publicação da lwIP, cuidando dos detalhes
 * como o cálculo do tamanho da mensagem e a configuração da Qualidade de Serviço (QoS).
 * A implementação interna utiliza QoS 1, que garante a entrega da mensagem
 * pelo menos uma vez, sendo ideal para dados de sensores que não podem ser perdidos.
 *
 * @param topico A string do tópico para o qual a mensagem será publicada (ex: "Sensores/dados/json").
 * @param mensagem O conteúdo (payload) da mensagem a ser enviada.
 */
void publicar_mensagem_mqtt(const char *topico, const char *mensagem);

/**
 * @brief Verifica o status atual da conexão com o broker MQTT.
 *
 * @details Realiza uma verificação segura para determinar se o cliente MQTT foi
 * inicializado e se está atualmente conectado e pronto para publicar mensagens.
 * É fundamental para evitar tentativas de publicação em um cliente desconectado.
 *
 * @return `true` se o cliente está conectado, `false` caso contrário (desconectado,
 * nunca conectado, ou se o cliente ainda não foi nem inicializado).
 */
bool cliente_mqtt_esta_conectado(void);

#endif // MQTT_LWIP_H