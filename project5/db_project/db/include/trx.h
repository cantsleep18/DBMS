#ifndef __TRX_H__
#define __TRX_H__

#include "uthash.h"
#include "lock_table.h"
#include <pthread.h>
#include <stdio.h>

typedef struct trx_t trx_t;
typedef struct lock_t lock_t;

struct trx_t{
    int trx_id;
    lock_t* next_lock;
    trx_t* next_trx;
    UT_hash_handle hh;
};

pthread_mutex_t trx_table_latch;
trx_t * trx_table;

int trx_init();
int trx_begin();
int trx_commit(int trx_id);

#endif