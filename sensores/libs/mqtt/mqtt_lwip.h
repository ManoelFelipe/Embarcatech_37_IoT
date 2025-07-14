/**
 * @file mqtt_lwip.h
 * @version 1.1
 * @date 14/07/2025
 * @brief Define a interface pública (API) para o módulo cliente MQTT.
 *
 * Este arquivo de cabeçalho declara as funções que podem ser chamadas
 * por outras partes do sistema para interagir com o cliente MQTT.
 * O objetivo é abstrair a complexidade da biblioteca lwIP, oferecendo
 * uma interface simples e focada nas necessidades do projeto.
 */

#ifndef MQTT_LWIP_H
#define MQTT_LWIP_H

// Incluído para permitir o uso do tipo de dados 'bool' (true/false),
// tornando o código mais legível.
#include <stdbool.h>

/**
 * @brief Inicializa o cliente MQTT e dispara a tentativa de conexão com o broker.
 *
 * Esta função é assíncrona; ela retorna imediatamente enquanto a conexão
 * é estabelecida em segundo plano pela pilha de rede lwIP. O status da
 * conexão pode ser verificado posteriormente com `cliente_mqtt_esta_conectado()`.
 *
 * @param broker_ip String contendo o endereço IP do broker.
 * @param broker_port Número da porta do broker.
 * @param device_id ID único do cliente para se identificar ao broker.
 */
void iniciar_mqtt_cliente(const char* broker_ip, int broker_port, const char* device_id);

/**
 * @brief Publica uma mensagem em um tópico MQTT específico.
 *
 * A função encapsula a chamada de publicação da lwIP, cuidando dos detalhes
 * como o cálculo do tamanho da mensagem e a configuração da qualidade de serviço (QoS).
 *
 * @param topico A string do tópico (ex: "Sensores/dados/Sensor_Luz").
 * @param mensagem O conteúdo (payload) da mensagem a ser enviada.
 */
void publicar_mensagem_mqtt(const char *topico, const char *mensagem);

/**
 * @brief Verifica o status atual da conexão com o broker MQTT.
 *
 * Realiza uma verificação segura para determinar se o cliente foi inicializado
 * e se está atualmente conectado ao broker.
 *
 * @return `true` se o cliente está conectado, `false` caso contrário.
 */
bool cliente_mqtt_esta_conectado(void);

#endif // MQTT_LWIP_H