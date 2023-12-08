#include "../inc/BloomStore.h"

Bloom_store_t::Bloom_store_t(int32_t fd,uint32_t * seeds):fd(fd), seeds(seeds){
    in_flash_bf_chain_len = 0;
    write_buf_ofs=0;
    bf_chain_begin_addr = 0;
}

void Bloom_store_t::set_para(int32_t fd, uint32_t * seeds){
    this->fd = fd;
    this->seeds = seeds;
    in_flash_bf_chain_len = 0;
    write_buf_ofs=0;
    bf_chain_begin_addr = 0;
}

/**
 * @brief look up key in the bloom filter chain parallelly 
 * 
 * @param key key to look up for
 * @param remainder_bf_chain buffer  which stores the bf chain
 * @param res lookup result array
 */
void Bloom_store_t::bf_chain_parallel_lookup(uint32_t * key,BF_t * remainder_bf_chain ,bool * res){
    uint32_t ofs[K_FUNC];
    // get offset according to k hash functions
    for(int i=0;i<K_FUNC;i++)
        ofs[i] = murmur3_32((uint8_t *)key,KEY_SIZE,seeds[i]) % M_BITS;

    for(int i=0;i<K_FUNC;i++){
        for(uint32_t j=0;j<in_flash_bf_chain_len;j++)
            res[j] &= remainder_bf_chain[j].filter[ofs[i]];
    }
}

/**
 * @brief read several pages from flash at ofs
 * 
 * @param ofs offset to read from
 * @param buf buffer for storation
 * @param num number of pages to read
 * @return uint32_t offest before reading
 * @note  This function will not change the file offset
 */
uint32_t Bloom_store_t::read_pages(uint32_t ofs, void * buf, uint32_t num){
    uint32_t cur_ofs = lseek(fd,0,SEEK_CUR);

    int read_num = pread(fd,buf,FLASH_PAGE_SIZE*num,ofs);

    if(read_num != FLASH_PAGE_SIZE*(int)num){
        printf("cannot read a page, read num:%d\n",read_num);
        exit(-1);
    }

    return cur_ofs;
}

/**
 * @brief write(append) several pages into flash
 * 
 * @param buf data to write
 * @param num number of pages to write
 * @return uint32_t offset before writing
 * @note this function will change file offset
 */
uint32_t Bloom_store_t::write_pages(void * buf,uint32_t num){
    uint32_t cur_ofs = lseek(fd,0,SEEK_CUR);
    int write_num = write(fd,buf,FLASH_PAGE_SIZE*num);

    if(write_num != FLASH_PAGE_SIZE*(int)num){
        printf("cannot write a page, write num:%d\n",write_num);
        exit(-1);
    }

    return cur_ofs;
}

/**
 * @brief  flush kv pair write buffer and bf_buffer into flash
 * @param  none
 * @return none
 * @note   will not update any meta data, e.g. cur_bf_chain_len,write_buf_ofs
 */
void Bloom_store_t::flush(){
    // flush KV pair write buffer
    // update the bf.pointer to the begin addr of the data page
    BF_buf.pointer = write_pages((void *)KV_pair_buf,1);;

    // flush BF buffer

    uint32_t used_pages = ((in_flash_bf_chain_len)*BF_SIZE/FLASH_PAGE_SIZE)+1;

    uint32_t alloced_size = FLASH_PAGE_SIZE*used_pages;
    alloced_size += (used_pages*FLASH_PAGE_SIZE > (in_flash_bf_chain_len+1)*BF_SIZE) ? 0:FLASH_PAGE_SIZE;

    void * remainder_chain_page = aligned_alloc(FLASH_PAGE_SIZE,alloced_size);

    if(!remainder_chain_page){
        printf("cannot alloc data\n");
        exit(-1);
    }

    uint32_t cur_ofs = read_pages(bf_chain_begin_addr,remainder_chain_page,used_pages);

    ((BF_t*) remainder_chain_page)[in_flash_bf_chain_len] = BF_buf; 

    write_pages(remainder_chain_page,alloced_size/FLASH_PAGE_SIZE);

    // update bf chain beginning address after flushing
    bf_chain_begin_addr = cur_ofs;

    free(remainder_chain_page);
}

/**
 * @brief look up for the given key, return results in the given value array
 * 
 * @param key a 5-elements array
 * @param value a 11-elements array
 * @return true find the key-value pair
 * @return false key does not exist
 */
bool Bloom_store_t::KV_lookup(uint32_t * key, uint32_t * value){
    // first search in the active bf
    bool in_active_bf = BF_buf.filter.find(key,seeds);

    if(in_active_bf){
        for(int i=(int)write_buf_ofs-1;i>=0;i--){
            // if find in the KV pair write buffer
            if(memcmp(KV_pair_buf[i].key,key,KEY_SIZE)==0){
                if(!is_value_valid(KV_pair_buf[i].value))
                    return false;
                memcpy(value,KV_pair_buf[i].value,VALUE_SIZE);
                return true;
            }
        }
    }

    // number of pages that contain the remainder bf chain
    uint32_t used_pages = ((in_flash_bf_chain_len)*BF_SIZE/FLASH_PAGE_SIZE)+1;
    void * remainder_bf_chain = aligned_alloc(FLASH_PAGE_SIZE,used_pages*FLASH_PAGE_SIZE);

    read_pages(bf_chain_begin_addr,remainder_bf_chain,used_pages);

    bool * res = (bool*) malloc(in_flash_bf_chain_len * sizeof(bool));
    memset(res,1,in_flash_bf_chain_len*sizeof(bool));
    
    // if cannot find in the remainder_bf_chain
    bf_chain_parallel_lookup(key,(BF_t*)remainder_bf_chain,res);

    // found in the remainder_bf_chain, search from the largest index bf
    KV_pair_t * temp_data_buf = (KV_pair_t *)aligned_alloc(FLASH_PAGE_SIZE,FLASH_PAGE_SIZE);
    bool found = false;
    for(int i=in_flash_bf_chain_len-1;i>=0;i--){
        if(res[i]){
            read_pages(((BF_t *)remainder_bf_chain)[i].pointer, (void *)temp_data_buf, 1);
            for(int j=KV_PAIR_NUM_PER_PAGE-1;j>=0;j--){
                // if find in the KV pair write buffer
                if(memcmp(temp_data_buf[j].key,key,KEY_SIZE)==0){
                    if(!is_value_valid(temp_data_buf[j].value))
                        goto done;
                    memcpy(value,temp_data_buf[j].value,VALUE_SIZE);
                    found = true;
                    goto done;
                }
            }
        }
    }

done:
    free(res);
    free(temp_data_buf);
    free(remainder_bf_chain);

    // cannot find in all the data pages
    return found;

}

/**
 * @brief insert a key-value pair
 * 
 * @param key 
 * @param value 
 */
void Bloom_store_t::KV_insertion(uint32_t * key, uint32_t * value){
    //insert the KV pair into the kv pair write buffer
    memcpy(KV_pair_buf[write_buf_ofs].key,key,KEY_SIZE);
    memcpy(KV_pair_buf[write_buf_ofs++].value,value,VALUE_SIZE);
    // insert the ke into filter
    BF_buf.filter.insert(key,seeds);
    // if the write buffer is full, flush both write buffer and the bf_buffer
    if(write_buf_ofs == KV_PAIR_NUM_PER_PAGE){
        flush();
        // reset write buffer
        write_buf_ofs = 0;
        in_flash_bf_chain_len++;
        //reset bf_buffer
        BF_buf.filter.reset();
    }
}

/**
 * @brief delete the key-pair according to the given key
 * 
 * @param key 
 * @note The deletion works as a null-value insertion for the key
 */
void Bloom_store_t::KV_deletion(uint32_t * key){
    uint8_t value[VALUE_SIZE];
    memset(value,0,VALUE_SIZE);

    KV_insertion(key,(uint32_t *)value);
}

/**
 * @brief check whether the given value is null
 * 
 * @param value value to be checked
 * @return true valid value
 * @return false null value
 */
bool Bloom_store_t::is_value_valid(uint32_t * value){
    for(int i=0;i<VALUE_SIZE/4;i++){
        if(value[i])
            return true;
    }
    return false;
}