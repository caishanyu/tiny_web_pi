#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdio.h>
#include "debug.h"

#define HTTP_URL_MAX_LEN (64)

#define DOCTYPE_HTML    "<!DOCTYPE html>"
#define HTML            "<html>"
#define HTML_END        "</html>"
#define BODY            "<body>"
#define BODY_END        "</body>"
#define PARAGRAPH       "<p>"
#define PARAGRAPH_END   "</p>"
#define LINE_BREAK      "<br>"
#define HEADER_1        "<h1>"
#define HEADER_1_END    "</h1>"
#define HEADER_2        "<h2>"
#define HEADER_2_END    "</h2>"
#define HEADER_3        "<h3>"
#define HEADER_3_END    "</h3>"
#define HEADER_4        "<h4>"
#define HEADER_4_END    "</h4>"
#define HEADER_5        "<h5>"
#define HEADER_5_END    "</h5>"
#define HEADER_6        "<h6>"
#define HEADER_6_END    "</h6>"
#define BOLD            "<b>"
#define BOLD_END        "</b>"
#define ITALIC          "<i>"
#define ITALIC_END      "</i>"

typedef error_code (*http_gen_func)(OUT char *buffer);

struct http_gen_list_node_s
{
    http_gen_func func;
    struct http_gen_list_node_s *next;
};

struct http_gen_list_s
{
    struct http_gen_list_node_s header;
    struct http_gen_list_node_s *p_tail;
};

typedef enum
{
    HTTP_METHOD_GET = 1,

    HTTP_METHOD_UNSUPPORT,
}http_method_e;

typedef enum
{
    HTTP_V_1_1 = 1,

    HTTP_V_UNSUPPORT,
}http_version_e;

struct http_request_s
{
    http_method_e method;
    char url[HTTP_URL_MAX_LEN];
    http_version_e version;
};

/*
    Name:   http_init
    Func:   initialize http module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code http_init();

/*
    Name:   http_gen_func_register
    Func:   register html generate function
    In:     func    - function to register
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code http_gen_func_register(IN http_gen_func func);

/*
    Name:   http_request_prase
    Func:   prase the http request info
    In:     request             - buffer of http origin request
    Out:    http_request_info   - prase result
    Return:
            0 - ok
            others - fail
*/
error_code http_request_prase(IN char *request, OUT struct http_request_s *http_request_info);

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
error_code http_request_response(IN int connect_fd, IN struct http_request_s *http_request);

/* ================ debug func ================ */

void debug_show_http_request(IN struct http_request_s *http_request_info);

#endif