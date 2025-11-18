#ifndef __DEBUG_H__
#define __DEBUG_H__

/* 标识入参or出参 */
#define IN
#define OUT
/* 可选参数 */
#define OPTIONAL

/* 私有属性 */
#define PRIVATE static

/* 错误码 */
typedef enum
{
    err_no_error = 0,       // ok

    err_bad_param,          // 参数错误
    err_buffer_oof,         // 缓冲区不足
    err_no_memory,          // 内存不足

    err_file_opera_fail,    // 文件操作失败

    err_socket_opera_fail,  // socket操作失败

    err_http_get_req_line_fail, // 获取http请求行失败

    err_pthread_opera_fail,     // 线程操作失败
    err_thread_pool_not_init,   // 线程池未初始化

    err_packet_parser_opera_fail,   // 报文解析器操作失败
    err_packet_parser_full,         // 解析器已满
    err_packet_parser_exist,        // 解析器已存在
    err_packet_parser_not_exist,    // 解析器不存在
    err_packet_parser_filter_name_exist,        // 过滤条件存在
    err_packet_parser_filter_name_not_exist,    // 过滤条件不存在

    err_list_create_fail,           // 链表创建失败
    err_list_cmp_func_not_exist,    // 比较函数不存在
    err_list_data_not_exist,        // 链表中不存在数据
    err_list_position_invalid,      // 链表位置非法

    err_unknown,            // 未知
}error_code;

/* debug打印 */
#define RED_CLR     "\033[31m"
#define GREEN_CLR   "\033[32m"
#define RESET_CLR   "\033[0m"

/* debug等级 */
typedef enum
{
    dbg_normal,
    dbg_major,
    dbg_error,
}dbg_level;

void dbg(dbg_level lev, const char *func, int line, char *fmt, ...);

/* debug */
#define DBG(fmt, args...)       dbg(dbg_normal, __func__, __LINE__, fmt, ##args);
#define DBG_MAJ(fmt, args...)   dbg(dbg_major, __func__, __LINE__, fmt, ##args);
#define DBG_ERR(fmt, args...)   dbg(dbg_error, __func__, __LINE__, fmt, ##args);

/* 执行函数，如果返回值不为err_no_error，则返回 */
#define PFM_IF_FAIL_RET(func) do{  \
    error_code _ret = (func); \
    if(err_no_error != _ret)    \
    {   \
        DBG_ERR("Call %s fail ret %d", #func, _ret);  \
        return _ret; \
    }   \
}while(0);
/* 执行函数，如果返回值不为err_no_error，则直接返回，用于返回类型void的场合 */
#define PFM_IF_FAIL_RET_VOID(func) do{  \
    error_code _ret = func; \
    if(err_no_error != _ret)    \
    {   \
        DBG_ERR("Call %s fail ret %d", #func, _ret);  \
        return; \
    }   \
}while(0);
/* 执行函数，如果返回值不为err_no_error，则continue */
#define PFM_IF_FAIL_CONTINUE(func) {    \
    error_code _ret = (func); \
    if(err_no_error != _ret)    \
    {   \
        DBG_ERR("Call %s fail ret %d", #func, _ret);  \
        continue;   \
    }   \
}

/* 判断表达式是否真，失败则返回表达式结果 */
#define PFM_ENSURE_RET(cond)   do {    \
    int _ret = (cond);  \
    if(!_ret) return (r_et);     \
}while(0);
/* 判断表达式是否真，失败则返回指定值 */
#define PFM_ENSURE_RET_VAL(cond, ret)   do {    \
    if(!(cond))     return (ret);     \
}while(0);
/* 判断表达式是否真，失败则返回，用于返回值void的场景 */
#define PFM_ENSURE_RET_VOID(cond)   do {    \
    if(!(cond))     return;     \
}while(0);

#endif