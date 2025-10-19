#include <string.h>
#include <stdio.h>

#include "socket.h"
#include "http.h"

static struct socket_web_s socket_web_info = {};

/*
    Name:   socket_web_init
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
static error_code socket_web_init()
{
    int ret = 0;
    struct sockaddr_in addr = {};

    memset(&socket_web_info, 0, sizeof(struct socket_web_s));

    // create socket
    socket_web_info.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_web_info.socket_fd < 0)
    {
        DBG_ERR("socket() fail");
        goto error;
    }

    // bind socket on 0.0.0.0:10808
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(WEB_SOCK_PORT);
    ret = bind(socket_web_info.socket_fd, (const struct sockaddr*)&addr, sizeof(addr));
    if(ret < 0)
    {
        DBG_ERR("bind() fail");
        goto error;
    }

    // listen
    ret = listen(socket_web_info.socket_fd, WEB_SOCK_LISTEN_COUNT);
    if(ret < 0)
    {
        DBG_ERR("listen() fail");
        goto error;
    }

    DBG_MAJ("socket init done");

    return err_no_error;

error:
    DBG_ERR("socket init fail");
    if(socket_web_info.socket_fd > 0) 
        close(socket_web_info.socket_fd);
    return err_socket_opera_fail;
}

error_code socket_handle_connect()
{
    struct sockaddr_in client_addr = {};
    int connect_fd = 0;
    socklen_t client_len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE] = {};
    struct http_request_s http_request = {};

    DBG("Start accept connections");

    while(1)
    {
        connect_fd = accept(socket_web_info.socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if(connect_fd < 0)
        {
            DBG_ERR("connect fd error, %d", connect_fd);
            continue;
        }

        /* handle new client */
        DBG_MAJ("handle new connection");

        /* parse connect request */
        read(connect_fd, buffer, BUFFER_SIZE);
        PFM_IF_FAIL_CONTINUE(http_request_prase(buffer, &http_request));
        debug_show_http_request(&http_request);

        /* handle connection between client */
        http_request_response(connect_fd, &http_request);

        close(connect_fd);
    }

    return err_socket_opera_fail;
}

/*
    Name:   socket_web_run
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code socket_web_run()
{
    PFM_IF_FAIL_RET(socket_web_init());

    socket_handle_connect();

    return err_no_error;
}