/**
 * @file lwipopts.h
 * @version 1.0
 * @date 14/07/2025
 * @brief Arquivo de configuração para a pilha de rede lwIP (Lightweight IP).
 *
 * Este arquivo permite personalizar o comportamento e o consumo de memória
 * da pilha de rede. As opções aqui definidas ativam/desativam protocolos,
 * ajustam tamanhos de buffers e controlam o comportamento geral da rede.
 * Para a maioria dos projetos com o Pico W, as configurações padrão são adequadas.
 *
 * @note Este arquivo é incluído no processo de compilação da lwIP e suas
 * definições sobrescrevem os valores padrão da biblioteca.
 *
 * @see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html para detalhes completos
 * sobre todas as opções de configuração disponíveis.
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

// =================================================================================
// --- Configurações Gerais do Sistema ---
// =================================================================================

// NO_SYS=1: Configura a lwIP para rodar sem um sistema operacional em tempo real (RTOS).
// Neste modo, o processamento da pilha de rede deve ser chamado periodicamente
// a partir do loop principal da aplicação (conhecido como "main-loop polling").
#ifndef NO_SYS
#define NO_SYS                      1
#endif

// LWIP_SOCKET=0: Desativa a API de Sockets compatível com Berkeley/POSIX.
// Em vez dela, usamos a API nativa da lwIP (conhecida como "raw API" ou "callback API"),
// que é mais leve em termos de consumo de memória e mais eficiente para sistemas embarcados.
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif

// =================================================================================
// --- Gerenciamento de Memória ---
// A memória é um recurso crítico em microcontroladores.
// =================================================================================

#if PICO_CYW43_ARCH_POLL
// MEM_LIBC_MALLOC=1: Usa as funções padrão `malloc()` e `free()` da biblioteca C
// para alocação de memória. Simples, mas pode levar à fragmentação da memória.
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC=0: Usa o gerenciador de memória interno da lwIP, que é baseado
// em "pools" de memória de tamanhos fixos. É mais rápido, evita fragmentação,
// mas é menos flexível que o malloc. Essencial para ambientes sem polling.
#define MEM_LIBC_MALLOC             0
#endif

// MEM_ALIGNMENT=4: Garante que todas as alocações de memória sejam alinhadas em
// fronteiras de 4 bytes, o que é um requisito para muitos processadores ARM.
#define MEM_ALIGNMENT               4

// MEM_SIZE=4000: Define o tamanho total do heap (pilha de memória) que a lwIP
// pode usar para alocações dinâmicas (em bytes).
#define MEM_SIZE                    4000

// MEMP_NUM_TCP_SEG=32: Número de segmentos TCP que podem ser enfileirados.
// Essencial para o desempenho de múltiplas conexões ou transferências rápidas.
#define MEMP_NUM_TCP_SEG            32

// MEMP_NUM_ARP_QUEUE=10: Número de pacotes IP que podem ser enfileirados
// aguardando a resolução de um endereço ARP.
#define MEMP_NUM_ARP_QUEUE          10

// PBUF_POOL_SIZE=24: Número de buffers de pacotes (pbufs) disponíveis no pool principal.
// Pbufs são as estruturas de dados centrais da lwIP para manipular pacotes de rede.
#define PBUF_POOL_SIZE              24

// =================================================================================
// --- Configurações de Protocolos de Rede ---
// =================================================================================

#define LWIP_ARP                    1      // Ativa o protocolo ARP (Address Resolution Protocol) para mapear IPs para endereços MAC.
#define LWIP_ETHERNET               1      // Habilita o suporte geral para a camada de enlace Ethernet.
#define LWIP_ICMP                   1      // Ativa o protocolo ICMP (Internet Control Message Protocol), usado para o comando 'ping'.
#define LWIP_RAW                    1      // Habilita a API "Raw", que permite a aplicação interagir diretamente com pacotes IP.
#define LWIP_DHCP                   1      // Ativa o cliente DHCP para obter um endereço IP, máscara de sub-rede e gateway automaticamente.
#define LWIP_IPV4                   1      // Habilita o suporte para o protocolo IPv4.
#define LWIP_TCP                    1      // Habilita o protocolo TCP (Transmission Control Protocol), necessário para HTTP, MQTT, etc.
#define LWIP_UDP                    1      // Habilita o protocolo UDP (User Datagram Protocol).
#define LWIP_DNS                    1      // Ativa o cliente DNS para resolver nomes de domínio (ex: "google.com") para endereços IP.
#define LWIP_TCP_KEEPALIVE          1      // Habilita o envio de pacotes "keep-alive" para detectar e fechar conexões TCP inativas.

// --- Configurações Específicas do TCP ---

// TCP_WND: Tamanho da "janela de recepção" TCP (em bytes). Um valor maior pode melhorar o desempenho em redes de alta latência.
#define TCP_WND                     (8 * TCP_MSS)

// TCP_MSS: Maximum Segment Size (Tamanho máximo do segmento). Define o maior payload que um segmento TCP pode carregar.
#define TCP_MSS                     1460

// TCP_SND_BUF: Tamanho do buffer de envio TCP (em bytes).
#define TCP_SND_BUF                 (8 * TCP_MSS)

// TCP_SND_QUEUELEN: Número de pbufs que podem ser enfileirados para transmissão por conexão TCP.
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))


// =================================================================================
// --- Configurações da Interface de Rede (Netif) ---
// =================================================================================

#define LWIP_NETIF_STATUS_CALLBACK  1  // Ativa callbacks para mudanças de status da interface (ex: IP alterado).
#define LWIP_NETIF_LINK_CALLBACK    1  // Ativa callbacks para mudanças no estado do link físico (ex: cabo conectado/desconectado).
#define LWIP_NETIF_HOSTNAME         1  // Permite definir um nome de host (hostname) para o dispositivo.
#define LWIP_NETCONN                0  // Desativa a API Netconn, uma camada de abstração sequencial sobre a API raw.
#define LWIP_NETIF_TX_SINGLE_PBUF   1  // Otimização para interfaces que podem enviar um pbuf de cada vez.

// =================================================================================
// --- Configurações de Estatísticas e Debug ---
// =================================================================================

#define MEM_STATS                   0  // Desativa estatísticas de uso de memória do heap.
#define SYS_STATS                   0  // Desativa estatísticas do sistema (semáforos, mutexes, mailboxes).
#define MEMP_STATS                  0  // Desativa estatísticas de uso de pools de memória.
#define LINK_STATS                  0  // Desativa estatísticas da camada de enlace.

// #define ETH_PAD_SIZE                2 // Padding opcional para alinhar o payload IP. Desativado por padrão.
#define LWIP_CHKSUM_ALGORITHM       3  // Algoritmo de checksum otimizado.

// --- Configurações do DHCP ---

#define DHCP_DOES_ARP_CHECK         0  // Desativa a verificação via ARP para ver se o IP oferecido pelo DHCP já está em uso.
#define LWIP_DHCP_DOES_ACD_CHECK    0  // Desativa a verificação de conflito de endereço (ACD) via DHCP.

// Ativa as mensagens de depuração e estatísticas globais se não estivermos em modo "Release" (NDEBUG não definido).
#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

// --- Controle Fino de Mensagens de Debug ---
// LWIP_DBG_OFF desliga completamente as mensagens de um módulo específico para manter o console limpo.
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

// MEMP_NUM_SYS_TIMEOUT: Aumenta o número de timers de sistema disponíveis.
// Aplicações como MQTT, que dependem de timeouts e keep-alives, precisam de mais timers
// do que o padrão.
#define MEMP_NUM_SYS_TIMEOUT         16

#endif /* __LWIPOPTS_H__ */