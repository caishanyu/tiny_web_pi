#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#define BUFFER_SIZE (4096)
#define CRLF        "\r\n"        /* HTTP协议标准换行符 */

#define EQUAL       (0)
#define NOT_EQUAL   (1)

// net
#define IPV4    (0x0800)
#define ARP     (0x0806)
#define IPV6    (0x86dd)
#define TCP     (0x6)
#define UDP     (0x17)
#define HTTP_PORT   (80)

#define NETIF_ETH0      "eth0"

#endif