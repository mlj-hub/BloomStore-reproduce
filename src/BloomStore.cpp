#include "../inc/BloomStore.h"

Bloom_store_t::Bloom_store_t(int32_t n, int32_t k, int32_t fd){
    BF_buf.filter.set_para(n,k);
    this->fd = fd;
    cur_bf_chain_len = 0;
}

void Bloom_store_t::flush(){
    // flush KV pair write buffer
    // no need to seek, since we view flash as an append-only log
    int num = write(fd,(void *)KV_pair_buf,FLASH_PAGE_SIZE);
    if(num != FLASH_PAGE_SIZE){
        printf("kv pair flush not complete, written num: %d\n",num);
        exit(-1);
    }

    // flush BF buffer
    // get the current offset, for later appending
    uint32_t cur_ofs = lseek(fd,0,SEEK_CUR);

    if(lseek(fd,bf_chain_begin_addr,SEEK_SET) != bf_chain_begin_addr){
        printf("seek error, cannot read bf chain\n");
        exit(-1);
    }
    // number of pages that contain the remainder bf chain
    uint32_t used_pages = ((cur_bf_chain_len-1)*BF_SIZE/FLASH_PAGE_SIZE)+1;
    uint32_t alloced_size;


    // if the free space is enough for one more bf
    if(used_pages*FLASH_PAGE_SIZE - cur_bf_chain_len*BF_SIZE >=0)
        alloced_size = FLASH_PAGE_SIZE*used_pages;
    else
        alloced_size = FLASH_PAGE_SIZE*(used_pages+1);

    void * remainder_chain_page = aligned_alloc(FLASH_PAGE_SIZE,alloced_size);

    int read_num = read(fd,remainder_chain_page,used_pages*FLASH_PAGE_SIZE);
    if( read_num != used_pages*FLASH_PAGE_SIZE){
        printf("read bf_chain not complete, read num:%d\n",read_num);
        exit(-1);
    }
    
    ((BF_t*) remainder_chain_page)[cur_bf_chain_len-1] = BF_buf; 

    if(lseek(fd,cur_ofs,SEEK_SET) != cur_ofs){
        printf("seek error, cannot write bf chain\n");
        exit(-1);
    }

    int write_num = write(fd,remainder_chain_page,alloced_size);
    if(write_num != alloced_size){
        printf("bf chain flush not complete, written num: %d\n",write_num);
        exit(-1);
    }

    free(remainder_chain_page);
    
}