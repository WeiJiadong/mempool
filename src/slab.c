/*************************************************************************
	> File Name:    slab.c
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年04月30日 星期日 18时20分55秒
 ************************************************************************/

#include <stdio.h>

#include <slab.h>

#define IS_FIND     (1)

/*内部接口声明*/
static void  initPages(slab_t *slab);
static void  initSlots(slab_t *slab);
static void *getSlabMem(slab_t *slab, int size);
static void *reallocSlab(slab_t *slab, int slot, int page);

static int getPage(page_t *pages, int page);
static int getSlot(int size);

/*内部接口定义*/

static int getPage(page_t *pages, int page)
{
    int index = 0;
    int max   = 0;

    for (index = page; index < max;++page)
    {
        if (pages[index].type & IS_FREE)
        {
            return index;
        }
    }
}

static void *reallocSlab(slab_t *slab, int slot, int page)
{
    void   *ptr   = NULL;
    int     index = 0;
    page_t *prev  = NULL;
    page_t *next  = NULL;
    page_t *pages = NULL;
    slot_t *slots = NULL;
    /*
    * 1、获取一个新的页面。
    * 2、将该页面连接到对应的slot上。
    * 3、对页面1进行初始化。
    * 4、完成分配。
    * */

    index = getPage(slab->pages, page);
    if (-1 == index)
    {
        printf("[reallocSlab]:<getPage>:内存不足，申请内存失败！\n");
        
        return ptr;
    }

    slots = slab->slots;
    pages = slab->pages;
    
    prev = &(slots[slot].page);
    next = &(pages[index]);

    next->next = prev->next;
    prev->next = next;
    next->prev = prev;
    next->type = IS_BUSY;

    next->bitmap  = BITE_FREE;
    next->bitmap |= 1;

    ptr = slab->begin + page * pagesize;

    return ptr;
}

static void initSlots(slab_t *slab)
{
    int shift = 0;
    int page  = 0;
    int index = 0;
    int loop  = 0;

    for (page = slab->max_alloc; page; page >>= 1, ++shift);

    slab->slots = getMem(slab->pool, (shift - MIN_SHIFT) * sizeof(slot_t));

    for (index = 0, loop = MIN_SHIFT; loop < shift; ++loop)
    {
        slab->slots[index].page.prev = NULL;
        slab->slots[index].page.next = NULL;
        slab->slots[index].shift = loop;
        
        ++index;
    }
}

static void initPages(slab_t *slab)
{
    int page  = 0;
    int index = 0;

    page = (slab->end - slab->begin) / pagesize;
    slab->pages = getMem(slab->pool, page);

    for (index = 0; index < page;++index)
    {
        slab->pages[index].type   |= IS_FREE;
        slab->pages[index].bitmap  = BITE_FREE;
        slab->pages[index].prev    = NULL;
        slab->pages[index].next    = NULL;
    }
}

static int getSlot(int size)
{
    int slot    = 0;
    int ret_val = 0;

    for (slot = 0; size; size >>= 1, slot++);

    slot -= MIN_SHIFT;

    return slot;
}

int checkFull(char bitmap, int count)
{
    int ret_val = 0;
    int loop    = 0;

    for (loop = 0; loop < count; ++loop)
    {
        if (!(count & 1))
        {
            ret_val = 1;
            break;
        }
    }

    if (ret_val)
    {
        ret_val = 0;
    }

    return ret_val;
}

static void *getSlabMem(slab_t *slab, int size)
{
    void   *ret_val   = NULL;
    page_t *ptr       = NULL;
    int     page      = 0;
    int     slot      = 0;
    int     find_flag = 0;
    int     find_bit  = 0;
    int     map_flag  = 0;
    int     offset    = 0;
    int     count     = 0;
    int     chunk     = 0;
    int     full      = 0;
    
    /*
     * 1、获取slot,检测对应的slots[slot]；
     * 1-1、若对应slot有空闲内存页，则进行分配，若已满，则将其暂离。
     * 1-2、若对应slot指向为空，或无空闲内存页，则进行内存页申请。
     * 2、做一些扫尾工作。
     * */
    if (size >= slab->min_alloc)
    {
        slot = getSlot(size);
    }
    else
    {
        slot = 0;
    }
    
    find_flag = 0;
    for (ptr = slab->slots[slot].page.next; ptr; ptr = ptr->next)
    {
        if (ptr->type & IS_FULL)
        {
            continue;
        }
        else
        {
            find_bit = ptr->bitmap;
            map_flag = 1;
            if (find_bit == BITE_FREE)
            {
                find_flag = 1;

                break;
            }
            for(count = 0; find_bit; ++count)
            {
                if (!(find_bit & IS_FIND))
                {
                    find_flag = 1;
                    
                    break;
                }
                map_flag <<= 1;
                find_bit >>= 1;
            }
        }
    }

    if (find_flag)
    {
        chunk   = 1 << (slab->slots[slot].shift);
        offset  = count * chunk;
        ret_val = (char *)ptr + offset;
        ptr->bitmap |= map_flag;

        full = checkFull(ptr->bitmap, count);
        if (full)
        {
            ptr->type &= ~IS_BUSY;
            ptr->type &= ~IS_FREE;
            ptr->type &= IS_BUSY;
        }
    }
    else
    {
        if (ptr)
        {
            page = (ptr - slab->pages) / sizeof(page_t);
        }
        else
        {
            page = 0;
        }
        ret_val = reallocSlab(slab, slot, page);
    }

    return ret_val;
}
slab_t *createSlab(int alloc_size)
{
    slab_t *slab      = NULL;
    pool_t *pool      = NULL;
    pool_t *slab_pool = NULL;

    int size  = 0;
    int loop  = 0;
    int count = 0;
    
    pool = createPool(SLAB_TEMP_POOL_SIZE);
    if (!pool)
    {
        printf("[createSlab]:<createPool>:失败！\n");

        return slab;
    }

    size = SLAB_POOL_SIZE;
    size = size > alloc_size ? size : alloc_size;

    slab_pool = createPool(size);

    if (size % pagesize)
    {
        size = (size / pagesize + 1) * pagesize;
    }

    slab = getMem(pool, sizeof(slab_t));
    if (!slab)
    {
        printf("[createSlab]:<getMem>:失败！\n");

        return slab;
    }

    slab->begin = slab_pool->data.begin;
    slab->end   = slab_pool->data.end;

    slab->min_alloc = MIN_ALLOC;
    slab->max_alloc = SLAB_MAX_ALLOC;
    
    slab->pool = pool;
    slab->slab_pool = slab_pool;
    
    initSlots(slab);    
    initPages(slab);

    return slab;
}

void destorySlab(slab_t *slab)
{
    destroyPool(slab->pool);
    destroyPool(slab->slab_pool);
}

void *allocBySlab(slab_t *slab, int size)
{
    void *ret_val = NULL;

    /*
     * 判断size范围
     * 1. 若size > max_alloc，则返回getMem()的内存
     * 2. 若min_alloc <= size <= max_alloc返回对应chunk的内存
     * 3. 若小于min_alloc，则返回NULL;
     * */

    if (size > slab->max_alloc)
    {
        ret_val = getMem(slab->pool, size);
    }
    else
    {
        if (size > 0)
        {
            ret_val = getSlabMem(slab, size);
        }
    }
    
    return ret_val;
}

void freeBySlab(slab_t *slab, void *ptr)
{
    int     page  = 0;
    int     shift = 0;
    int     chunk = 0;
    page_t *pages = NULL;
    page_t *p     = NULL;
    slot_t *slot  = NULL;
    page_t *next  = NULL;
    page_t *prev  = NULL;
    page_t *node  = NULL;

    int  offset = 0;
    int  count  = 0;
    char temp   = 0;
    int  loop   = 0;
    /*   设计思路
    * 1、定位pages数组的元素位置 
    * 2、通过pages元素的prev,定位slots数组的元素位置
    * 3、计算chunk大小
    * 4、通过chunk大小获取页内chunk编号
    * 5、通过chunk编号，释放对应的内存（更改bitmap对应的位）
    * 6、做判断扫尾工作。
    * */

    pages = slab->pages;
    page  = (ptr - slab->begin) / pagesize;

    for (p = pages[page].prev; p; p = p->prev);

    offset = (char *)&slot->page - (char *)slot;

    slot = (slot_t *)((char *)ptr - offset);

    shift = slot->shift;
    chunk = 1 << (shift + MIN_SHIFT);
    count = (ptr - slab->begin) % pagesize / chunk;
    for (loop = 0, temp = 1; loop < count; ++loop)
    {
        temp << 1;
    }

    temp = ~temp;
    pages[page].bitmap &= temp;

    if (pages[page].type & IS_FULL)
    {
        pages[page].type = IS_BUSY;

        if (pages[page].next)
        {
            pages[page].next->prev = &(slot->page);
        }
        pages[page].next = slot->page.next;
        slot->page.next  = &pages[page];
        pages[page].prev = &(slot->page);
    }
    if (BITE_FREE == pages[page].bitmap)
    {
        node = &pages[page];
        prev = node->prev;
        next = node->next;

        if (next)
        {
            next->prev = node->prev;
        }
        prev->next = next;

        node->prev = NULL;
        node->next = NULL;
        pages[page].type = IS_FREE;
    }
}
