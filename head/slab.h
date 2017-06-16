/*************************************************************************
	> File Name:    slab.h
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年04月30日 星期日 18时20分54秒
 ************************************************************************/

#ifndef _SLAB_H
#define _SLAB_H

#include <pool.h>

extern int pagesize;

/*slab状态量的定义*/
#define IS_BUSY   (0x01)
#define IS_FULL   (0x02)
#define IS_FREE   (0x04)

#define BITE_FREE (0x0)

/*属性值定义*/
#define SLAB_POOL_SIZE      (128*pagesize)
#define SLAB_TEMP_POOL_SIZE (4*pagesize)
#define MIN_ALLOC           (32)
#define MIN_SHIFT           (5)
#define SLAB_MAX_ALLOC           (pagesize/2)

/*类型的重定义*/
typedef struct page_s page_t;
typedef struct slot_s slot_t;
typedef struct slab_s slab_t;

/*page页面的管理结构*/
struct page_s
{
    char    type;
    char    bitmap;
    page_t *prev;
    page_t *next;
};

/*slot结构定义*/
struct slot_s
{
    page_t page;
    char   shift;
};

/*slab管理结构*/
struct slab_s
{
    void *begin;
    void *end;
    int min_alloc;
    int max_alloc;
    
    slot_t *slots;
    page_t *pages;
 
    pool_t *slab_pool;
    pool_t *pool;
};

/*外部接口声明*/
slab_t *createSlab(int alloc_size);
void destorySlab(slab_t *slab);
void *allocBySlab(slab_t *slab, int size);
void freeBySlab(slab_t *slab, void *ptr);

#endif
