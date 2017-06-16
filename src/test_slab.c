/*************************************************************************
	> File Name:    test_slab.c
	> Author:       vivid
	> Mail:         353470469@qq.com
	> Created Time: 2017年05月10日 星期三 23时03分39秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#include <sys/time.h>

#include <slab.h>
extern int pagesize;

int main(int argc, char *argv[])
{
    slab_t *slab  = NULL;
    void   *addr  = NULL;
    int     loop  = 0;
    int     count = 0;
    int     size  = 0;


    struct timeval start;
    struct timeval end;

    if (argc != 3)
    {
        printf("Usage: cmd size cnt.\n");
        
        return -1;
    }

    count = atoi(argv[1]);
    size  = atoi(argv[2]);

    slab = createSlab(1024);
   /*
    * 内存池性能测试
    * */    
    
    memset(&start, 0x0, sizeof(struct timeval));
    memset(&end, 0x0, sizeof(struct timeval));

    gettimeofday(&start, NULL);
    for (loop = 0; loop < count; ++loop)
    {
        addr = allocBySlab(slab, size);
        freeBySlab(slab, addr);
    }

    gettimeofday(&end, NULL);

    printf("[mempool]:size=%d,count=%d\n", size, count);
    printf("\t%d.%d\n", end.tv_sec-start.tv_sec,
          end.tv_usec-start.tv_usec);

    destorySlab(slab);

    /*
    * malloc 测试
    * */

    memset(&start, 0x0, sizeof(struct timeval));
    memset(&end, 0x0, sizeof(struct timeval));

    gettimeofday(&start, NULL);
    for (loop = 0; loop < count; ++loop)
    {
        addr = malloc(size);
        free(addr);
    }
    gettimeofday(&end, NULL);
    
    printf("[malloc]:size=%d,count=%d\n", size, count);
    printf("\t%d.%d\n", end.tv_sec-start.tv_sec,
          end.tv_usec-start.tv_usec);



    return 0;
}
