#ifndef BLOOM_STORE_H
#define BLOOM_STORE_H

#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../inc/BloomFilter.h"

// below are in bytes
#define FLASH_PAGE_SIZE 4096
#define FLASH_BLOCK_SIZE 32*FLASH_PAGE_SIZE
#define KV_PAIR_SIZE 64
#define MAX_BF_CHAIN_LEN 96
#define KV_PAIR_NUM_PER_PAGE (FLASH_PAGE_SIZE/KV_PAIR_SIZE)
#define BF_SIZE (sizeof(Bloom_filter_t) + sizeof(uint32_t))
#define DATA_PAGES_PER_INSTANCE KV_PAIR_NUM_PER_PAGE*MAX_BF_CHAIN_LEN
#define BF_BLOCKS_PER_INSTANCE  16

class Bloom_store_t{
    struct KV_pair_t{
        uint32_t key[5];
        uint32_t value[11];
    }KV_pair_buf[KV_PAIR_NUM_PER_PAGE] __attribute__((aligned(FLASH_PAGE_SIZE)));

    struct BF_t{
        Bloom_filter_t filter;
        uint32_t pointer;
        BF_f & operator=(BF_t & rhs){
            this->filter = rhs.filter;
            this->pointer = rhs.pointer;
            return *this;
        }
    }BF_buf;

    // file descriptor for block bevice
    int32_t fd;
    // i-th bloomstore instance, used for part block device
    int32_t id;
    // current bf_chain_len
    uint32_t cur_bf_chain_len;
    // begining address of the flash pages storing the remainder of the BFchain
    uint32_t bf_chain_begin_addr;

    Bloom_store_t(int32_t n,int32_t k,int32_t fd);

    void flush();
};

#endif