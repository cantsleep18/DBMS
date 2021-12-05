#include "uthash.h"
#include <pthread.h>
#include <stdio.h>
#include "trx.h"

int trx_id = 1;

int trx_init(){ //이 선언에는 스토리지 클래스 또는 형식 지정자가 없습니다. error keep occurs so I made init function
    trx_table_latch = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    trx_table = NULL;
}

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
    
    while(tmp_lock!=NULL){
        tmp_lock = tmp_lock->trx_next_lock;
        lock = tmp_lock;
        lock_release(lock);
    }
    pthread_mutex_unlock(&trx_table_latch);
}