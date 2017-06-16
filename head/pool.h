/*************************************************************************
	> File Name:    pool.h
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年02月19日 星期日 20时17分44秒
 ************************************************************************/

#ifndef _POOL_H
#define _POOL_H

#include <stdio.h>
#include <malloc.h>

#include <unistd.h>

/*一些相关量定义*/
#define MAX_ALLOC  (128)
#define DEFAULT    (16)
#define MAX_FAILED (4)

/*类型重定义*/

typedef struct data_s  data_t;
typedef struct pool_s  pool_t;
typedef struct clean_s clean_t;

typedef void (*handle_t)(void *);

/*类型结构定义*/

struct data_s           
{
    void   *begin;
    void   *end;
    pool_t *next;
    int     failed;
};

struct clean_s
{
    handle_t handle;
    void *args;
};

struct pool_s
{
    data_t   data;
    int   max;
    pool_t * current;
    clean_t *clean;
};

/*内存池相关操作*/

pool_t *createPool(int size);
void destroyPool(pool_t *pool);

void handle_clean(pool_t *pool,handle_t func, void *args);

void *getMem(pool_t *pool, int size);

#endif
