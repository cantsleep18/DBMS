#include "lock_table.h"
#include "uthash.h"
#include <pthread.h>
#include <stdio.h>

struct pair_t
{
  int64_t table_id;
  int64_t key; 
};

struct hash_table_t
{
  pair_t pair;
  lock_t* head;
  lock_t* tail;
  UT_hash_handle hh;
};

struct lock_t {
  lock_t* prev;
  lock_t* next;
  hash_table_t* sentinel;
  pthread_cond_t con;  
};

typedef struct lock_t lock_t;
typedef struct hash_table_t hash_table_t;
typedef struct pair_t pair_t;


hash_table_t* hash_table = NULL;
pthread_mutex_t lock_table_latch = PTHREAD_MUTEX_INITIALIZER;

int init_lock_table(){
  return 0;
}

lock_t* lock_acquire(int64_t table_id, int64_t key) {
  pthread_mutex_lock(&lock_table_latch);

  lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
  hash_table_t* hash_table_entry = (hash_table_t*)malloc(sizeof(hash_table_t));
  
  lock->con = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

  hash_table_entry->pair.table_id = table_id;
  hash_table_entry->pair.key = key;

  hash_table_t* find_result;
  HASH_FIND(hh, hash_table, &hash_table_entry->pair, sizeof(pair_t), find_result);

  if(find_result == NULL){
    // printf("acquire: 1\n");
    hash_table_entry->head = (lock_t*)malloc(sizeof(lock_t));
    hash_table_entry->tail = (lock_t*)malloc(sizeof(lock_t));

    hash_table_entry->head->next = lock;
    hash_table_entry->tail->prev = lock;

    lock->prev = NULL;
    lock->next = NULL;
 
    lock->sentinel = hash_table_entry;

    HASH_ADD(hh, hash_table, pair ,sizeof(pair_t),hash_table_entry);
  }else{
    if(hash_table->head == NULL){
      printf("acquire: 2\n");
      HASH_DEL(hash_table, find_result);

      hash_table_entry->head = (lock_t*)malloc(sizeof(lock_t));
      hash_table_entry->tail = (lock_t*)malloc(sizeof(lock_t));

      hash_table_entry->head->next = lock;
      hash_table_entry->tail->prev = lock;
      
      lock->prev = NULL;
      lock->next = NULL;
      lock->sentinel = hash_table_entry;
      
      HASH_ADD(hh, hash_table, pair ,sizeof(pair_t),hash_table_entry);
    }else{
      // printf("acquire: 3\n");
      hash_table_entry = find_result;
      HASH_DEL(hash_table, find_result);

      lock->prev = hash_table_entry->tail->prev; // lock <- new lock
      hash_table_entry->tail->prev->next = lock; // lock -> new lock
      lock->next = NULL; // new lock -> NULL
      hash_table_entry->tail->prev = lock; // new lock <- tail

      
      HASH_ADD(hh, hash_table, pair ,sizeof(pair_t),hash_table_entry);
      lock->sentinel = hash_table_entry;
      pthread_cond_wait(&lock->con, &lock_table_latch);
    } 
  }
  
  pthread_mutex_unlock(&lock_table_latch);

  return lock;
};

int lock_release(lock_t* lock_obj){
  pthread_mutex_lock(&lock_table_latch);
  
  if(lock_obj->sentinel->head == NULL){
    // printf("release: 1\n");
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
    // printf("release: 3\n");
    lock_t* new_head = lock_obj->next;
    lock_obj->sentinel->head->next = new_head;
    lock_obj->prev = NULL;
    
    pthread_cond_signal(&(new_head->con));
  }
  free(lock_obj);
 
  pthread_mutex_unlock(&lock_table_latch);

  return 0;
}
