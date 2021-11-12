#include "buffer.h"
#include "bpt.h"
#include <string.h>
#define leaf_order 32
#define inter_order 249

int fd=-1;

int init_db(int num_buf){
    return buf_init(num_buf);
}

int64_t open_table(char *pathname){
    fd = buf_open_database_file(pathname);
    header_t header;

    buf_read_page(fd, 0, (page_t*)&header);
    header.root_page_num = 0;
    
    
    buf_write_page(fd, 0, (page_t*)&header);
    printf("%ld\n", header.root_page_num);
    if(fd < 0)
        return -1;
    return fd;
}

int db_find(int64_t table_id, int64_t key, char *ret_val, uint16_t *val_size){
    header_t header;   
    internal_t node;
    leaf_t leaf;
    pagenum_t next_page_num, leaf_page_num;
    buf_read_page(table_id, 0, (page_t*)&header);
    buf_write_page(table_id, 0, (page_t*)&header);
    if(header.root_page_num == 0){
        return -1;
    }
    
    leaf_page_num = find_leaf(table_id, key, header.root_page_num);
    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);

    printf("leaf_num_of_key: %d\n",leaf.num_of_keys);   
    for(int i=0; i<leaf.num_of_keys; i++){
        printf("db.key: %d\n", leaf.record[i].key);
    }
    printf("-----------------------\n");
    
    for(int i = 0; i<leaf.num_of_keys;i++){
        if(leaf.record[i].key == key){
            memcpy(ret_val, leaf.record[i].value, 112); // check problem
            *val_size = leaf.record[i].size;
            // printf("found key: %d\n",(int)key);
            printf("-----------------------\n");
            return 0;
        }
    }
    return -1;  
}

pagenum_t find_leaf(uint64_t table_id, int64_t key, pagenum_t root_page_num){
    internal_t root;
    pagenum_t next_page_num = root_page_num;  

    buf_read_page(table_id, root_page_num, (page_t*)&root);
    buf_write_page(table_id, root_page_num, (page_t*)&root);
    while(!root.is_leaf){
        int i = 0;
        while(i < root.num_of_keys){ 
            if(key >= root.record[i].key)
                i++;
            else
                break;
        }
        
        if(i==0)
            next_page_num = root.left_page_num;
        else
            next_page_num = root.record[i-1].key_page_num;
        
        buf_read_page(table_id, next_page_num, (page_t*)&root);
        buf_write_page(table_id, next_page_num, (page_t*)&root);
    }
    
    return next_page_num;
}



int db_insert(int64_t table_id, int64_t key, char *value, uint16_t val_size){

    char * insert_value = (char*)malloc(val_size);    
    header_t header;
    leaf_t leaf;
    pagenum_t leaf_page_num;
    
    // printf("key: %d\n",(int)key);
    if(db_find(table_id, key, insert_value, &val_size) == 0){
        // printf("insert_find");
        free(insert_value);
        return -1;
    }

    buf_read_page(table_id, 0, (page_t*)&header);
    buf_write_page(table_id, 0, (page_t*)&header);
    // printf("rootpagenum: %d\n", (int)header.root_page_num);
    if(header.root_page_num == 0){
        start_new_tree(table_id, key, value, val_size);
        free(insert_value);
        return 0;
    }
    
    // printf("root_num: %d\n", header.root_page_num);
    leaf_page_num = find_leaf(table_id, key, header.root_page_num);
    // printf("leaf_num: %d\n", leaf_page_num);
   
    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);
    // printf("leaf_parent: %ld\n", leaf.parent_page_num);
    if(leaf.num_of_keys < leaf_order -1){ // ???
        insert_into_leaf(table_id, leaf_page_num, key, value, val_size);
        free(insert_value);
        return 0;
    }
    free(insert_value);
    
    return insert_into_leaf_after_splitting(table_id, leaf_page_num, key, value, val_size);
}

void start_new_tree(uint64_t table_id, int64_t key, char *value, uint16_t val_size){
    header_t header;
    leaf_t root;
    pagenum_t root_num;
    printf("start_new_tree---------\n"); 
    root_num = buf_alloc_page(table_id);
    buf_read_page(table_id, root_num, (page_t*)&root);
    buf_read_page(table_id, 0, (page_t*)&header);
    header.root_page_num = root_num;
    
    root.parent_page_num = 0;
    root.is_leaf = 1;
    root.num_of_keys = 1;
    root.right_sibling_page_num = 0;
    root.record[0].key = key;
    root.record[0].offset = 4096 - val_size;
    root.record[0].size = val_size;
    memcpy(root.record[0].value, value, 112);

    buf_write_page(table_id, root_num, (page_t*)&root);
    // printf("header root: %ld\n",header.root_page_num);
    buf_write_page(table_id, 0, (page_t*)&header);
    

}

void insert_into_leaf(uint64_t table_id ,pagenum_t leaf_page_num, uint64_t key, char *value, uint16_t val_size){
    leaf_t leaf;
    int insertion_point = 0;
    printf("insert_into_leaf---------\n");
    int i;

    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);

    while(insertion_point < leaf.num_of_keys && leaf.record[insertion_point].key < key){
        insertion_point++;
    }

    for(i = leaf.num_of_keys; i > insertion_point; i--){
        leaf.record[i].key = leaf.record[i-1].key;
        leaf.record[i].offset = leaf.record[i-1].offset; 
        leaf.record[i].size = leaf.record[i-1].size;
        memcpy(leaf.record[i].value, leaf.record[i-1].value, 112);
    }
    leaf.record[insertion_point].key = key;
    leaf.record[insertion_point].offset = leaf.record[insertion_point+1].offset - val_size; //record[i-1]- val_size
    leaf.record[insertion_point].size = val_size;
    memcpy(leaf.record[insertion_point].value, value, 112);
    
    leaf.num_of_keys++;
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);
    
}


int insert_into_leaf_after_splitting(uint64_t table_id ,pagenum_t leaf_page_num, uint64_t key, char *value, uint16_t val_size){
    leaf_t leaf;
    leaf_t tmp;
    leaf_t new_leaf;
    printf("insert_into_leaf_after_splitting---------\n");
    pagenum_t new_leaf_page_num = buf_alloc_page(table_id);

    buf_read_page(table_id, new_leaf_page_num, (page_t*)&new_leaf);
    new_leaf.is_leaf = 1;
    
    int insertion_point = 0;

    int i, j;

    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);    
    pagenum_t tmp_page_num = leaf.right_sibling_page_num;
    leaf.right_sibling_page_num = new_leaf_page_num;
    new_leaf.right_sibling_page_num = tmp_page_num;

    new_leaf.parent_page_num = leaf.parent_page_num;

    while(insertion_point < leaf.num_of_keys && leaf.record[insertion_point].key < key){
        insertion_point++;
    }

    for(i = 0, j = 0; i<leaf.num_of_keys; i++, j++){
        if ( j == insertion_point)
            j++;
        tmp.record[j].key = leaf.record[i].key;
        tmp.record[j].size = leaf.record[i].size;
        memcpy(tmp.record[j].value, leaf.record[i].value, 112);
    }
    tmp.record[insertion_point].key = key;
    tmp.record[insertion_point].size = val_size;
    memcpy(tmp.record[insertion_point].value, value, 112);
    
    int split = cut(leaf_order);
 
    leaf.num_of_keys = 0;
    
    leaf.record[0].offset = 4096 - tmp.record[0].size;
    for(i=1; i<split; i++){
        leaf.record[i].offset = leaf.record[i-1].offset - tmp.record[i].size;
    }
    for(i=0; i<split; i++){
        leaf.record[i].key = tmp.record[i].key;
        leaf.record[i].size = tmp.record[i].size;
        memcpy(leaf.record[i].value, tmp.record[i].value, 112);
        leaf.num_of_keys++;
    }

    new_leaf.num_of_keys = 0;
    leaf.record[split].offset = 4096 - tmp.record[split].size;
    for(i=split+1 ,j=1; i < leaf_order; i++,j++){
        new_leaf.record[j].offset = leaf.record[j-1].offset - tmp.record[i].size;
    }  

    for(i=split ,j=0; i < leaf_order; i++,j++){
        new_leaf.record[j].key = tmp.record[i].key;
        new_leaf.record[j].size = tmp.record[i].size;
        memcpy(new_leaf.record[j].value, tmp.record[i].value, 112);
        new_leaf.num_of_keys++;
    }  

    uint64_t split_key = new_leaf.record[0].key;
    // printf("new_leaf: %ld, leaf: %ld\n", new_leaf_page_num, leaf_page_num);
    buf_write_page(table_id, new_leaf_page_num, (page_t*)&new_leaf);
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);
    
    return insert_into_parent(table_id, split_key,leaf_page_num, new_leaf_page_num);
}

int insert_into_parent(uint64_t table_id, uint64_t key,pagenum_t leaf_page_num, pagenum_t new_leaf_page_num){
    uint64_t left_index;
    printf("insert_into_parent---------\n"); // check option
    internal_t parent;
    leaf_t leaf;
    
    pagenum_t parent_page_num;


    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);
    parent_page_num = leaf.parent_page_num;
    printf("insert_into_parent---------first\n"); // check option
    // printf("parentpage: %d\n",parent_page_num);
    if(parent_page_num == 0)
        return insert_into_new_root(table_id, key, leaf_page_num, new_leaf_page_num);
    
    // printf("insert_into_parent---------second \n"); // check option
    left_index = get_left_index(table_id, parent_page_num, leaf_page_num);
    // printf("left_index: %d\n",left_index);

    buf_read_page(table_id, parent_page_num, (page_t*)&parent);
    buf_write_page(table_id, parent_page_num, (page_t*)&parent);
    // printf("parent.num_of_keys: %ld\n", parent.num_of_keys);
    if(parent.num_of_keys < inter_order-1){
        // printf("parent.num_of_keys: %d\n",parent.num_of_keys);
        return insert_into_node(table_id, key, leaf_page_num, new_leaf_page_num, left_index);
    }
    return insert_into_node_after_splitting(table_id, key, leaf_page_num, new_leaf_page_num, left_index);
}

int insert_into_new_root(uint64_t table_id, uint64_t key, pagenum_t leaf_page_num, pagenum_t new_leaf_page_num){
    header_t header;
    internal_t root;
    leaf_t left, right;
    printf("insert_into_new_root---------\n"); // check option
    pagenum_t root_page_num = buf_alloc_page(table_id);
    // printf("node page alloc: %ld\n", root_page_num);
    
    buf_read_page(table_id, root_page_num, (page_t*)&root);

    root.is_leaf = 0;
    root.left_page_num = leaf_page_num;
    root.num_of_keys = 1; // ??
    root.parent_page_num = 0;
    root.record[0].key = key;
    root.record[0].key_page_num = new_leaf_page_num;

    buf_write_page(table_id, root_page_num, (page_t*)&root);

    buf_read_page(table_id, leaf_page_num, (page_t*)&left);
    left.parent_page_num = root_page_num;
    buf_write_page(table_id, leaf_page_num, (page_t*)&left);

    buf_read_page(table_id, new_leaf_page_num, (page_t*)&right);
    right.parent_page_num = root_page_num;
    buf_write_page(table_id, new_leaf_page_num, (page_t*)&right);

    buf_read_page(table_id, 0, (page_t*)&header);
    header.root_page_num = root_page_num;
    buf_write_page(table_id, 0, (page_t*)&header);
    
    return 0;
}

int insert_into_node(uint64_t table_id, uint64_t key, pagenum_t left_page_num, 
                        pagenum_t new_right_page_num, uint64_t left_index){
    internal_t parent;
    internal_t left;
    pagenum_t parent_page_num;

    printf("Insert_into_node---------\n");

    buf_read_page(table_id, left_page_num, (page_t*)&left);
    buf_write_page(table_id, left_page_num, (page_t*)&left);
    parent_page_num = left.parent_page_num;

    buf_read_page(table_id, parent_page_num, (page_t*)&parent);

    for(int i= parent.num_of_keys; i>left_index+1; i--){
        parent.record[i].key =  parent.record[i-1].key;
        parent.record[i].key_page_num = parent.record[i-1].key_page_num;
    }

    parent.num_of_keys++; 
    parent.record[left_index+1].key_page_num = new_right_page_num;
    parent.record[left_index+1].key = key;    
    
    buf_write_page(table_id, parent_page_num, (page_t*)&parent);

    return 0;
}

int insert_into_node_after_splitting(uint64_t table_id, uint64_t key,pagenum_t left_page_num,
                                        pagenum_t new_right_page_num, uint64_t left_index){
    
    printf("Insert_into_node_after_splitting---------\n");

    int i, j;
    internal_t old_node, new_node, tmp,left;
    pagenum_t old_node_page_num, new_node_page_num;

    buf_read_page(table_id, left_page_num, (page_t*)&left);
    buf_write_page(table_id, left_page_num, (page_t*)&left);
    old_node_page_num = left.parent_page_num;

    buf_read_page(table_id, old_node_page_num, (page_t*)&old_node);

    for(i=0, j=0; i<old_node.num_of_keys; i++, j++){
        if(j == left_index+1)
            j++;
        tmp.record[j].key_page_num = old_node.record[i].key_page_num;
        tmp.record[j].key = old_node.record[i].key;
    }

    tmp.record[left_index+1].key_page_num = new_right_page_num; 
    tmp.record[left_index+1].key = key;

    int split = cut(inter_order-1); // ???

    new_node_page_num = buf_alloc_page(table_id);

    buf_read_page(table_id, new_node_page_num, (page_t*)&new_node);
    new_node.is_leaf = 0;
    new_node.left_page_num = tmp.record[split].key_page_num;
    new_node.num_of_keys = 0;
    new_node.parent_page_num = old_node.parent_page_num;

    old_node.num_of_keys = 0;

    for(i=0;i<split;i++){
        old_node.record[i].key_page_num = tmp.record[i].key_page_num;
        old_node.record[i].key = tmp.record[i].key;
        old_node.num_of_keys++;
    }   
    uint64_t k_prime = tmp.record[split].key;

    for(++i, j=0; i<inter_order; i++, j++){
        new_node.record[j].key_page_num = tmp.record[i].key_page_num;
        new_node.record[j].key = tmp.record[i].key;
        new_node.num_of_keys++;
    }

    buf_write_page(table_id, old_node_page_num, (page_t*)&old_node);
    buf_write_page(table_id, new_node_page_num, (page_t*)&new_node);

    internal_t tmp_page;

    for(i=0; i<new_node.num_of_keys;i++){
        buf_read_page(table_id, new_node.record[i].key_page_num, (page_t*)&tmp_page);
        tmp_page.parent_page_num = new_node_page_num;
        buf_write_page(table_id, new_node.record[i].key_page_num, (page_t*)&tmp_page);
    }
        buf_read_page(table_id, new_node.left_page_num, (page_t*)&tmp_page);
        tmp_page.parent_page_num = new_node_page_num;
        buf_write_page(table_id, new_node.left_page_num, (page_t*)&tmp_page);
    
    return insert_into_parent(table_id, k_prime, old_node_page_num, new_node_page_num);
}

int db_delete(int64_t table_id, int64_t key){
    pagenum_t key_page_num;
    header_t header;
    leaf_t key_page;
    printf("db_delete-------------------\n");
    char * tmp_value = (char*)malloc(112);    
    uint16_t tmp_size;

    buf_read_page(table_id, 0, (page_t*)&header);
    buf_write_page(table_id, 0, (page_t*)&header);
    // buf_write_page(table_id, 0, (page_t*)&header);
    key_page_num =find_leaf(table_id, key, header.root_page_num);
    
    if(db_find(table_id, key, tmp_value, &tmp_size) != 0){
        free(tmp_value);
        printf("db_delete_find_fail--------\n");
        return -1;
    }
    free(tmp_value);
    delete_entry(table_id, header.root_page_num, key_page_num, key);
}

int delete_entry(uint64_t table_id, pagenum_t root_page_num, pagenum_t key_page_num, uint64_t key){
    header_t header;
    internal_t key_page;
    internal_t parent_page;
    internal_t neighbor_page;
    int neighbor_index;
    int k_prime_index;
    pagenum_t neighbor_page_num;
    int64_t k_prime;
    printf("delete_entry-------------------\n");
    remove_entry_from_node(table_id, key_page_num, key);
    buf_read_page(table_id, key_page_num, (page_t*)&key_page);
    buf_write_page(table_id, key_page_num, (page_t*)&key_page);
    
    buf_read_page(table_id, 0, (page_t*)&header);
    buf_write_page(table_id, 0, (page_t*)&header);

    if(key_page_num == root_page_num){
        adjust_root(table_id, root_page_num);
        return 0;
    } 

    if(key_page.num_of_keys>0) //???
        return 0;
    
    neighbor_index = get_neighbor_index(table_id, key_page_num);
    
    buf_read_page(table_id, key_page.parent_page_num, (page_t*)&parent_page);
    buf_write_page(table_id, key_page.parent_page_num, (page_t*)&parent_page);
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = parent_page.record[k_prime_index].key;
    neighbor_page_num = neighbor_index == -1 ? parent_page.record[0].key_page_num : parent_page.record[neighbor_index-1].key_page_num;
    if(neighbor_index == 0)
        neighbor_page_num = parent_page.left_page_num;

    int capacity = key_page.is_leaf ? leaf_order : inter_order-1;

    buf_read_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);
    buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);

    if(neighbor_page.num_of_keys + key_page.num_of_keys < capacity){
        coalesce_nodes(table_id, root_page_num, key_page_num, neighbor_page_num, neighbor_index, k_prime, key);
    }else{
        redistribute_nodes(table_id, root_page_num, key_page_num, neighbor_page_num, 
                                        neighbor_index, k_prime_index, k_prime);
    }
    printf("pass\n");
    return 0;
}

void remove_entry_from_node(int64_t table_id, pagenum_t key_page_num, uint64_t key){
    leaf_t leaf;
    internal_t internal;
    int i = 0,j =0, num_pointer;
    printf("remove_entry_from_node-------------------\n");

    buf_read_page(table_id, key_page_num, (page_t*)&leaf);
    buf_write_page(table_id, key_page_num,(page_t*)&leaf);
    if(leaf.is_leaf == 1){
        buf_read_page(table_id, key_page_num, (page_t*)&leaf);
        while(leaf.record[i].key != key)
            i++;
        j = i;
        for(++i; i<leaf.num_of_keys; i++){ // need to fix , addition change? leftmost like
            leaf.record[i-1].key = leaf.record[i].key;
            leaf.record[i-1].offset = leaf.record[i].offset-leaf.record[j].offset;
            leaf.record[i-1].size = leaf.record[i].size;
            memcpy(leaf.record[i-1].value, leaf.record[i].value, 112);
        }
        leaf.num_of_keys--;
        buf_write_page(table_id, key_page_num,(page_t*)&leaf);
    }else{
        buf_read_page(table_id, key_page_num, (page_t*)&internal);
        while(internal.record[i].key != key)
            i++;
        for(++i; i<internal.num_of_keys; i++){ // need to fix , addition change? leftmost like
            internal.record[i-1].key = internal.record[i].key;
            internal.record[i-1].key_page_num = internal.record[i].key_page_num;
        }
        internal.num_of_keys--;
        buf_write_page(table_id, key_page_num,(page_t*)&internal);
    }   

}

void adjust_root(uint64_t table_id,pagenum_t root_page_num){
    internal_t root;
    internal_t new_root;
    pagenum_t new_root_page_num;
    header_t header;
    printf("adjust_root-------------------\n");
    buf_read_page(table_id, 0, (page_t*)&header);
    buf_read_page(table_id, root_page_num, (page_t*)&root);

    if(root.num_of_keys > 0)
        return;

    if(!root.is_leaf){
        new_root_page_num = root.left_page_num;
        header.root_page_num = new_root_page_num;

        buf_read_page(table_id, new_root_page_num, (page_t*)&new_root);
        new_root.parent_page_num = 0;

        buf_write_page(table_id, 0, (page_t*)&header);
        buf_write_page(table_id, new_root_page_num, (page_t*)&new_root);

    }else{
        header.root_page_num = 0;
        buf_write_page(table_id, 0, (page_t*)&header);

    }
    buf_write_page(table_id, root_page_num, (page_t*)&root);
    buf_free_page(table_id, root_page_num);
}

int get_neighbor_index(uint64_t table_id, pagenum_t key_page_num){
    printf("get_neighbor_index-------------------\n");
    internal_t key_page;
    internal_t parent_page;

    buf_read_page(table_id, key_page_num, (page_t*)&key_page);
    buf_write_page(table_id, key_page_num, (page_t*)&key_page);
    buf_read_page(table_id, key_page.parent_page_num, (page_t*)&parent_page);
    buf_write_page(table_id, key_page.parent_page_num, (page_t*)&parent_page);
    for(int i =0; i <= parent_page.num_of_keys; i++){
        if(parent_page.record[i].key_page_num == key_page_num)
            return i;
    }
    if(parent_page.left_page_num == key_page_num){
        return -1;        
    }


    return 0;
}

void coalesce_nodes(uint64_t table_id,pagenum_t root_page_num,pagenum_t key_page_num, 
                    pagenum_t neighbor_page_num,pagenum_t neighbor_index,
                    pagenum_t k_prime, uint64_t key){
    int i, j, neighbor_insertion_index, tmp;
    internal_t neighbor_page;
    internal_t key_page;
    internal_t tmp_page;

    leaf_t neighbor_page_leaf;
    leaf_t key_page_leaf;
    pagenum_t tmp_page_num, key_page_parent_num;
    printf("coalesce_nodes-------------------\n");
    
    if(neighbor_index == -1){
        tmp_page_num = key_page_num;
        key_page_num = neighbor_page_num;
        neighbor_page_num = tmp_page_num;
    }                    

    buf_read_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);
    buf_read_page(table_id, key_page_num, (page_t*)&key_page);

    buf_read_page(table_id, neighbor_page_num, (page_t*)&neighbor_page_leaf);
    buf_read_page(table_id, key_page_num, (page_t*)&key_page_leaf);

    neighbor_insertion_index = neighbor_page.num_of_keys;

    if(!key_page.is_leaf){
        neighbor_page.record[neighbor_insertion_index].key = k_prime;
        neighbor_page.record[neighbor_insertion_index].key_page_num = key_page.left_page_num; 
        neighbor_page.num_of_keys++;

        tmp = key_page.num_of_keys;
        for(i = neighbor_insertion_index+1, j=0; j<tmp; i++, j++){
            neighbor_page.record[i].key = key_page.record[j].key;
            neighbor_page.record[i].key_page_num = key_page.record[j].key_page_num;
            neighbor_page.num_of_keys++;
            key_page.num_of_keys--;
        }

        for(i = 0; i < neighbor_page.num_of_keys;i++){
            buf_read_page(table_id, neighbor_page.record[i].key_page_num, (page_t*)&tmp_page);
            tmp_page.parent_page_num = neighbor_page_num;
            buf_write_page(table_id, neighbor_page.record[i].key_page_num, (page_t*)&tmp_page);
        }
        buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page_leaf);
        buf_write_page(table_id, key_page_num, (page_t*)&key_page_leaf);
        buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page); //added
        buf_write_page(table_id, key_page_num, (page_t*)&key_page); 
    }else{    
        for(i=neighbor_insertion_index, j=0; j<key_page.num_of_keys; i++, j++){//fixhere
            neighbor_page_leaf.record[i].key = key_page_leaf.record[j].key;
            neighbor_page_leaf.record[i].size = key_page_leaf.record[j].size;
            neighbor_page_leaf.record[i].offset = neighbor_page_leaf.record[i-1].offset + key_page_leaf.record[j].size;
            memcpy(neighbor_page_leaf.record[i].value, key_page_leaf.record[j].value, 112);
            neighbor_page_leaf.num_of_keys++;
        }
        neighbor_page.left_page_num = key_page.left_page_num;
        buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page); //added
        buf_write_page(table_id, key_page_num, (page_t*)&key_page); 
        buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page_leaf); //added
        buf_write_page(table_id, key_page_num, (page_t*)&key_page_leaf);
    }
    key_page_parent_num = key_page.parent_page_num;
    buf_free_page(table_id, key_page_num);
    delete_entry(table_id, root_page_num, key_page_parent_num, k_prime);
}

void redistribute_nodes(uint64_t table_id,pagenum_t root_page_num,pagenum_t key_page_num,pagenum_t neighbor_page_num, 
                                        pagenum_t neighbor_index,pagenum_t k_prime_index,pagenum_t k_prime){
    internal_t root, key_page, neighbor_page, parent, tmp;          
    int i = 0;
    printf("redistribute_nodes-------------------\n");
    buf_read_page(table_id, root_page_num, (page_t*)&root);
    buf_read_page(table_id, key_page_num, (page_t*)&key_page);
    buf_read_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);
    if(neighbor_index != -1){
        if(!key_page.is_leaf){
            key_page.record[key_page.num_of_keys+1].key_page_num = key_page.record[key_page.num_of_keys].key_page_num;
        }
        for(i=key_page.num_of_keys; i>0; i--){
            key_page.record[i].key = key_page.record[i-1].key;
            key_page.record[i].key_page_num = key_page.record[i-1].key_page_num;
        }
        if(!key_page.is_leaf){
            key_page.record[0].key_page_num = neighbor_page.record[neighbor_page.num_of_keys].key_page_num;
            buf_read_page(table_id, key_page.record[0].key_page_num, (page_t*)&tmp);
            tmp.parent_page_num = key_page_num; // ??
            buf_write_page(table_id, key_page.record[0].key_page_num, (page_t*)&tmp);
            neighbor_page.record[neighbor_page.num_of_keys].key_page_num = 0;
            key_page.record[0].key = k_prime;

            buf_read_page(table_id,key_page.parent_page_num, (page_t*)&parent);
            parent.record[k_prime_index].key = neighbor_page.record[neighbor_page.num_of_keys-1].key;
            buf_write_page(table_id,key_page.parent_page_num, (page_t*)&parent);
        }
        else{
            key_page.record[0].key_page_num = neighbor_page.record[neighbor_page.num_of_keys-1].key_page_num;
            neighbor_page.record[neighbor_page.num_of_keys-1].key_page_num = 0;
            key_page.record[0].key = neighbor_page.record[neighbor_page.num_of_keys-1].key;
            buf_read_page(table_id,key_page.parent_page_num, (page_t*)&parent);
            parent.record[k_prime_index].key = key_page.record[0].key;
            buf_write_page(table_id,key_page.parent_page_num, (page_t*)&parent);
        }
    }else{
        if(key_page.is_leaf){
            key_page.record[key_page.num_of_keys].key = neighbor_page.record[0].key;
            key_page.record[key_page.num_of_keys].key_page_num = neighbor_page.record[0].key_page_num;
            buf_read_page(table_id,key_page.parent_page_num, (page_t*)&parent);
            parent.record[k_prime_index].key = neighbor_page.record[1].key;
            buf_write_page(table_id,key_page.parent_page_num, (page_t*)&parent);
        }else{
            key_page.record[key_page.num_of_keys].key = k_prime;
            key_page.record[key_page.num_of_keys+1].key_page_num = neighbor_page.record[0].key_page_num;
            buf_read_page(table_id, key_page.record[key_page.num_of_keys+1].key_page_num, (page_t*)&tmp);
            tmp.parent_page_num = key_page_num;
            buf_write_page(table_id, key_page.record[key_page.num_of_keys+1].key_page_num,(page_t*)&tmp);
            buf_read_page(table_id,key_page.parent_page_num, (page_t*)&parent);
            parent.record[k_prime_index].key = neighbor_page.record[0].key;
            buf_write_page(table_id,key_page.parent_page_num, (page_t*)&parent);
        }
        for (i=0; i < neighbor_page.num_of_keys-1; i++){
            neighbor_page.record[i].key = neighbor_page.record[i+1].key;
            neighbor_page.record[i].key_page_num = neighbor_page.record[i+1].key_page_num;
        }
        if(!key_page.is_leaf)
            neighbor_page.record[i].key_page_num = neighbor_page.record[i+1].key_page_num;
    }

    key_page.num_of_keys++;
    neighbor_page.num_of_keys--;
    buf_write_page(table_id, root_page_num, (page_t*)&root);
    buf_write_page(table_id, key_page_num, (page_t*)&key_page);
    buf_write_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);
}

int shutdown_db(){
    buf_close_database_file();
    
    return 0;
}

int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}

uint64_t get_left_index(uint64_t table_id, pagenum_t parent_page_num, pagenum_t left_page_num){
    uint64_t left_index = 0;
    internal_t parent, left;

    buf_read_page(table_id, parent_page_num, (page_t*)&parent);
    buf_write_page(table_id, parent_page_num, (page_t*)&parent);
    buf_read_page(table_id, left_page_num, (page_t*)&left);
    buf_write_page(table_id, left_page_num, (page_t*)&left);
    
    while(left_index <= parent.num_of_keys && parent.record[left_index].key_page_num != left_page_num){
        left_index++;
    }
    if(parent.left_page_num == left_page_num)
        return -1;
    
    return left_index;
}
