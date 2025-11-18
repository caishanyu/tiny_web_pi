#ifndef __LIST_H__
#define __LIST_H__

#include <pthread.h>
#include "debug.h"
#include "list.h"

typedef int (*list_cmp_func)(void *d1, void *d2);

// list node
struct list_node_s
{
    void *data;
    struct list_node_s *next;
};

// list
struct list_s
{
    struct list_node_s header;
    int size;
    pthread_mutex_t mtx;
    list_cmp_func cmp;       // compare data of nodes
};

// 遍历链表内部节点，需要手动加锁
// list - 指向链表的指针，输入参数
// node - 指向当前节点，供使用
// in_data - 指向当前节点内的数据，供使用
#define list_for_each(list, node, in_data)  \
    for( (node) = (list)->header.next, ((in_data) = NULL);    \
        NULL != (node) && ((in_data)=(node)->data, 1); \
        (node) = (node)->next, ((in_data) = NULL))

// 遍历链表内部节点，包括header，需要手动加锁
// list - 指向链表的指针，输入参数
// node - 指向当前节点，供使用
// in_data - 指向当前节点内的数据，供使用
#define list_for_each_from_header(list, node, in_data)  \
    for( (node) = &((list)->header), ((in_data) = NULL);    \
        NULL != (node) && ((in_data)=(node)->data, 1); \
        (node) = (node)->next, ((in_data) = NULL))

/*
    Name:   list_create
    Func:   create a list
    In:     cmp - cmpare func between nodes
    Out:
    Return:
            ptr - ok
            NULL - fail
*/
struct list_s* list_create(IN OPTIONAL list_cmp_func cmp);

/*
    Name:   list_destroy
    Func:   destroy a list
    In:     l - list
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code list_destroy(IN struct list_s *l);

// list get
/*
    Name:   list_get_size
    Func:   destroy a list
    In:     l - list
    Out:    size - ptr to size
    Return:
            0 - ok
            others - fail
*/
error_code list_get_size(IN struct list_s *l, OUT int *size);

/*
    Name:   list_contain_data
    Func:   check if list contain data
    In:     l       - list
            data    - pointer to data
    Out:    index   - index of data, if exist
    Return:
            0 - ok
            others - fail
*/
error_code list_contain_data(IN struct list_s *l, IN void *data, OUT int *index);

// list add
/*
    Name:   list_append_index
    Func:   insert data in index position
    In:     l       - list
            data    - pointer to data
            index   - index to insert, start from 0
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code list_append_index(IN struct list_s *l, IN void *data, IN int index);

/*
    Name:   list_append_head
    Func:   insert data in head of list
    In:     l       - list
            data    - pointer to data
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_append_head(struct list_s *l, void *data);

/*
    Name:   list_append_tail
    Func:   insert data in tail of list
    In:     l       - list
            data    - pointer to data
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_append_tail(struct list_s *l, void *data);

// list del
/*
    Name:   list_remove_index
    Func:   remove data in index of list
    In:     l       - list
            index   - position
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_remove_index(struct list_s *l, int index);

/*
    Name:   list_remove_head
    Func:   remove data in head of list
    In:     l       - list
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_remove_head(struct list_s *l);

/*
    Name:   list_remove_tail
    Func:   remove data in tail of list
    In:     l       - list
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_remove_tail(struct list_s *l);

#endif