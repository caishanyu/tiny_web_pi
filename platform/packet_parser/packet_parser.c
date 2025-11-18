#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "packet_parser.h"
#include "http.h"

PRIVATE struct packet_parser_s g_packet_parser_arr[IF_COUNT_MAX] = {};
PRIVATE pthread_mutex_t g_packet_parser_arr_mtx = {};

#define G_PACKET_PARSER_ARR_LOCK()      pthread_mutex_lock(&g_packet_parser_arr_mtx);
#define G_PACKET_PARSER_ARR_UNLOCK()    pthread_mutex_unlock(&g_packet_parser_arr_mtx);

#define PACKET_DETAIL   0

/*
    Name:   packet_register_cmp_info_func
    Func:   compare info function for packet register list
    In:     dp  - data of packet
            dr  - data of register func
    Out:
    Return:
            0 - equal
            others - no euqal
*/
PRIVATE int packet_register_cmp_info_func(void *dp, void *dr)
{
    packet_register_info_t *packet = NULL;
    packet_register_info_t *register_info = NULL;
    char mac_empty[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    if(!dp && !dr)  return EQUAL;
    else if(!dp || !dr) return NOT_EQUAL;

    packet = (packet_register_info_t*)dp;
    register_info = (packet_register_info_t*)dr;

    // 注册了dst_mac
    if(0 != memcmp(register_info->dst_mac, mac_empty, MAC_ADDR_LEN) && 0 != memcmp(packet->dst_mac, register_info->dst_mac, MAC_ADDR_LEN))
    {
        return NOT_EQUAL;
    }
    // 注册了ether_type
    if(0 != register_info->ether_type && packet->ether_type != register_info->ether_type)
    {
        return NOT_EQUAL;
    }
    // 注册了ip_protocol
    if(0 != register_info->ip_protocol && packet->ip_protocol != register_info->ip_protocol)
    {
        return NOT_EQUAL;
    }
    // 注册了l4_dst_port
    if(0 != register_info->l4_dst_port && packet->l4_dst_port != register_info->l4_dst_port)
    {
        return NOT_EQUAL;
    }
    // 注册了l4_src_port
    if(0 != register_info->l4_src_port && packet->l4_src_port != register_info->l4_src_port)
    {
        return NOT_EQUAL;
    }

    return EQUAL;
}

/*
    Name:   g_packet_parser_insert
    Func:   insert a struct into array
    In:     packet_parser - pointer to struct
    Out:
    Return:
            0 - ok
            others - fail
*/
PRIVATE error_code g_packet_parser_insert(IN struct packet_parser_s *packet_parser)
{
    int i = 0;

    PFM_ENSURE_RET_VAL(packet_parser, err_bad_param);

    G_PACKET_PARSER_ARR_LOCK();

    for(i = 0; i < IF_COUNT_MAX; ++ i)
    {
        if(strlen(g_packet_parser_arr[i].if_name) > 0)
        {
            continue;
        }

        // repeated
        if(0 == strcmp(g_packet_parser_arr[i].if_name, packet_parser->if_name))
        {
            G_PACKET_PARSER_ARR_UNLOCK();
            return err_packet_parser_exist;
        }

        // find first available
        memcpy(&g_packet_parser_arr[i], packet_parser, sizeof(struct packet_parser_s));
        G_PACKET_PARSER_ARR_UNLOCK();
        return err_no_error;
    }

    G_PACKET_PARSER_ARR_UNLOCK();

    return err_packet_parser_full;
}

/*
    Name:   g_packet_parser_remove
    Func:   remove a struct from array
    In:     if_name - interface name
    Out:
    Return:
            0 - ok
            others - fail
*/
PRIVATE error_code g_packet_parser_remove(IN const char *if_name)
{
    int i = 0;

    PFM_ENSURE_RET_VAL(if_name, err_bad_param);

    G_PACKET_PARSER_ARR_LOCK();

    for(i = 0; i < IF_COUNT_MAX; ++ i)
    {
        // find
        if(0 == strcmp(if_name, g_packet_parser_arr[i].if_name))
        {
            memset(&g_packet_parser_arr[i], 0, sizeof(struct packet_parser_s));
            G_PACKET_PARSER_ARR_UNLOCK();
            return err_no_error;
        }
    }

    G_PACKET_PARSER_ARR_UNLOCK();

    return err_packet_parser_not_exist;
}

/*
    Name:   g_packet_parser_get_ptr
    Func:   get pointer to packet_parser from array
    In:     if_name - interface name
    Out:
    Return:
            ptr - OK
            NULL - FAIL
*/
PRIVATE struct packet_parser_s* g_packet_parser_get_ptr(IN const char *if_name)
{
    int i = 0;

    PFM_ENSURE_RET_VAL(if_name, NULL);

    G_PACKET_PARSER_ARR_LOCK();

    for(i = 0; i < IF_COUNT_MAX; ++ i)
    {
        // find
        if(0 == strcmp(if_name, g_packet_parser_arr[i].if_name))
        {
            G_PACKET_PARSER_ARR_UNLOCK();
            return &g_packet_parser_arr[i];
        }
    }

    G_PACKET_PARSER_ARR_UNLOCK();

    return NULL;
}

PRIVATE error_code packet_parser_parse(
    IN struct packet_parser_s *packet_parser,
    IN char *buffer,
    IN int recv_bytes
)
{
    int i = 0;
    char dst_mac[MAC_ADDR_LEN] = {};
    char src_mac[MAC_ADDR_LEN] = {};
    char bc_mac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    packet_direction direction = 0;
    struct packet_statics_s *p_statics = NULL;
    uint16_t ether_type = 0;
    uint8_t ip_protocol = 0;
    uint16_t l4_dst_port = 0;
    uint16_t l4_src_port= 0;
    struct list_node_s *node = NULL;
    struct packet_register_info_s *data = NULL;
    struct packet_register_info_s cmp = {};

    PFM_ENSURE_RET_VAL(packet_parser && buffer, err_bad_param);

#if PACKET_DETAIL       // 打印具体内容
    printf("Display packet: \r\n");
    for(i = 0; i < recv_bytes; ++ i)
    {
        printf("%02x ", buffer[i]);
        if((i+1)%8 == 0)    printf("\t");
        if((i+1)%16 == 0)   printf("\r\n");
    }
    printf("\r\n");
#endif

    pthread_mutex_lock(&packet_parser->mtx);

    /* analyze field */
    // mac
    memcpy(dst_mac, buffer, MAC_ADDR_LEN);
    memcpy(src_mac, buffer+6, MAC_ADDR_LEN);
    if(0 == memcmp(src_mac, packet_parser->if_mac, MAC_ADDR_LEN))
    {
        direction = packet_tx;
        p_statics = &packet_parser->tx;
    }
    else
    {
        direction = packet_rx;
        p_statics = &packet_parser->rx;
    }
    if(0 == memcmp(dst_mac, bc_mac, MAC_ADDR_LEN))
    {
        p_statics->bc_counts += 1;
    }
    else if(1 == (dst_mac[0] & 0x1))
    {
        p_statics->mc_counts += 1;
    }
    else
    {
        p_statics->uc_counts += 1;
    }
    // ether_type
    ether_type = ((buffer[12] << 8) | (buffer[13]));
    // ip_protocol
    if(IPV4 == ether_type)
    {
        ip_protocol = (buffer[23]);
        // l4_port
        if(TCP == ip_protocol || UDP == ip_protocol)
        {
            l4_src_port = ((buffer[34] << 8) | (buffer[35]));
            l4_dst_port = ((buffer[36] << 8) | (buffer[37]));
        }
    }

    memcpy(cmp.dst_mac, dst_mac, MAC_ADDR_LEN);
    cmp.ether_type = ether_type;
    cmp.ip_protocol = ip_protocol;
    cmp.l4_dst_port = l4_dst_port;
    cmp.l4_src_port = l4_src_port;
    // check filter list
    list_for_each(packet_parser->packet_register_list, node, data)
    {
        if(0 == packet_parser->packet_register_list->cmp(&cmp, (struct packet_register_info_s*)data))
        {
            ++ ((struct packet_register_info_s*)data)->packet_counts;
        }
    }

    p_statics->packet_counts += 1;
    p_statics->packet_bytes += recv_bytes;

    pthread_mutex_unlock(&packet_parser->mtx);

#if 0
    DBG("%s dirction packet bytes %d, src_mac %02x:%02x:%02x:%02x:%02x:%02x, dst_mac %02x:%02x:%02x:%02x:%02x:%02x, ether_type %04x, ip_protocol %02x, l4_src_port %04x, l4_dst_port %04x", 
        direction == packet_tx ? "tx" : "rx" , recv_bytes,
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],
        ether_type, ip_protocol, l4_src_port, l4_dst_port);
#endif

    return err_no_error;
}

PRIVATE error_code packet_parser_gen_html(OUT char *buffer)
{
    char pp_statics[BUFFER_SIZE] = {};
    int i = 0;

    PFM_ENSURE_RET_VAL(buffer, err_bad_param);

    memset(buffer, 0, BUFFER_SIZE);
    strcat(buffer, HEADER_1 "packet_parser" HEADER_1_END);

    G_PACKET_PARSER_ARR_LOCK();

    for(i = 0; i < IF_COUNT_MAX; ++ i)
    {
        if(strlen(g_packet_parser_arr[i].if_name) == 0)
        {
            continue;
        }
        else
        {
            pthread_mutex_lock(&g_packet_parser_arr[i].mtx);
            strcat(buffer, PARAGRAPH);
            strcat(buffer, BOLD "rx static" BOLD_END LINE_BREAK);
            
            strcat(buffer, PARAGRAPH_END);
            pthread_mutex_unlock(&g_packet_parser_arr[i].mtx);
        }
    }

    G_PACKET_PARSER_ARR_UNLOCK();

    return err_no_error;
}

/* ========== public function ==========*/

/*
    Name:   packet_parser_init
    Func:   initialize packet_parser module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_init()
{
    pthread_mutex_init(&g_packet_parser_arr_mtx, NULL);
    memset(g_packet_parser_arr, 0, sizeof(struct packet_parser_s) * IF_COUNT_MAX);

    PFM_IF_FAIL_RET(http_gen_func_register(packet_parser_gen_html));

    DBG_MAJ("packet_parser module init ok");

    return err_no_error;
}

/*
    Name:   packet_parser_create
    Func:   initialize packet_parser
    In:     if_name         - interface name
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_create(IN const char *if_name)
{
    error_code ret = err_no_error;
    unsigned int if_index = 0;
    struct sockaddr_ll sll = {};
    socklen_t sll_size = sizeof(sll);
    struct packet_parser_s packet_parser = {};
    int mac_socket_fd = 0;
    struct ifreq ifr = {};
    struct sigevent sev = {};
    struct itimerspec its = {};
    sigset_t mask = {};

    PFM_ENSURE_RET_VAL(if_name, err_bad_param);

    packet_parser.packet_register_list = list_create(packet_register_cmp_info_func);
    if(!packet_parser.packet_register_list)
    {
        ret = err_list_create_fail;
        goto error;
    }

    pthread_mutex_init(&packet_parser.mtx, NULL);
    strcpy(packet_parser.if_name, if_name);
    packet_parser.socket_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(packet_parser.socket_fd < 0)
    {
        DBG_ERR("create socket fail");
        ret = err_socket_opera_fail;
        goto error;
    }

    if_index = if_nametoindex(if_name);
    if(!if_index)
    {
        DBG_ERR("get interface index fail");
        ret = err_packet_parser_opera_fail;
        goto error;
    }
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_index;

    ret = bind(packet_parser.socket_fd, (struct sockaddr*)&sll, sizeof(sll));
    if(ret < 0)
    {
        DBG_ERR("bind socket fail");
        ret = err_socket_opera_fail;
        goto error;
    }

    memcpy(&packet_parser.addr, (struct sockaddr*)&sll, sizeof(struct sockaddr));

    // get interface mac address
    mac_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(-1 == mac_socket_fd)
    {
        ret = err_socket_opera_fail;
        goto error;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ - 1);
    if(-1 == ioctl(mac_socket_fd, SIOCGIFHWADDR, &ifr))
    {
        ret = err_file_opera_fail;
        goto error;
    }
    memcpy(packet_parser.if_mac, (char*)ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);
    DBG("interface mac: %02x:%02x:%02x:%02x:%02x:%02x", 
        packet_parser.if_mac[0], packet_parser.if_mac[1], 
        packet_parser.if_mac[2], packet_parser.if_mac[3], 
        packet_parser.if_mac[4], packet_parser.if_mac[5]);

    if(err_no_error != (ret = g_packet_parser_insert(&packet_parser)))
    {
        goto error;
    }

    DBG("packet_parser on if: %s init ok", if_name);

    return ret;

error:
    if(mac_socket_fd)   close(mac_socket_fd);
    if(packet_parser.socket_fd > 0)    close(packet_parser.socket_fd);
    pthread_mutex_destroy(&packet_parser.mtx);
    return ret;
}

/*
    Name:   packet_parser_start_work
    Func:   start work!
    In:     if_name         - interface name
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_start_work(IN const char *if_name)
{
    struct packet_parser_s *ptr = NULL;
    ssize_t recv_bytes = 0;
    socklen_t sll_size = 0;

    char buffer[BUFFER_SIZE] = {};

    ptr = g_packet_parser_get_ptr(if_name);
    if(!ptr)
    {
        return err_packet_parser_not_exist;
    }

    while(1)
    {
        recv_bytes = recvfrom(ptr->socket_fd, buffer, sizeof(buffer), 0, &ptr->addr, &sll_size);
        if(recv_bytes < 0)
        {
            DBG_ERR("recvfrom fail");
            continue;
        }

        packet_parser_parse(ptr, buffer, recv_bytes);
    }

    return err_no_error;
}

/*
    Name:   packet_parser_show_statics
    Func:   show statics
    In:     ptr     - pointer to pp
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code packet_parser_show_statics(IN struct packet_parser_s* ptr)
{
    struct list_node_s *node = NULL;
    packet_register_info_t *data = NULL;

    PFM_ENSURE_RET_VAL(ptr, err_bad_param);

    pthread_mutex_lock(&ptr->mtx);

    printf("========>>>>\r\n");
    printf("rx: total counts %d, %.3f(KB)", ptr->rx.packet_counts, (double)ptr->rx.packet_bytes / 1024.0);
    printf("| uc %d, bc %d, mc %d\r\n", ptr->rx.uc_counts, ptr->rx.bc_counts, ptr->rx.mc_counts);
    printf("tx: total counts %d, %.3f(KB)", ptr->tx.packet_counts, (double)ptr->tx.packet_bytes / 1024.0);
    printf("| uc %d, bc %d, mc %d\r\n", ptr->tx.uc_counts, ptr->tx.bc_counts, ptr->tx.mc_counts);
    printf("register filters as follow:\r\n");
    list_for_each(ptr->packet_register_list, node, data)
    {
        printf("%s: counts %d\r\n", ((packet_register_info_t*)data)->name, ((packet_register_info_t*)data)->packet_counts);
    }
    printf("<<<<========\r\n");

    pthread_mutex_unlock(&ptr->mtx);

    return err_no_error;
}

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
)
{
    struct packet_parser_s* pp = NULL;
    packet_register_info_t* pri = NULL;
    error_code ret = 0;
    struct list_node_s *node = NULL;
    packet_register_info_t *data = NULL;

    PFM_ENSURE_RET_VAL(if_name && filter_name, err_bad_param);

    if(NULL == dst_mac && 0 == ether_type && 0 == ip_protocol && 0 == l4_dst_port && 0 == l4_src_port)
        return err_bad_param;

    pri = (packet_register_info_t*)malloc(sizeof(packet_register_info_t));
    if(!pri)    return err_no_memory;
    memset(pri, 0, sizeof(packet_register_info_t));
    
    memcpy(pri->name, filter_name, REGISTER_NAME_LEN - 1);
    if(dst_mac) memcpy(pri->dst_mac, dst_mac, MAC_ADDR_LEN);
    pri->ether_type = ether_type;
    pri->ip_protocol = ip_protocol;
    pri->l4_dst_port = l4_dst_port;
    pri->l4_src_port = l4_src_port;

    pp = g_packet_parser_get_ptr(if_name);
    if(!pp)
    {
        ret = err_packet_parser_not_exist;
        goto done;
    }

    pthread_mutex_lock(&pp->mtx);
    // check if filter name exist
    list_for_each(pp->packet_register_list, node, data)
    {
        if(0 == strncmp(filter_name, ((packet_register_info_t*)data)->name, REGISTER_NAME_LEN-1))
        {
            ret = err_packet_parser_filter_name_exist;
            pthread_mutex_unlock(&pp->mtx);
            goto done;
        }
    }
    // append to tail
    ret = list_append_tail(pp->packet_register_list, (void*)pri);
    pthread_mutex_unlock(&pp->mtx);

done:
    if(err_no_error != ret)     free(pri);
    return ret;
}

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
)
{
    struct packet_parser_s *pp = NULL;
    struct list_node_s *node = NULL;
    packet_register_info_t *data = NULL;
    int index = 0;
    int find_flag = 0;
    error_code ret = 0;

    PFM_ENSURE_RET_VAL(if_name && filter_name, err_bad_param);

    pp = g_packet_parser_get_ptr(if_name);
    if(!pp) return err_packet_parser_not_exist;

    pthread_mutex_lock(&pp->mtx);

    list_for_each(pp->packet_register_list, node, data)
    {
        // find node to del, prior
        if(0 == strncmp(filter_name, ((packet_register_info_t*)data)->name, REGISTER_NAME_LEN-1))
        {
            find_flag = 1;
            break;
        }
        ++ index;
    }

    if(find_flag)   ret = list_remove_index(pp->packet_register_list, index);
    else            ret = err_packet_parser_filter_name_not_exist;

    pthread_mutex_unlock(&pp->mtx);

    return ret;
}