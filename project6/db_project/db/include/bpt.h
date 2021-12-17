#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lock_table.h"
#include "buffer.h"

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

#include "file.h"

// DB
int init_db();
int64_t open_table(char *pathname);
int db_insert(int64_t table_id, int64_t key, char *value, uint16_t val_size);
int db_find(int64_t table_id, int64_t key, char *ret_val, uint16_t *val_size, int trx_id);
int db_update(int64_t table_id , int64_t key, char* values , uint16_t new_val_size , uint16_t* old_val_size , int trx_id);
int db_delete(int64_t table_id, int64_t key);
int shutdown_db();

// FUNCTION PROTOTYPES.
pagenum_t find_leaf(uint64_t table_id, int64_t key, pagenum_t root_page_num);
void start_new_tree(uint64_t table_id, int64_t key, char *value, uint16_t val_size);
void insert_into_leaf(uint64_t table_id ,pagenum_t leaf_page_num, uint64_t key, char *value, uint16_t val_size);
int insert_into_leaf_after_splitting(uint64_t table_id ,pagenum_t leaf_page_num, uint64_t key, char *value, uint16_t val_size);
int insert_into_parent(uint64_t table_id, uint64_t key,pagenum_t leaf_page_num, pagenum_t new_leaf_page_num);
int insert_into_new_root(uint64_t table_id, uint64_t key, pagenum_t leaf_page_num, pagenum_t new_leaf_page_num);
int insert_into_node(uint64_t table_id, uint64_t key, pagenum_t left_page_num, 
                        pagenum_t new_right_page_num, uint64_t left_index);
int insert_into_node_after_splitting(uint64_t table_id, uint64_t key, pagenum_t left_page_num, 
                        pagenum_t new_right_page_num, uint64_t left_index);

int db_delete(int64_t table_id, int64_t key);
int delete_entry(uint64_t table_id, pagenum_t root_page_num, pagenum_t key_page_num, uint64_t key);
void remove_entry_from_node(int64_t table_id, pagenum_t key_page_num, uint64_t key);
void adjust_root(uint64_t table_id,pagenum_t root_page_num);
int get_neighbor_index(uint64_t table_id, pagenum_t key_page_num);
void coalesce_nodes(uint64_t table_id,pagenum_t root_page_num,pagenum_t key_page_num, 
                    pagenum_t neighbor_page_num,pagenum_t neighbor_index,
                    pagenum_t k_prime,uint64_t key);
void redistribute_nodes(uint64_t table_id,pagenum_t root_page_num,pagenum_t key_page_num,pagenum_t neighbor_page_num, 
                                        pagenum_t neighbor_index,pagenum_t k_prime_index,pagenum_t k_prime);
 

//util
int cut( int length );
uint64_t get_left_index(uint64_t table_id, pagenum_t parent_page_num, pagenum_t left_page_num);

#endif