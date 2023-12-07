#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <bitset>
#include <ctime>

#include "../inc/Util.h"


class Bloom_filter_t{
public:
    // vector
    std::bitset<M_BITS> v;

    Bloom_filter_t();

    //hash function, reference from https://en.wikipedia.org/wiki/MurmurHash
    uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);
    static inline uint32_t murmur_32_scramble(uint32_t k) {
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        return k;
    }

    void insert(uint32_t * key,uint32_t * seeds);
    bool find(uint32_t * key,uint32_t * seeds);

    Bloom_filter_t & operator=(Bloom_filter_t & rhs);
};

#endif