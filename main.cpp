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

#define PAIR_NUM 2454205
#define BYTE_PER_KV 1.0f
#define INSTANCE_NUM ((int)(PAIR_NUM*BYTE_PER_KV)/FLASH_PAGE_SIZE + 1)
#define KV_PAIR_FILE "test_case/key_value_10M.txt"
#define INSERT_FILE "test_case/key_value_10M_key.txt"
#define RES_FILE "bloom_store_res.txt"

#define CHOICE_SEEDS 154169715
#define LOOKUP_AFTER_INSERT 0

Bloom_store_t store_instance[INSTANCE_NUM];

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

    std::ifstream lookup_f(KV_PAIR_FILE,std::ios::in);
    if(!lookup_f){
        printf("cannot open kv pair file\n");
    }

    std::ifstream insert_f(INSERT_FILE,std::ios::in);
    if(!insert_f){
        printf("cannot open insert file\n");
    }

    std::ofstream ofs(RES_FILE,std::ios::out | std::ios::trunc);
    if(!ofs){
        printf("cannot open res file\n");
    }

    for(int i=0;i<INSTANCE_NUM;i++)
        store_instance[i].set_para(fd,seeds);

    std::string line;
    uint32_t key[5];
    uint32_t value[11];

    struct timeval start_time;
    struct timeval end_time;
    double time_duration = 0.0;
    uint64_t ops = 0;


#if LOOKUP_AFTER_INSERT
    uint64_t cnt = 0;
    while(std::getline(insert_f,line)){
        uint32_t key_num = std::stoi(line);
        key[0] = key[1] = key[2] = key[3] = key[4] = key_num;
        uint32_t id = get_store_instance_id(key);

        value[0] = key_num;
        store_instance[id].KV_insertion(key,value);

        cnt++;
        printf("ins %lu\n",cnt);
    }

    cnt = 0;
    while(std::getline(lookup_f,line)){
        uint32_t key_num = std::stoi(line);
        key[0] = key[1] = key[2] = key[3] = key[4] = key_num;
        uint32_t id = get_store_instance_id(key);
        // if find in the instance, print the value
        cnt++;
        printf("look %lu\n",cnt);
        gettimeofday(&start_time, NULL);
        if(store_instance[id].KV_lookup(key,value)){
            gettimeofday(&end_time,NULL);
            time_duration+=(double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_usec - start_time.tv_usec)/1000.0/1000.0;

            ops++;
        }
        else{
            printf("cannot find\n");
            exit(-1);
        }
    }

#else

    while(std::getline(lookup_f,line)){
        uint32_t key_num = std::stoi(line);
        key[0] = key[1] = key[2] = key[3] = key[4] = key_num;
        uint32_t id = get_store_instance_id(key);

        // find the key
        gettimeofday(&start_time, NULL);
        bool res = store_instance[id].KV_lookup(key,value);
        gettimeofday(&end_time,NULL);
        
        ops++;
        time_duration+=(double)(end_time.tv_sec - start_time.tv_sec) + (double)(end_time.tv_usec - start_time.tv_usec)/1000.0/1000.0;


        if(res){
            ofs<<value[0]<<"\n";
        }
        // if not find, insert it, print not find
        else{
            value[0] = key_num;
            store_instance[id].KV_insertion(key,value);
            ofs<<"not find\n";
        }
    }
    
#endif

    printf("flash page size: %d\n",FLASH_PAGE_SIZE);
    printf("instance num: %d, byte per kv pair: %d\n",INSTANCE_NUM,BYTE_PER_KV);
    printf("hash function num: %d, bits in filter: %d\n",K_FUNC,M_BITS);
    printf("ops num: %lu\n",ops);
    printf("caused time: %lfs\n", time_duration);
    printf("ops/sec: %lf\n",(double)ops/time_duration);
    printf("used space: %lfM\n",(double)lseek(fd,0,SEEK_CUR)/1024.0/1024.0);


    uint32_t total_read=0,total_write=0;
    for(int i=0;i<INSTANCE_NUM;i++){
        total_read+=store_instance[i].read_page_num;
        total_write+=store_instance[i].write_page_num;
    }

    printf("total_read: %u, total_write: %u\n",total_read,total_write);

    close(fd);
    return 0;
}