#ifndef __PACKET_PARSER__
#define __PACKET_PARSER__

#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "debug.h"
#include "common_def.h"
#include "list.h"

#define IF_COUNT_MAX    (16)
#define IF_NAME_LEN     (16)
#define MAC_ADDR_LEN    (6)
#define REGISTER_NAME_LEN   (16)

typedef enum packet_direction
{
    packet_tx = 0,
    packet_rx,
}packet_direction;

typedef struct packet_register_info_s{
    char name[REGISTER_NAME_LEN];   // 注册条件名

    uint8_t dst_mac[MAC_ADDR_LEN];
    uint16_t ether_type;
    uint8_t ip_protocol;
    uint16_t l4_dst_port;
    uint16_t l4_src_port;

    uint32_t packet_counts;
}packet_register_info_t;

struct packet_statics_s
{
    uint32_t packet_counts;     // total counts
    uint32_t packet_bytes;      // total bytes
    uint32_t uc_counts;         // unicast
    uint32_t bc_counts;         // broadcast
    uint32_t mc_counts;         // multicast
};

struct packet_parser_s
{
    char if_name[IF_NAME_LEN];      // interface name, symbol
    char if_mac[MAC_ADDR_LEN];      // interface mac address
    int socket_fd;                  // socket fd
    struct sockaddr addr;           // socket addr
    pthread_mutex_t mtx;            // mutex
    struct packet_statics_s rx;     // rx statics
    struct packet_statics_s tx;     // tx statics

    struct list_s *packet_register_list;    // 过滤注册链表
};

/*
    Name:   packet_parser_init
    Func:   initialize packet_parser module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_init();

/*
    Name:   packet_parser_create
    Func:   initialize packet_parser
    In:     if_name         - interface name
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_create(IN const char *if_name);

/*
    Name:   packet_parser_start_work
    Func:   start work!
    In:     if_name         - interface name
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_start_work(IN const char *if_name);

/*
    Name:   packet_parser_show_statics
    Func:   show statics
    In:     ptr     - pointer to pp
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_show_statics(IN struct packet_parser_s* ptr);

/*
    Name:   packet_parser_register_filter
    Func:   for app, register filter to packet_parser
    In:     if_name         - interface name
            filter_name     - name of filters
            filters         - 0 not care
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_register_filter(
    IN const char *if_name,
    IN const char *filter_name,
    IN OPTIONAL uint8_t *dst_mac,
    IN OPTIONAL uint16_t ether_type,
    IN OPTIONAL uint8_t ip_protocol,
    IN OPTIONAL uint16_t l4_dst_port,
    IN OPTIONAL uint16_t l4_src_port
);

/*
    Name:   packet_parser_unregister_filter
    Func:   for app, unregister filter from packet_parser
    In:     if_name         - interface name
            filter_name     - filter name
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_unregister_filter(
    IN const char *if_name,
    IN const char *filter_name
);

#endif