#include "buffer.h"


buffer_t * buf_pool = NULL;
buf_control_t * buf_control = NULL;

int buf_size_num = 0;
int left_buf = 0;

int buf_init(int num_buf){
    buf_pool = (buffer_t*)malloc(sizeof(buffer_t)*num_buf);
    
    buf_size_num = num_buf;
    left_buf = num_buf;

    buf_latch = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER; 

    for(int i=0; i<num_buf; i++){
        buf_pool[i].frame = (page_t*)malloc(sizeof(page_t));
        buf_pool[i].table_id = -1;
        buf_pool[i].buffer_index = i;
        buf_pool[i].page_num = -1;
        buf_pool[i].is_dirty = 0;
        buf_pool[i].is_pinned = 0;
        buf_pool[i].buf_pool_latch = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        buf_pool[i].prev = -1;
        buf_pool[i].next = -1;
    }

    buf_control = (buf_control_t*)malloc(sizeof(buf_control_t)*2);
    buf_control[start].buffer_index = -2;
    buf_control[start].next = -1;
    buf_control[start].prev = -1;

    buf_control[end].buffer_index = -3;
    buf_control[end].next = -1;
    buf_control[end].prev = -1;

    return 0;
}

int buf_open_database_file(const char* pathname){
    
    pthread_mutex_lock(&buf_latch);

    fd = file_open_database_file(pathname);

    pthread_mutex_unlock(&buf_latch);

    return fd;
}

pagenum_t buf_alloc_page(int fd){ 

    pthread_mutex_lock(&buf_latch);

    pagenum_t alloc_page_num;
    header_t tmp;
    int i = 0;

    alloc_page_num = file_alloc_page(fd);
    for(i=0; i< buf_size_num; i++){
        if(buf_pool[i].page_num == 0){
            file_read_page(fd, 0, buf_pool[i].frame);
            break;
        }
    }

    pthread_mutex_unlock(&buf_latch);

    return alloc_page_num;
}

void buf_free_page(int fd,pagenum_t pagenum){

    pthread_mutex_lock(&buf_latch);

    file_free_page(fd, pagenum);

    pthread_mutex_unlock(&buf_latch);
}

void buf_read_page(int fd, pagenum_t pagenum, page_t* dest){

    pthread_mutex_lock(&buf_latch);

    int i = 0;
    int buffer_index = 0;
    int prev_buf=0, next_buf=0, start_next=0;
    page_t* tmp_page;
    for(i=0; i<buf_size_num;i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == 0 && pagenum == 0 ){
            
            memcpy(dest, buf_pool[i].frame, sizeof(page_t));

            pthread_mutex_unlock(&buf_latch);

            return;
        }                
    }

    for(i=0; i<buf_size_num;i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == pagenum ){
            buf_pool[i].is_pinned += 1;

            memcpy(dest, buf_pool[i].frame, sizeof(page_t));

            pthread_mutex_unlock(&buf_latch);

            return;
        }                
    }
    if(left_buf != 0){
        for(i=0; i<buf_size_num;i++){
   
            if(buf_pool[i].table_id == -1 && buf_pool[i].page_num == -1 ){
                buf_pool[i].table_id = fd;
                buf_pool[i].page_num = pagenum;
                buf_pool[i].is_pinned += 1;

                if(i==0){ 
                    buf_control[start].next = buf_pool[i].buffer_index;
                    buf_control[end].prev = buf_pool[i].buffer_index;

                    buf_pool[i].prev = buf_control[start].buffer_index;
                    buf_pool[i].next = buf_control[end].buffer_index;
                }else{
                    buf_pool[i-1].prev = buf_pool[i].buffer_index;
                    
                    buf_pool[i].next = buf_pool[i-1].buffer_index;
                    buf_pool[i].prev = buf_control[start].buffer_index;                  
                    
                    buf_control[start].next = buf_pool[i].buffer_index;
                }
                left_buf -= 1; 

                file_read_page(fd, pagenum, buf_pool[i].frame);
                memcpy(dest, buf_pool[i].frame, sizeof(page_t));

                pthread_mutex_unlock(&buf_latch);
                
                return;                
            }
        }     
    }else{
        buffer_index = buf_control[end].prev;
        i = 0;

        int flag = 0;
        while(1){
            for(int j=0; j<buf_size_num; j++){
                if(buf_pool[j].is_pinned == 0){
                    flag = 1;
                    break;
                }
            }
            if(flag == 1)
                break;
        }

        while(buf_pool[buffer_index].is_pinned >= 1 && i<buf_size_num){ 
            buffer_index = buf_pool[buffer_index].prev;
            i++;
        }

        prev_buf = buf_pool[buffer_index].prev;
        next_buf = buf_pool[buffer_index].next;

        buf_pool[prev_buf].next = buf_pool[buffer_index].next;
        buf_pool[next_buf].prev = buf_pool[buffer_index].prev;

        start_next = buf_control[start].next;
        
        buf_control[start].next = buf_pool[buffer_index].buffer_index;
        buf_pool[start_next].prev = buf_pool[buffer_index].buffer_index;

        buf_pool[buffer_index].next = buf_pool[start_next].buffer_index;
        buf_pool[buffer_index].prev = buf_control[start].buffer_index;

        if(buf_pool[buffer_index].is_dirty == 1){
            file_write_page(fd, buf_pool[buffer_index].page_num, buf_pool[buffer_index].frame);
            buf_pool[buffer_index].is_dirty = 0; 
        }
 
        buf_pool[buffer_index].table_id = fd;
        
        buf_pool[buffer_index].page_num = pagenum;
        buf_pool[buffer_index].is_pinned += 1; 

        file_read_page(fd, pagenum, buf_pool[buffer_index].frame);
        memcpy(dest, buf_pool[buffer_index].frame, sizeof(page_t));

        pthread_mutex_unlock(&buf_latch);

        return;
    }
}

void buf_write_page(int fd, pagenum_t pagenum, const page_t* src){

    pthread_mutex_lock(&buf_latch);

    for(int i=0; i<buf_size_num; i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == 0 && pagenum == 0){
            memcpy(buf_pool[i].frame, src, sizeof(page_t));
            file_write_page(fd, 0, src);
            
            pthread_mutex_unlock(&buf_latch);
            
            return;
        }
    }
    for(int i=0; i<buf_size_num; i++){
       
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == pagenum ){
            
            buf_pool[i].is_dirty = 1;
            
            buf_pool[i].is_pinned -= 1; 
            
            memcpy(buf_pool[i].frame, src, sizeof(page_t));

            pthread_mutex_unlock(&buf_latch);

            return;
        }
        // for(int i=0; i< buf_size_num; i++){
        //     printf("%d-table.id %d, pagenum %ld, pin %d\n",i, buf_pool[i].table_id, buf_pool[i].page_num, buf_pool[i].is_pinned);
        // }
        // printf("%d, %ld, %d\n", fd, pagenum, i);
    }

    pthread_mutex_unlock(&buf_latch);
}

void buf_close_database_file(){ 

    pthread_mutex_lock(&buf_latch);

    for(int i=0; i<buf_size_num;i++){
        file_write_page(fd, buf_pool[i].page_num, buf_pool[i].frame);
        free(buf_pool[i].frame);
    }
    free(buf_pool);
    free(buf_control);
    
    file_close_database_file();

    pthread_mutex_unlock(&buf_latch);
}