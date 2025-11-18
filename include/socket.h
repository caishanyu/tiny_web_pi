#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/socket.h>
#include <arpa/inet.h>

#include "debug.h"
#include "common_def.h"

#define WEB_SOCK_PORT           (10808)
#define WEB_SOCK_LISTEN_COUNT   (10)

struct socket_web_s
{
    int socket_fd;
};

/*
    Name:   socket_web_run
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code socket_web_run();

#endif