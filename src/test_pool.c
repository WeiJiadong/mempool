/*************************************************************************
	> File Name:    test.c
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年04月26日 星期三 23时13分08秒
 ************************************************************************/

#include <stdio.h>
#include <sys/time.h>  
#include <pool.h>
#include <string.h>

extern int pagesize;

int main(int argc, char *argv[])
{
    pool_t *pool  = NULL;
    void   *res   = NULL;

    int count = 0;
    int loop  = 0;
    int size  = 0;
    

    count =  1;
    size  =  3000;

    pool = createPool(1024);
    res = getMem(pool, 30000);
    if (pool->current == pool){
        printf("current 指向pool.\n");
    }


    for (loop = 0; loop < count; ++loop)
    {
        res = getMem(pool, size);
    }

    destroyPool(pool);
    
    return 0;
}
