#include <string.h>
#include <stdlib.h>
#include "list.h"

#define LIST_LOCK_UNLOCK(l, code)   do{ \
    pthread_mutex_lock(&((l)->mtx));    \
    {code}  \
    pthread_mutex_unlock(&((l)->mtx));  \
}while(0);
#define LIST_LOCK(l)    pthread_mutex_lock(&((l)->mtx))
#define LIST_UNLOCK(l)    pthread_mutex_unlock(&((l)->mtx))

/*
    Name:   list_create
    Func:   create a list
    In:     cmp - cmpare func between nodes
    Out:
    Return:
            ptr - ok
            NULL - fail
*/
struct list_s* list_create(IN OPTIONAL list_cmp_func cmp)
{
    struct list_s* l_ret = NULL;

    l_ret = (struct list_s*)malloc(sizeof(struct list_s));
    if(!l_ret)
    {
        DBG_ERR("malloc space for list fail");
        goto error;
    }
    memset(l_ret, 0, sizeof(struct list_s));

    l_ret->cmp = cmp;
    pthread_mutex_init(&l_ret->mtx, NULL);

    return l_ret;

error:

    if(l_ret)   free(l_ret);
    return NULL;
}

/*
    Name:   list_destroy
    Func:   destroy a list
    In:     l - list
    Out:
    Return:
            0 - ok
            others - fail
*/
error_code list_destroy(IN struct list_s *l)
{
    struct list_node_s *p_n = NULL;
    PFM_ENSURE_RET_VAL(l, err_bad_param);

    while(err_no_error == list_remove_head(l))      {}  // remove all node

    // 此处有个问题，可能解锁后其它线程又占用了锁，后续可优化
    pthread_mutex_destroy(&l->mtx);
    free(l);

    return err_no_error;
}

/*
    Name:   list_get_size
    Func:   get size of list
    In:     l - list
    Out:    size - ptr to size
    Return:
            0 - ok
            others - fail
*/
error_code list_get_size(IN struct list_s *l, OUT int *size)
{
    PFM_ENSURE_RET_VAL(l && size, err_bad_param);

    LIST_LOCK_UNLOCK(l, {*size = l->size;});

    return err_no_error;
}

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
error_code list_contain_data(IN struct list_s *l, IN void *data, OUT int *index)
{
    struct list_node_s *p_n = NULL;
    int idx = 0;

    PFM_ENSURE_RET_VAL(l && data && index, err_bad_param);

    LIST_LOCK(l);

    if(!l->cmp)
    {
        LIST_UNLOCK(l);
        return err_list_cmp_func_not_exist;
    }

    p_n = l->header.next;
    while(p_n)
    {
        ++ idx;
        if(0 == l->cmp(p_n->data, data))
        {
            *index = idx;
            LIST_UNLOCK(l);
            return err_no_error;
        }
        p_n = p_n->next;
    }

    LIST_UNLOCK(l);

    return err_list_data_not_exist;
}

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
error_code list_append_index(IN struct list_s *l, IN void *data, IN int index)
{
    struct list_node_s *prior = NULL;
    struct list_node_s *new_node = NULL;
    int i = 0;

    PFM_ENSURE_RET_VAL(l, err_bad_param);
    PFM_ENSURE_RET_VAL(index >= 0, err_list_position_invalid);

    new_node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if(!new_node)
    {
        return err_no_memory;
    }
    memset(new_node, 0, sizeof(struct list_node_s));
    new_node->data = data;

    LIST_LOCK(l);

    if(index > l->size)
    {
        LIST_UNLOCK(l);
        free(new_node);
        return err_list_position_invalid;
    }

    // find the prior node of index
    prior = &l->header;
    for(i = 0; i < index; ++ i)     prior = prior->next;

    // insert
    new_node->next = prior->next;
    prior->next = new_node;
    ++ l->size;

    LIST_UNLOCK(l);

    return err_no_error;
}

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
int list_append_head(struct list_s *l, void *data)
{
    return list_append_index(l, data, 0);
}

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
int list_append_tail(struct list_s *l, void *data)
{
    struct list_node_s *tail = NULL;
    struct list_node_s *new_node = NULL;
    int i = 0;

    PFM_ENSURE_RET_VAL(l, err_bad_param);

    new_node = (struct list_node_s*)malloc(sizeof(struct list_node_s));
    if(!new_node)
    {
        return err_no_memory;
    }
    memset(new_node, 0, sizeof(struct list_node_s));
    new_node->data = data;
    new_node->next = NULL;

    LIST_LOCK(l);

    // find the prior node of index
    tail = &l->header;
    while(tail->next)   tail = tail->next;

    // insert
    tail->next = new_node;
    ++ l->size;

    LIST_UNLOCK(l);

    return err_no_error;
}

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
int list_remove_index(struct list_s *l, int index)
{
    int i = 0;
    struct list_node_s *prior = NULL;
    struct list_node_s *tmp = NULL;

    PFM_ENSURE_RET_VAL(l, err_bad_param);
    PFM_ENSURE_RET_VAL(index >= 0, err_list_position_invalid);

    LIST_LOCK(l);

    if(index >= l->size)
    {
        LIST_UNLOCK(l);
        return err_list_position_invalid;
    }

    // find the prior node of index
    for(i = 0; i < index; ++ i)     prior = prior->next;
    // remove
    tmp = prior->next;
    prior->next = tmp->next;
    free(tmp);
    -- l->size;

    LIST_UNLOCK(l);

    return err_no_error;
}

/*
    Name:   list_remove_head
    Func:   remove data in head of list
    In:     l       - list
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_remove_head(struct list_s *l)
{
    return list_remove_index(l, 0);
}

/*
    Name:   list_remove_tail
    Func:   remove data in tail of list
    In:     l       - list
    Out:
    Return:
            0 - ok
            others - fail
*/
int list_remove_tail(struct list_s *l)
{
    struct list_node_s *tail = NULL;
    struct list_node_s *tmp = NULL;

    PFM_ENSURE_RET_VAL(l, err_bad_param);

    LIST_LOCK(l);

    if(0 == l->size)
    {
        LIST_UNLOCK(l);
        return err_list_position_invalid;
    }
    else if(1 == l->size)
    {
        free(l->header.next);
        l->header.next = NULL;
        l->size = 0;
        LIST_UNLOCK(l);
        return err_no_error;
    }

    // find the prior node of tail
    while(tail->next->next)     tail = tail->next;
    // remove
    tmp = tail->next;
    tail->next = NULL;
    free(tmp);
    -- l->size;

    LIST_UNLOCK(l);

    return err_no_error;
}