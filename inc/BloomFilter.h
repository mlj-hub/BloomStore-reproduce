#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <bitset>
#include <ctime>

// number of bits
#define M_BITS 1000

class Bloom_filter_t{
public:
    // number of elements in the set S
    int32_t n;
    // number of hash functions
    int32_t k;
    // number of bits
    int32_t m = M_BITS;
    // vector
    std::bitset<M_BITS> v;
    uint32_t * seeds;

    Bloom_filter_t(int32_t n,int32_t k);
    ~Bloom_filter_t();

    //hash function, reference from https://en.wikipedia.org/wiki/MurmurHash
    uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);
    static inline uint32_t murmur_32_scramble(uint32_t k) {
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        return k;
    }

    void insert(uint32_t * key);
    bool find(uint32_t * key);
};

#endif