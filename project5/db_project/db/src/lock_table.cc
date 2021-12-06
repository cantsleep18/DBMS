#include "lock_table.h"

int init_lock_table(){
  lock_table_latch = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  hash_table = NULL;
  
  return 0;
}

lock_t* lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode) {
  pthread_mutex_lock(&lock_table_latch);

  lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
  trx_t * trx;
  hash_table_t* hash_table_entry = (hash_table_t*)malloc(sizeof(hash_table_t));
  
  trx->trx_id = trx_id;
  lock->con = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

  hash_table_entry->pair.table_id = table_id;
  hash_table_entry->pair.page_id = page_id;

  hash_table_t* find_result;
  HASH_FIND(hh, hash_table, &hash_table_entry->pair, sizeof(pair_t), find_result);

  if(find_result == NULL){ // // No table_id, page_id pair
    // printf("acquire: 1\n");
    hash_table_entry->head = (lock_t*)malloc(sizeof(lock_t));
    hash_table_entry->tail = (lock_t*)malloc(sizeof(lock_t));

    hash_table_entry->head->next = lock;
    hash_table_entry->tail->prev = lock;

    lock->prev = NULL;
    lock->next = NULL;
 
    lock->sentinel = hash_table_entry;

    lock->record_id = key;
    lock->lock_mode = lock_mode;
    lock->lock_status = ACQUIRED;
    lock->owner_trx_id = trx_id;


    pthread_mutex_lock(&trx_table_latch);
    
    HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);

    lock->trx_next_lock = trx->next_lock;
    trx->next_lock = lock;

    pthread_mutex_unlock(&trx_table_latch);

    HASH_ADD(hh, hash_table, pair ,sizeof(pair_t),hash_table_entry);

    pthread_mutex_unlock(&lock_table_latch);

    return lock;
  }else{ // pair exists
    hash_table_entry = find_result;

    int x_lock_flag = 0; // check (same record_id, lock type X) 
    lock_t * tmp_lock = NULL;
    tmp_lock = find_result->head->next;
    
    // when given lock_mode is S-LOCk
    if(lock_mode == 0){
      // check where to put lock
      while(tmp_lock != NULL){
        if (tmp_lock->owner_trx_id == trx_id){
          pthread_mutex_unlock(&lock_table_latch);
          
          return tmp_lock;
        }else if(tmp_lock->record_id == key&&tmp_lock->lock_mode == 1){
          x_lock_flag = 1;
          break;
        }
        tmp_lock = tmp_lock->next; 
      }
      
      if(x_lock_flag == 0){
        lock->prev = hash_table_entry->tail->prev; // lock <- new lock
        hash_table_entry->tail->prev->next = lock; // lock -> new lock
        lock->next = NULL; // new lock -> NULL
        hash_table_entry->tail->prev = lock; // new lock <- tail

        lock->sentinel = hash_table_entry;

        lock->record_id = key;
        lock->lock_mode = lock_mode;
        lock->owner_trx_id = trx_id;
        lock->lock_status = ACQUIRED;

        pthread_mutex_lock(&trx_table_latch);
        
        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);

        lock->trx_next_lock = trx->next_lock;
        trx->next_lock = lock;

        pthread_mutex_unlock(&trx_table_latch);

        pthread_mutex_unlock(&lock_table_latch);

        return lock;
      }else{ // x_lock_flag == 1
        lock->prev = hash_table_entry->tail->prev; // lock <- new lock
        hash_table_entry->tail->prev->next = lock; // lock -> new lock
        lock->next = NULL; // new lock -> NULL
        hash_table_entry->tail->prev = lock; // new lock <- tail

        lock->sentinel = hash_table_entry;

        lock->record_id = key;
        lock->lock_mode = lock_mode;
        lock->owner_trx_id = trx_id;
        lock->lock_status = WAITING;

        pthread_mutex_lock(&trx_table_latch);
        
        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);

        lock->trx_next_lock = trx->next_lock;
        trx->next_lock = lock;

        pthread_mutex_unlock(&trx_table_latch);

        pthread_cond_wait(&lock->con, &lock_table_latch);
        
        pthread_mutex_unlock(&lock_table_latch);

        return lock;
      }
    }else if(lock_mode == 1){ // given lock is LOCK X
      
      int same_lock_flag = 0;
      
      while(tmp_lock != NULL){
        //check with same trx_id
        if(tmp_lock->owner_trx_id == trx_id && tmp_lock->lock_mode == 0){
          tmp_lock->lock_mode == 1;
          return tmp_lock;
        }else if(tmp_lock->owner_trx_id == trx_id && tmp_lock->lock_mode == 1){
          return  tmp_lock;
        }

        // same recorded lock
        if (tmp_lock->record_id == key){
          same_lock_flag = 1;
        }
        tmp_lock = tmp_lock->next; 
      }

      if(same_lock_flag == 1){
        lock->prev = hash_table_entry->tail->prev; // lock <- new lock
        hash_table_entry->tail->prev->next = lock; // lock -> new lock
        lock->next = NULL; // new lock -> NULL
        hash_table_entry->tail->prev = lock; // new lock <- tail

        lock->sentinel = hash_table_entry;

        lock->record_id = key;
        lock->lock_mode = lock_mode;
        lock->owner_trx_id = trx_id;
        lock->lock_status = WAITING;

        pthread_mutex_lock(&trx_table_latch);
        
        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);

        lock->trx_next_lock = trx->next_lock;
        trx->next_lock = lock;

        pthread_mutex_unlock(&trx_table_latch);

        pthread_cond_wait(&lock->con, &lock_table_latch);
        
        pthread_mutex_unlock(&lock_table_latch);

        return lock;
      }else{
        lock->prev = hash_table_entry->tail->prev; // lock <- new lock
        hash_table_entry->tail->prev->next = lock; // lock -> new lock
        lock->next = NULL; // new lock -> NULL
        hash_table_entry->tail->prev = lock; // new lock <- tail

        lock->sentinel = hash_table_entry;

        lock->record_id = key;
        lock->lock_mode = lock_mode;
        lock->owner_trx_id = trx_id;
        lock->lock_status = ACQUIRED;

        pthread_mutex_lock(&trx_table_latch);
        
        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);

        lock->trx_next_lock = trx->next_lock;
        trx->next_lock = lock;

        pthread_mutex_unlock(&trx_table_latch);

        pthread_mutex_unlock(&lock_table_latch);

        return lock;
      }
    }
  }
};

int lock_release(lock_t* lock_obj){
  pthread_mutex_lock(&lock_table_latch);
  
  lock_t*tmp_lock = NULL;
  int first_lock_flag = -1;
  int tmp_record = -1;

  if(lock_obj->sentinel->head == NULL){
    // printf("release: 1\n");
    pthread_mutex_unlock(&lock_table_latch);
    return -1;
  }else if(lock_obj->sentinel->head->next == lock_obj->sentinel->tail->prev){
    // printf("release: 2\n");
    lock_obj->sentinel->head->next = NULL;
    lock_obj->sentinel->tail->prev = NULL;

    HASH_DEL(hash_table, lock_obj->sentinel);
    
    free(lock_obj->sentinel->head);
    free(lock_obj->sentinel->tail);
    free(lock_obj->sentinel);
  }else{
    tmp_lock = lock_obj;
    lock_obj->prev->next = lock_obj->next;
    lock_obj->next->prev = lock_obj->prev;
    
    while(tmp_lock != NULL){
          if(tmp_lock->lock_status == WAITING && tmp_lock->lock_mode == 0){
            tmp_lock->lock_status == ACQUIRED;

            tmp_record = tmp_lock->record_id;

            pthread_cond_signal(&(tmp_lock->con));
            first_lock_flag = 0;
          }else if(tmp_lock->lock_status == WAITING && tmp_lock->lock_mode == 1){
            tmp_lock->lock_status == ACQUIRED;
            pthread_cond_signal(&(tmp_lock->con));
            
            pthread_mutex_unlock(&lock_table_latch);
            return 0;
          }

          if(first_lock_flag == 0){
            while(tmp_lock != NULL){
              if(tmp_lock->lock_mode ==1 && tmp_lock->record_id == tmp_record){
                break;
              }else if(tmp_lock->lock_status == WAITING && tmp_lock->lock_mode == 0 && tmp_lock->record_id == tmp_record){
                tmp_lock->lock_status = ACQUIRED;
                pthread_cond_signal(&(tmp_lock->con));
              }
              tmp_lock = tmp_lock->next; 
            }
            break;
          }


      tmp_lock = tmp_lock->next; 
    }
  }
  free(lock_obj);
 
  pthread_mutex_unlock(&lock_table_latch);

  return 0;
}
