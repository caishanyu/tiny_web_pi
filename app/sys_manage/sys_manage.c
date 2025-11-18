#include <string.h>
#include <stdio.h>

#include "sys_manage.h"
#include "debug.h"
#include "http.h"

static error_code sys_manage_gen_html(OUT char *buffer);

/*
    Name:   sys_manage_init
    Func:   initialize sys_manage module
    In:
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code sys_manage_init()
{
    PFM_IF_FAIL_RET(http_gen_func_register(sys_manage_gen_html));

    DBG_MAJ("sys_manage module init OK");

    return err_no_error;
}

/*
    Name:   sys_manage_get_meminfo
    In:
    Out:    meminfo
    Return:
            0 - ok
            others - fail
*/
error_code sys_manage_get_meminfo(char *meminfo)
{
    FILE *fp = NULL;
    char buffer[BUFFER_SIZE] = {};

    PFM_ENSURE_RET_VAL(meminfo, err_bad_param);

    fp = fopen(MEMINFO_DIR, "rb");
    if(!fp)
    {
        printf("open meminfo file fail");
        return err_file_opera_fail;
    }

    fread(buffer, 1, BUFFER_SIZE, fp);

    strcpy(meminfo, buffer);

    fclose(fp);

    return err_no_error;
}

/*
    Name:   sys_manage_gen_html
    Func:   generate info of sys_manage, display in web
    In:
    Out:    buffer      - sysManage info for html
    Return:
            0 - ok
            others - fail
*/
static error_code sys_manage_gen_html(OUT char *buffer)
{
    char meminfo[BUFFER_SIZE] = {};
    char *meminfo_line = NULL;
    int line = 0;

    PFM_ENSURE_RET_VAL(buffer, err_bad_param);

    PFM_IF_FAIL_RET(sys_manage_get_meminfo(meminfo));

    strcat(buffer, HEADER_1 "system memory brief" HEADER_1_END);

    strcat(buffer, PARAGRAPH);
    meminfo_line = strtok(meminfo, CRLF);
    do
    {
        ++ line;
        strcat(buffer, meminfo_line);
        strcat(buffer, LINE_BREAK);
    }while(NULL != (meminfo_line = strtok(NULL, CRLF)) && line <= 6);
    strcat(buffer, PARAGRAPH_END);

    return err_no_error;
}