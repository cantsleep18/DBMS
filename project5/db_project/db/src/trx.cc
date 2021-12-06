#include "uthash.h"
#include <pthread.h>
#include <stdio.h>
#include "trx.h"

int trx_id = 1;

int trx_begin(){
    pthread_mutex_lock(&trx_table_latch);
    trx_t *trx = (trx_t*)malloc(sizeof(trx_t));
    if(trx_id <= 0 ){
        trx_id = 1;
    }
    trx->trx_id = trx_id;
    trx->next_lock =NULL;
    trx->next_trx = NULL;
    HASH_ADD(hh, trx_table, trx_id, sizeof(int), trx);
    pthread_mutex_unlock(&trx_table_latch);

    trx_id++;

    return trx->trx_id;
}

int trx_commit(int trx_id){
    pthread_mutex_lock(&trx_table_latch);

    lock_t* lock;
    trx_t * trx = NULL;
    trx->trx_id = trx_id;
    
    HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx);
    lock = trx->next_lock;
    lock_t* tmp_lock = trx->next_lock;
    lock_release(lock);

    lock_t* tmp_tail = tmp_lock->sentinel->tail;
    
    while(tmp_lock != tmp_tail){
        tmp_lock = tmp_lock->trx_next_lock;
        lock = tmp_lock;
        lock_release(lock);
    }
    HASH_DEL(trx_table, trx);
    free(trx);
    pthread_mutex_unlock(&trx_table_latch);
}