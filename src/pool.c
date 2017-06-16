/*************************************************************************
	> File Name:    pool.c
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年02月25日 星期六 17时22分42秒
 ************************************************************************/

#include <pool.h>

/*相关字段的定义*/
int pagesize    = 0;
int maxalloc    = 0;
int defaultsize = 0;
int maxfailed   = 0;

/*本文件的内部接口声明*/

static void    resetPool(pool_t *pool, int size);
static pool_t *addNode(pool_t *pool, int size);
static void    resetNode(pool_t *pool, int size);
static void    initOS();

static void *reallocMem(pool_t *pool, int size);

/*本文件的内部接口定义*/
static void initOS()
{
    pagesize    = getpagesize();
    maxalloc    = MAX_ALLOC * pagesize;
    defaultsize = DEFAULT * pagesize;
    maxfailed   = MAX_FAILED;
}

static void resetPool(pool_t *pool, int size)
{
    pool->data.begin  = (char *)pool + sizeof(pool_t);
    pool->data.end    = (char *)pool->data.begin + size;
    pool->data.next   = NULL;
    pool->data.failed = 0;

    pool->current = pool;
    pool->clean   = NULL;
    pool->max = size > maxalloc ? maxalloc : size;
}

static pool_t *addNode(pool_t *pool, int size)
{
    pool_t *p_ptr   = NULL;
    pool_t *newnode = NULL;
    
    for (p_ptr = pool; !p_ptr && 
         p_ptr->data.next; p_ptr = p_ptr->data.next);
    
    if (!p_ptr)
    {
        printf("[addNode]:pool 为空！\n");

        return ;
    }

    if (size % pagesize)
    {
        size  = (size / pagesize + 1) * pagesize;
    }
    
    size = size > defaultsize ? size : defaultsize;

    newnode = malloc(size);
    if (!newnode)
    {
        printf("[addNode]:<malloc>:内存申请失败！\n");

        return ;
    }

    resetNode(newnode, size);

    p_ptr->data.next = newnode; 

    return newnode;
}

static void resetNode(pool_t *newnode, int size)
{
    newnode->data.begin  = (char *)newnode + sizeof(data_t);
    newnode->data.end    = (char *)newnode + size;

    newnode->data.failed = 0;
    newnode->data.next = NULL;
}

static void *reallocMem(pool_t *pool, int size)
{
    pool_t *ptr = NULL;
    void   *mem = NULL;
    
    ptr = addNode(pool, size);
    
    
    if (!ptr)
    {
        printf("[reallocMem]:内存池指针为空！\n");

        return NULL;
    }

    mem = ptr->data.begin;
    ptr->data.begin += size;

    return mem;
}

/*外部接口定义*/

pool_t *createPool(int size)
{
    pool_t *pool = NULL;

    initOS();

    size += sizeof(pool_t);

    if (size % pagesize)
    {
        size = (size / pagesize + 1) * pagesize;
    }

    pool = malloc(size);
    if (!pool)
    {
        printf("[createPool]<malloc>:malloc失败!\n");
        return pool;
    }

    size = size - sizeof(pool_t);
    resetPool(pool, size);

    return pool;
}

void destroyPool(pool_t *pool)
{
    pool_t *p_ptr = NULL;
    pool_t *q_ptr = NULL;

    if (!pool)
    {
        printf("[destroyPool]:传入内存池为空!\n");

        return ;
    }

    if (pool->clean)
    {
        pool->clean->handle(pool->clean->args);
        free(pool->clean);
        
        pool->clean = NULL;
    }

    for (p_ptr = pool; p_ptr; )
    {
        q_ptr = p_ptr->data.next;
        
        free(p_ptr);

        p_ptr = q_ptr;
    }
}

void *getMem(pool_t *pool, int size)
{
    void *ptr = NULL;
    void *mem = NULL;

    pool_t *p_ptr = NULL;

    int flag = 0;

    if (size > maxalloc)
    {
        printf("申请内存过大!\n");

        return mem;
    }

    for (p_ptr = pool->current; 
         p_ptr; p_ptr = p_ptr->data.next)
    {
        if (p_ptr->data.end - p_ptr->data.begin >= size)
        {  
            ptr = p_ptr->data.begin;
            p_ptr->data.begin += size;
            
            flag = 1;

            break;
        }
        /*处理当前只有一个节点的情况*/
        if (!p_ptr->data.next)
        {
            break;
        }
    }
    
    if (!flag)
    {
        size += sizeof(pool_t);
        if (size % pagesize)
        {
            size = (size / pagesize + 1) * pagesize;
        }
        ptr = reallocMem(p_ptr, size);
        
        pool->current->data.failed++;
    }

    
    if (pool->current->data.failed > maxfailed)
    {
        pool->current = pool->current->data.next;    
    }
    
    return ptr;
}

void handle_clean(pool_t *pool, handle_t func, void *args)
{
    clean_t *temp = NULL;

    temp = malloc(sizeof(clean_t));
    if (!temp)
    {
        printf("[handle_clean]:<malloc>：内存申请失败！\n");

        return ;
    }

    temp->handle = func;
    temp->args   = args;

    pool->clean = temp;
}
