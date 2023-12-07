#include "../inc/BloomFilter.h"

Bloom_filter_t::Bloom_filter_t(){
    v.reset();
}

Bloom_filter_t::Bloom_filter_t(int32_t n, int32_t k): n(n),k(k){
    v.reset();
    seeds = new uint32_t[k];
    srand(time(NULL));
    // generate seeds
    for (int i=0;i<k;i++)
        seeds[i] = rand();
}

Bloom_filter_t::~Bloom_filter_t(){
    delete seeds;
}

void Bloom_filter_t::set_para(int32_t n,int32_t k){
    this->n = n;
    this->k = k;
    seeds = new uint32_t[k];
    srand(time(NULL));
    // generate seeds
    for (int i=0;i<k;i++)
        seeds[i] = rand();
}

uint32_t Bloom_filter_t::murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;
    uint32_t k;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) {
        // Here is a source of differing results across endiannesses.
        // A swap here has no effects on hash properties though.
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }
    // A swap is *not* necessary here because the preceding loop already
    // places the low bytes in the low places according to whatever endianness
    // we use. Swaps only apply when the memory is copied in a chunk.
    h ^= murmur_32_scramble(k);
    /* Finalize. */
    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

void Bloom_filter_t::insert(uint32_t * key){
    // key is a array containing 5 elements
    for(int i=0;i<k;i++){
        uint32_t hash_value = murmur3_32((const uint8_t *)key,5*4,seeds[i]);
        v.set(hash_value % m,1);
    }
}

bool Bloom_filter_t::find(uint32_t * key){
    // key is a array containing 5 elements
    bool exist = 1;
    for(int i=0;i<k;i++){
        uint32_t hash_value = murmur3_32((const uint8_t *)key,5*4,seeds[i]);
        exist &= v.test(hash_value % m);
    }
    return exist;
}

Bloom_filter_t & Bloom_filter_t::operator=(Bloom_filter_t & rhs){
    this->v = rhs.v;
    return *this;
}