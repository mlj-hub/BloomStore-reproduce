#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>

#include "inc/BloomFilter.h"
#include "inc/BloomStore.h"

#define PAIR_NUM 50

int main(int argc,char ** argv){
    int fd;
    fd = open(argv[1],O_RDWR|O_DIRECT|O_SYNC);
    if(fd == -1){
        printf("unable to open device\n");
        exit(-1);
    }
    printf("fd: %d\n",fd);

    uint32_t seeds[K_FUNC];
    std::default_random_engine e{std::random_device{}()};
    for(int i=0;i<K_FUNC;i++){
        seeds[i] = e();
    }

    Bloom_store_t bloom_store(fd,seeds);

    uint32_t key[PAIR_NUM][5];
    uint32_t value[PAIR_NUM][11];

    for(int i=0;i<PAIR_NUM;i++){
        for(int j=0;j<5;j++)
            key[i][j] = e();
        for(int j=0;j<11;j++)
            value[i][j] = e();
    }

    for(int i=0;i<PAIR_NUM;i++)
        bloom_store.KV_insertion(key[i],value[i]);

    uint32_t test_value[PAIR_NUM][11];
    for(int i=0;i<PAIR_NUM;i++){
        bool res = bloom_store.KV_lookup(key[i],test_value[i]);
        if(!res){
            printf("pair %d cannot find\n",i);
            continue;
        }
        if(memcmp(test_value[i],value[i],VALUE_SIZE))
            printf("pair %d value error\n",i);
        else
            printf("pair %d value correct\n",i);
    }

    close(fd);
    return 0;
}