#include <stdio.h>

#include "sys_manage.h"
#include "debug.h"
#include "socket.h"

int main()
{
    /*================ sys_init ================*/
    // platform init
    PFM_IF_FAIL_RET(http_init());

    // app init
    PFM_IF_FAIL_RET(sys_time_init());
    PFM_IF_FAIL_RET(sys_manage_init());

    /*================ sys_run ================*/

    // run web server
    socket_web_run();

    return 0;
}