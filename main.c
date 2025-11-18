#include <stdio.h>

#include "sys_manage.h"
#include "sys_time.h"
#include "debug.h"
#include "socket.h"
#include "http.h"
#include "packet_parser.h"

PRIVATE pthread_t tid_pp = 0;
PRIVATE void* pp_thread(void *args)
{
    packet_parser_start_work(NETIF_ETH0);
    return NULL;
}

int main()
{
    /*================ sys_init ================*/
    // platform init
    // http module init
    PFM_IF_FAIL_RET(http_init());
    // create pp for eth0
    PFM_IF_FAIL_RET(packet_parser_init());
    PFM_IF_FAIL_RET(packet_parser_create(NETIF_ETH0));

    // app init
    PFM_IF_FAIL_RET(sys_time_init());
    PFM_IF_FAIL_RET(sys_manage_init());

    /*================ sys_run ================*/

    // create thread to run packet_praser
    pthread_create(&tid_pp, NULL, pp_thread, NULL);

    // run web server
    socket_web_run();

    return 0;
}