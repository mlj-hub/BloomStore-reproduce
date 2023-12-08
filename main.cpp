#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sys/time.h>

#include "inc/BloomFilter.h"
#include "inc/BloomStore.h"

#define INSTANCE_NUM 96
#define KV_PAIR_FILE "key_value_demo.txt"
#define RES_FILE "bloom_store_demo_res.txt"

#define CHOICE_SEEDS 154169715

uint32_t get_store_instance_id(uint32_t * key){
    return murmur3_32((uint8_t *)key,KEY_SIZE,CHOICE_SEEDS) % INSTANCE_NUM;
}

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

    std::ifstream ifs(KV_PAIR_FILE,std::ios::in);
    if(!ifs){
        printf("cannot open kv pair file\n");
    }

    std::ofstream ofs(RES_FILE,std::ios::out | std::ios::trunc);
    if(!ofs){
        printf("cannot open kv pair file\n");
    }

    Bloom_store_t store_instance[INSTANCE_NUM];
    for(int i=0;i<INSTANCE_NUM;i++)
        store_instance[i].set_para(fd,seeds);

    std::string line;
    uint32_t key[5];
    uint32_t value[11];

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    uint64_t ops = 0;
    while(std::getline(ifs,line)){
        uint32_t key_num = std::stoi(line);
        key[0] = key[1] = key[2] = key[3] = key[4] = key_num;
        uint32_t id = get_store_instance_id(key);
        // if find in the instance, print the value
        if(store_instance[id].KV_lookup(key,value)){
            ofs<<value[0]<<"\n";
            ops++;
        }
        // if not find, insert it, print not find
        else{
            value[0] = key_num;
            store_instance[id].KV_insertion(key,value);
            ofs<<"not find\n";
            ops+=2;
        }
    }
    
    struct timeval end_time;
    gettimeofday(&end_time,NULL);

    double time_duration = (double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_usec - start_time.tv_usec)/1000.0/1000.0;

    printf("flash page size: %d\n",FLASH_PAGE_SIZE);
    printf("ops num: %lu\n",ops);
    printf("caused time: %lfs\n", time_duration);
    printf("ops/sec: %lf\n",(double)ops/time_duration);
    printf("used space: %lfM\n",(double)lseek(fd,0,SEEK_CUR)/1024.0/1024.0);

    close(fd);
    return 0;
}