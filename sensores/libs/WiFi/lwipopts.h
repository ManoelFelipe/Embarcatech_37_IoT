/**
 * @file lwipopts.h
 * @version 1.0 
 * @date 18/07/2025
 * @author Manoel Felipe Costa Furtado (baseado em implementações comuns)
 * @brief Arquivo de configuração para a pilha de rede lwIP (Lightweight IP).
 *
 * @details
 * Este arquivo permite personalizar o comportamento e o consumo de memória
 * da pilha de rede. As opções aqui definidas com `#define` sobrescrevem as
 * configurações padrão da lwIP, permitindo otimizar a pilha para os recursos
 * limitados de um microcontrolador e para as necessidades específicas da aplicação
 * (ex: habilitar MQTT, DHCP, DNS).
 *
 * @note Este arquivo é incluído diretamente no processo de compilação da lwIP.
 * Alterações incorretas aqui podem impedir a compilação ou causar comportamento
 * de rede instável.
 *
 * @warning Modificar estas configurações sem um entendimento claro do seu impacto
 * pode levar a problemas de desempenho, consumo excessivo de memória ou falhas
 * de comunicação.
 *
 * @see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html para detalhes completos
 * sobre todas as opções de configuração disponíveis.
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

// =================================================================================
// --- Configurações Gerais do Sistema Operacional e API ---
// =================================================================================

// NO_SYS=1: Configura a lwIP para rodar sem um sistema operacional de tempo real (RTOS).
// Neste modo "bare-metal", o processamento dos pacotes de rede deve ser chamado
// periodicamente a partir do loop principal da aplicação. O SDK do Pico gerencia isso.
#ifndef NO_SYS
#define NO_SYS                      1
#endif

// LWIP_SOCKET=0: Desativa a API de Sockets compatível com Berkeley/POSIX (ex: socket(), bind(), connect()).
// Em vez dela, usamos a API nativa da lwIP (conhecida como "raw API" ou "callback API"),
// que é mais leve em termos de consumo de memória e processamento, sendo mais adequada
// para sistemas embarcados com recursos restritos.
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif

// =================================================================================
// --- Gerenciamento de Memória (Recurso Crítico) ---
// =================================================================================

#if PICO_CYW43_ARCH_POLL
// MEM_LIBC_MALLOC=1: Configura a lwIP para usar as funções padrão `malloc()` e `free()`
// da biblioteca C para toda alocação de memória. É mais simples, mas pode levar à
// fragmentação da memória com o tempo, o que é arriscado em sistemas que rodam por muito tempo.
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC=0: Usa o gerenciador de memória interno da lwIP, que é baseado
// em "pools" de memória de tamanhos fixos. É mais rápido, determinístico e evita
// fragmentação, mas é menos flexível que o malloc. Essencial para ambientes sem polling.
#define MEM_LIBC_MALLOC             0
#endif

// MEM_ALIGNMENT=4: Garante que todas as alocações de memória sejam alinhadas em
// fronteiras de 4 bytes. Isso é um requisito de hardware para muitos processadores ARM (como o RP2040)
// para evitar falhas de barramento e garantir o desempenho.
#define MEM_ALIGNMENT               4

// MEM_SIZE=4000: Define o tamanho total do heap (em bytes) que a lwIP pode
// usar para alocações dinâmicas (ex: para buffers de pacotes, estruturas de conexão).
#define MEM_SIZE                    4000

// MEMP_NUM_TCP_SEG=32: Número de segmentos TCP que podem ser enfileirados.
// Essencial para o desempenho de múltiplas conexões ou transferências de dados rápidas.
#define MEMP_NUM_TCP_SEG            32

// MEMP_NUM_ARP_QUEUE=10: Número de pacotes IP que podem ser enfileirados
// enquanto aguardam a resolução de um endereço físico (MAC) via protocolo ARP.
#define MEMP_NUM_ARP_QUEUE          10

// PBUF_POOL_SIZE=24: Número de buffers de pacotes (pbufs) disponíveis no pool principal.
// Pbufs são as estruturas de dados centrais da lwIP para manipular pacotes de rede.
// Um número insuficiente aqui pode causar descarte de pacotes sob carga de rede.
#define PBUF_POOL_SIZE              24

// =================================================================================
// --- Configurações de Protocolos de Rede ---
// =================================================================================

#define LWIP_ARP                    1      // Ativa o protocolo ARP (Address Resolution Protocol) para mapear IPs para endereços MAC na rede local. Essencial.
#define LWIP_ETHERNET               1      // Habilita o suporte geral para a camada de enlace (necessário para Wi-Fi).
#define LWIP_ICMP                   1      // Ativa o protocolo ICMP (Internet Control Message Protocol), usado para diagnóstico de rede (ex: comando 'ping').
#define LWIP_RAW                    1      // Habilita a API "Raw" (callback API), que estamos usando.
#define LWIP_DHCP                   1      // Ativa o cliente DHCP para obter um endereço IP, máscara de sub-rede e gateway automaticamente do roteador.
#define LWIP_IPV4                   1      // Habilita o suporte para o protocolo IPv4.
#define LWIP_TCP                    1      // Habilita o protocolo TCP (Transmission Control Protocol), necessário para MQTT, HTTP, etc.
#define LWIP_UDP                    1      // Habilita o protocolo UDP (User Datagram Protocol).
#define LWIP_DNS                    1      // Ativa o cliente DNS para resolver nomes de domínio (ex: "mqtt.eclipse.org") para endereços IP.
#define LWIP_TCP_KEEPALIVE          1      // Habilita o envio de pacotes "keep-alive" para detectar e fechar conexões TCP inativas, liberando recursos.

// --- Configurações Específicas do TCP ---

// TCP_WND: Tamanho da "janela de recepção" TCP (em bytes). É a quantidade de dados que o dispositivo pode receber antes de precisar enviar uma confirmação.
#define TCP_WND                     (8 * TCP_MSS)

// TCP_MSS: Maximum Segment Size. Define o maior payload (em bytes) que um segmento TCP pode carregar. Geralmente é o MTU da rede (1500) menos os cabeçalhos IP e TCP (40).
#define TCP_MSS                     1460

// TCP_SND_BUF: Tamanho do buffer de envio TCP (em bytes). Memória alocada por conexão para armazenar dados que estão aguardando para serem transmitidos.
#define TCP_SND_BUF                 (8 * TCP_MSS)

// TCP_SND_QUEUELEN: Número de pbufs que podem ser enfileirados para transmissão por conexão TCP.
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))


// =================================================================================
// --- Configurações da Interface de Rede (Netif) ---
// =================================================================================

#define LWIP_NETIF_STATUS_CALLBACK  1  // Ativa callbacks para mudanças de status da interface (ex: quando um IP é atribuído pelo DHCP).
#define LWIP_NETIF_LINK_CALLBACK    1  // Ativa callbacks para mudanças no estado do link físico (ex: Wi-Fi conectado/desconectado).
#define LWIP_NETIF_HOSTNAME         1  // Permite definir um nome de host (hostname) para o dispositivo, que pode aparecer na lista de clientes do roteador.
#define LWIP_NETCONN                0  // Desativa a API Netconn, uma camada de abstração sequencial sobre a API raw que não estamos usando.
#define LWIP_NETIF_TX_SINGLE_PBUF   1  // Otimização para interfaces que só podem enviar um pbuf de cada vez.

// =================================================================================
// --- Configurações de Estatísticas e Depuração ---
// =================================================================================

// Desativa a coleta de estatísticas para economizar memória e ciclos de CPU.
#define MEM_STATS                   0  // Estatísticas de uso de memória do heap.
#define SYS_STATS                   0  // Estatísticas do sistema (semáforos, mutexes, mailboxes).
#define MEMP_STATS                  0  // Estatísticas de uso de pools de memória.
#define LINK_STATS                  0  // Estatísticas da camada de enlace.

#define LWIP_CHKSUM_ALGORITHM       3  // Usa um algoritmo de checksum otimizado para velocidade.

// --- Configurações do DHCP ---
#define DHCP_DOES_ARP_CHECK         0  // Desativa a verificação via ARP para ver se o IP oferecido pelo DHCP já está em uso (otimização).
#define LWIP_DHCP_DOES_ACD_CHECK    0  // Desativa a verificação de conflito de endereço (ACD) via DHCP (otimização).

// Ativa as mensagens de depuração e estatísticas globais se não estivermos em modo "Release" (quando NDEBUG é definido).
#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

// --- Controle Fino de Mensagens de Depuração ---
// LWIP_DBG_OFF desliga completamente as mensagens de um módulo específico para manter o console limpo.
// Útil para focar em um problema específico (ex: ligar apenas TCP_DEBUG e DHCP_DEBUG).
#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

// =================================================================================
// --- Configurações de Timers ---
// =================================================================================

// Aumenta o número de timers de sistema disponíveis na pool de memória.
// Aplicações como MQTT, que dependem de timeouts para retransmissão e pacotes
// keep-alives para manter a conexão ativa, precisam de mais timers do que o padrão.
// Um valor baixo aqui poderia fazer com que o cliente MQTT falhasse ao tentar agendar um keep-alive.
#define MEMP_NUM_SYS_TIMEOUT         16

#endif /* __LWIPOPTS_H__ */