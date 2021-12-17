#ifndef __LOCK_TABLE_H__
#define __LOCK_TABLE_H__

#include "trx.h"
#include "file.h"
#include <stdint.h>
#include "uthash.h"
#include <pthread.h>
#include <stdio.h>

#define WAIT 0
#define ACQUIRED 1
#define WAITING 2

typedef struct lock_t lock_t;
typedef struct hash_table_t hash_table_t;
typedef struct pair_t pair_t;

struct lock_t {
  lock_t* prev;
  lock_t* next;
  hash_table_t* sentinel;
  pthread_cond_t con;  
  int64_t record_id;
  int lock_mode;
  lock_t* trx_next_lock;
  int lock_status;
  int owner_trx_id;
};
 
struct pair_t
{
  int64_t table_id;
  int64_t page_id; 
};

struct hash_table_t
{
  pair_t pair;
  lock_t* head;
  lock_t* tail;
  UT_hash_handle hh;
};

static pthread_mutex_t lock_table_latch;
static hash_table_t* hash_table;

/* APIs for lock table */
int init_lock_table();
lock_t *lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode);
int lock_release(lock_t* lock_obj);

#endif /* __LOCK_TABLE_H__ */