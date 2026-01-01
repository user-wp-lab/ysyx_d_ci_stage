#include <stdio.h>

#define cache_way 8   //多少组
#define cache_set 4   //每组多少个
#define cache_line 8  //每块多大
#define cache_wszie cache_way*cache_set

#define cache_n (cache_way == 4) ? 2 : (cache_way == 8) ? 3 : (cache_way == 16) ? 4 : (cache_way == 32) ? 5 : (cache_way == 64) ? 6 : 0
#define cache_m (cache_line == 4) ? 2 : (cache_line == 8) ? 3 : (cache_line == 16) ? 4 : 0

int cachesim(unsigned int pc){
    typedef struct {
        unsigned int tag;
        int valid;
    } CacheBlock;
    typedef struct {
        CacheBlock blocks[cache_set];
    } CacheSet;
    int cache_size = cache_line/4;  //通过读取offset
    CacheSet cache[cache_way][cache_size];
    int temp   = 32 - cache_n - cache_m;
    int tag    = pc >> (cache_n + cache_m);
    int offset = (pc << (32 - cache_m)) >> (32 - cache_m);
    int index  = ((pc >> cache_m) << (temp + cache_m)) >> (temp + cache_m);

    unsigned int hit = 0;  //over wai
    unsigned int miss = 0;
    typedef struct {
        int ptr;
    } fifo_ptr;
    fifo_ptr fifo[cache_way];
    int cache_set_i = 0;              //over wai
    for (cache_set_i = 0 ; cache_set_i < cache_set ; cache_set_i++){
        if((cache[index][offset].blocks[cache_set_i].tag == tag) & (cache[index][offset].blocks[cache_set_i].valid == 1)){
            hit ++;
            cache_set_i = cache_set; 
        }
    }
    if(cache_set_i != cache_set){
        miss++;
        cache_set_i = 0;
        cache[index][offset].blocks[fifo[index].ptr].tag   = tag;
        cache[index][offset].blocks[fifo[index].ptr].valid = 1;
        fifo[index].ptr++;
        if(fifo[index].ptr >= cache_set){
            fifo[index].ptr = 0;
        }
    }
    return 0;
}