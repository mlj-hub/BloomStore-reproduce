#ifndef UTIL_H
#define UTIL_H_H


// number of bits
#define M_BITS 1000
// number of hash functions
#define K_FUNC 10

// below are in bytes
#define FLASH_PAGE_SIZE 4096
#define KV_PAIR_SIZE 64
#define MAX_BF_CHAIN_LEN 96
#define KEY_SIZE 5*4
#define VALUE_SIZE 11*4
#define KV_PAIR_NUM_PER_PAGE (FLASH_PAGE_SIZE/KV_PAIR_SIZE)
#define DATA_PAGES_PER_INSTANCE KV_PAIR_NUM_PER_PAGE*MAX_BF_CHAIN_LEN


#endif