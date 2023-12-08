#ifndef BLOOM_STORE_H
#define BLOOM_STORE_H

#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../inc/BloomFilter.h"
#include "../inc/Util.h"


class Bloom_store_t{
public:
    struct KV_pair_t{
        uint32_t key[5];
        uint32_t value[11];
    }KV_pair_buf[KV_PAIR_NUM_PER_PAGE] __attribute__((aligned(FLASH_PAGE_SIZE)));

    // indicate the offset of next writable KV pair
    uint32_t write_buf_ofs;

    struct BF_t{
        Bloom_filter_t filter;
        uint32_t pointer;
        BF_t & operator=(BF_t & rhs){
            this->filter = rhs.filter;
            this->pointer = rhs.pointer;
            return *this;
        }
    }BF_buf;

    // file descriptor for block bevice
    int32_t fd;
    // used for bloom filter
    uint32_t * seeds;
    // i-th bloomstore instance, used for part block device
    int32_t id;
    // current bf_chain_len
    uint32_t in_flash_bf_chain_len;
    // begining address of the flash pages storing the remainder of the BFchain
    uint32_t bf_chain_begin_addr;

    // Bloom filter size, in unit BYTE
    size_t BF_SIZE = sizeof(BF_t);

    Bloom_store_t(){};
    Bloom_store_t(int32_t fd,uint32_t * seeds);
    void set_para(int32_t fd,uint32_t * seeds);

    void flush();
    bool KV_lookup(uint32_t * key, uint32_t * value);
    void KV_insertion(uint32_t * key, uint32_t * value);
    void KV_deletion(uint32_t * key);
    void bf_chain_parallel_lookup(uint32_t * key,BF_t * remainder_bf_chain ,bool * res);
    // read NUM pages from OFS into BUF, return current offset
    uint32_t read_pages(uint32_t ofs, void * buf, uint32_t num);
    // write(append) NUM pages, return offset before writing
    uint32_t write_pages(void * buf,uint32_t num);
    bool is_value_valid(uint32_t * value);
};

#endif