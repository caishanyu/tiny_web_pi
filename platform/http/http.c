#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "http.h"
#include "common_def.h"
#include "socket.h"
#include "sys_manage.h"
#include "thread_pool.h"

static struct http_gen_list_s http_gen_list = {};
static struct thread_pool_s http_thread_pool = {};
static int http_init_done = 0;

/*
    Name:   http_init
    Func:   initialize http module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code http_init()
{
    memset(&http_gen_list, 0, sizeof(struct http_gen_list_s));
    http_gen_list.p_tail = &(http_gen_list.header);             // pointer to tail

    // init thread pool
    PFM_IF_FAIL_RET(thread_pool_create(WEB_SOCK_LISTEN_COUNT, &http_thread_pool));

    http_init_done = 1;

    DBG_MAJ("http module init OK");

    return err_no_error;
}

/*
    Name:   http_gen_func_register
    Func:   register html generate function
    In:     func    - function to register
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code http_gen_func_register(IN http_gen_func func)
{
    struct http_gen_list_node_s *p_new_node = NULL;

    PFM_ENSURE_RET_VAL(func && http_init_done, err_bad_param);

    p_new_node = (struct http_gen_list_node_s*)malloc(sizeof(struct http_gen_list_node_s));
    if(!p_new_node)
    {
        return err_no_memory;
    }

    p_new_node->func = func;
    p_new_node->next = NULL;
    http_gen_list.p_tail->next = p_new_node;
    http_gen_list.p_tail = p_new_node;

    return err_no_error;
}

/*
    Name:   http_request_prase
    Func:   prase the http request info
    In:     request             - buffer of http origin request
    Out:    http_request_info   - prase result
    Return:
            0 - ok
            others - fail
*/
error_code http_request_prase(IN char *request, OUT struct http_request_s *http_request_info)
{
    char *request_line = NULL;
    char *method = NULL;
    char *url = NULL;
    char *version = NULL;

    PFM_ENSURE_RET_VAL((request && http_request_info), err_bad_param);

    memset(http_request_info, 0, sizeof(struct http_request_s));

    request_line = strtok(request, CRLF);       // 第一行为请求行，格式：method url version
    if(!request_line)
    {
        DBG_ERR("Get request line fail");
        return err_http_get_req_line_fail;
    }
    method = strtok(request_line, " ");
    url = strtok(NULL, " ");
    version = strtok(NULL, " ");

    // match method
    if(!strcmp(method, "GET"))
    {
        http_request_info->method = HTTP_METHOD_GET;
    }
    else
    {
        http_request_info->method = HTTP_METHOD_UNSUPPORT;
    }

    // match version
    if(!strcmp(version, "HTTP/1.1"))
    {
        http_request_info->version = HTTP_V_1_1;
    }
    else
    {
        http_request_info->version = HTTP_V_UNSUPPORT;
    }

    // add url
    memcpy(http_request_info->url, url, strlen(url) < HTTP_URL_MAX_LEN ? strlen(url) : HTTP_URL_MAX_LEN);

    return err_no_error;
}

/*
    Name:   http_request_response
    Func:   handle http request info, and response to client
    In:     connect_fd          - file descriptor of connection
            http_request        - request info
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code http_request_response(IN int connect_fd, IN struct http_request_s *http_request)
{
    char html_content[BUFFER_SIZE*10] = {};
    char buffer[BUFFER_SIZE] = {};
    char buffer_module[BUFFER_SIZE] = {};
    struct http_gen_list_node_s *ptr = NULL;
    FILE *fp = NULL;

    PFM_ENSURE_RET_VAL((connect_fd > 0 && http_request), err_bad_param);

    // handle unsupported request

    if(0 == strcmp(http_request->url, "/"))
    {
        sprintf(html_content, "%s", DOCTYPE_HTML HTML BODY);

        // generate html from register list
        ptr = http_gen_list.header.next;
        while(ptr)
        {
            memset(buffer_module, 0, BUFFER_SIZE);
            ptr->func(buffer_module);
            strcat(html_content, buffer_module);

            ptr = ptr->next;
        }

        strcat(html_content, BODY_END HTML_END);

        // write to local file
        fp = fopen(HTML_MAIN_DIR, "wb");
        if(fp)
        {
            DBG_MAJ("write html file %s", HTML_MAIN_DIR);
            fwrite(html_content, 1, strlen(html_content), fp);
            fclose(fp);
        }

        snprintf(buffer, sizeof(buffer), 
            "HTTP/1.1 200 OK" CRLF
            "Content-Type: text/html" CRLF
            "Content-Length: %zu" CRLF
            "Connection: close" CRLF
            CRLF
            "%s",
            strlen(html_content), html_content);

        write(connect_fd, buffer, strlen(buffer));
    }
    else        // url not found
    {
        snprintf(buffer, BUFFER_SIZE, "HTTP/1.1 404 Not Found" CRLF "Content-Type: text/plain" CRLF CRLF HEADER_1 "404 Not Found" HEADER_1_END);
        write(connect_fd, buffer, strlen(buffer));
    }

    return err_no_error;
}

/* ================ debug func ================ */

void debug_show_http_request(IN struct http_request_s *http_request_info)
{
    char buffer[BUFFER_SIZE] = {};

    PFM_ENSURE_RET_VOID(http_request_info);

    strcat(buffer, "show http request structure:\r\n");

    // method
    strcat(buffer, "Method: ");
    switch(http_request_info->method)
    {
        case HTTP_METHOD_GET:
            strcat(buffer, "GET");
            break;
        default:
            break;
    }
    strcat(buffer, CRLF);

    // url
    strcat(buffer, "Url: ");
    strcat(buffer, http_request_info->url);
    strcat(buffer, CRLF);

    // version
    strcat(buffer, "Version: ");
    switch (http_request_info->version)
    {
        case HTTP_V_1_1:
            strcat(buffer, "HTTP/1.1");
            break;
        default:
            break;
    }
    strcat(buffer, CRLF);

    DBG_MAJ("%s", buffer);

    return;
}