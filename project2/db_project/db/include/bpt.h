#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

#include "file.h"

// Default order is 4.
#define DEFAULT_ORDER 4

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 20

// Constants for printing part or all of the GPL license.


#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// TYPES.

/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */
// typedef struct record {
//     int value;
// } record;

/* Type representing a node in the B+ tree.
 * This type is general enough to serve for both
 * the leaf and the internal node.
 * The heart of the node is the array
 * of keys and the array of corresponding
 * pointers.  The relation between keys
 * and pointers differs between leaves and
 * internal nodes.  In a leaf, the index
 * of each key equals the index of its corresponding
 * pointer, with a maximum of order - 1 key-pointer
 * pairs.  The last pointer points to the
 * leaf to the right (or NULL in the case
 * of the rightmost leaf).
 * In an internal node, the first pointer
 * refers to lower nodes with keys less than
 * the smallest key in the keys array.  Then,
 * with indices i starting at 0, the pointer
 * at i + 1 points to the subtree with keys
 * greater than or equal to the key in this
 * node at index i.
 * The num_keys field is used to keep
 * track of the number of valid keys.
 * In an internal node, the number of valid
 * pointers is always num_keys + 1.
 * In a leaf, the number of valid pointers
 * to data is always num_keys.  The
 * last leaf pointer points to the next leaf.
 */
// typedef struct node {
//     void ** pointers;
//     int * keys;
//     struct node * parent;
//     bool is_leaf;
//     int num_keys;
//     struct node * next; // Used for queue.
// } node;

// GLOBALS.

/* The order determines the maximum and minimum
 * number of entries (keys and pointers) in any
 * node.  Every node has at most order - 1 keys and
 * at least (roughly speaking) half that number.
 * Every leaf has as many pointers to data as keys,
 * and every internal node has one more pointer
 * to a subtree than the number of keys.
 * This global variable is initialized to the
 * default value.
 */
extern int order;

/* The queue is used to print the tree in
 * level order, starting from the root
 * printing each entire rank on a separate
 * line, finishing with the leaves.
 */
// extern node * queue;

/* The user can toggle on and off the "verbose"
 * property, which causes the pointer addresses
 * to be printed out in hexadecimal notation
 * next to their corresponding keys.
 */
// extern bool verbose_output;

// DB
int init_db();
int64_t open_table(char *pathname);
int db_insert(int64_t table_id, int64_t key, char *value, uint16_t val_size);
int db_find(int64_t table_id, int64_t key, char *ret_val, uint16_t *val_size);
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