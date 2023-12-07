#include "../inc/BloomStore.h"

Bloom_store_t::Bloom_store_t(int32_t fd,uint32_t * seeds):fd(fd), seeds(seeds){
    cur_bf_chain_len = 0;
}

/**
 * @brief look up key in the bloom filter chain parallelly 
 * 
 * @param key key to look up for
 * @param remainder_bf_chain buffer  which stores the bf chain
 * @param res lookup result array
 * @return true find in the chain
 * @return false cannot find in the chain
 */
bool Bloom_store_t::bf_chain_parallel_lookup(uint32_t * key,void * remainder_bf_chain ,bool * res){
    return true;
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

    if(lseek(fd,ofs,SEEK_SET) != ofs){
        printf("cannot seek to a page\n");
        exit(-1);
    }

    int read_num = read(fd,buf,FLASH_PAGE_SIZE*num);

    if(read_num != FLASH_PAGE_SIZE*num){
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
 * @return true write success
 * @return false write fail
 */
bool Bloom_store_t::write_pages(void * buf,uint32_t num){
    int write_num = write(fd,buf,FLASH_PAGE_SIZE*num);

    if(write_num != FLASH_PAGE_SIZE*num){
        printf("cannot read a page, read num:%d\n",write_num);
        exit(-1);
    }

    return true;
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
    uint32_t data_page_ofs = lseek(fd,0,SEEK_CUR);
    BF_buf.pointer = data_page_ofs;

    write_pages((void *)KV_pair_buf,1);

    // flush BF buffer

    uint32_t used_pages = ((cur_bf_chain_len-1)*BF_SIZE/FLASH_PAGE_SIZE)+1;

    uint32_t alloced_size = FLASH_PAGE_SIZE*used_pages;
    alloced_size += (used_pages*FLASH_PAGE_SIZE - cur_bf_chain_len*BF_SIZE)>=0 ? 0:FLASH_PAGE_SIZE;

    void * remainder_chain_page = aligned_alloc(FLASH_PAGE_SIZE,alloced_size);

    uint32_t cur_ofs = read_pages(bf_chain_begin_addr,remainder_chain_page,used_pages);

    ((BF_t*) remainder_chain_page)[cur_bf_chain_len-1] = BF_buf; 

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
        for(int i=0;i<write_buf_ofs;i++){
            // if find in the KV pair write buffer
            if(memcmp(KV_pair_buf[i].key,key,KEY_SIZE)){
                memcpy(value,KV_pair_buf[i].value,VALUE_SIZE);
                return true;
            }
        }
    }

    // number of pages that contain the remainder bf chain
    uint32_t used_pages = ((cur_bf_chain_len-1)*BF_SIZE/FLASH_PAGE_SIZE)+1;
    void * remainder_bf_chain = aligned_alloc(FLASH_PAGE_SIZE,used_pages*FLASH_PAGE_SIZE);

    uint32_t cur_ofs = read_pages(bf_chain_begin_addr,remainder_bf_chain,used_pages);

    bool * res = new bool[cur_bf_chain_len-1];
    
    // if cannot find in the remainder_bf_chain
    if(!bf_chain_parallel_lookup(key,remainder_bf_chain,res)){
        return false;
    }

    // found in the remainder_bf_chain, search from the largest index bf
    void * temp_data_buf = aligned_alloc(FLASH_PAGE_SIZE,FLASH_PAGE_SIZE);
    for(int i=cur_bf_chain_len-2;i>=0;i--){
        if(res[i]){
            read_pages(((BF_t *)remainder_bf_chain)[i].pointer, temp_data_buf, 1);
            for(int j=KV_PAIR_NUM_PER_PAGE-1;j>=0;j--){
                // if find in the KV pair write buffer
                if(memcmp(KV_pair_buf[i].key,key,KEY_SIZE)){
                    memcpy(value,KV_pair_buf[i].value,VALUE_SIZE);
                    return true;
                }
            }
        }
    }

    free(temp_data_buf);

    // cannot find in all the data pages
    return false;

}

bool Bloom_store_t::KV_insertion(uint32_t * key, uint32_t * value){
    
}