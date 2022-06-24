**Project_2 milestone_1**
=====================
Project 2's Main Object is to implement disk-based b+tree based on my disk space manager.    
In milestone_1, I designed my own Disk Space Manager APIs.
1. **file_open_database_file**
2. **file_alloc_page**
3. **file_free_page**
4. **file_read_page**
5. **file_write_page**
6. **file_close_database_file**


# Structure of 'file.h'

## Macro
```c
#include <stdint.h>

#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)// 10 MiB
#define PAGE_SIZE (4 * 1024)// 4 KiB
```
* INITIAL_DB_FILE_SIZE is size of a database file
* PAGE_SIZE is size of one page
## pagenum_t & page_t
```c
typedef uint64_t pagenum_t;
typedef struct page_t {
	// in-memory page structure
	char page_size[PAGE_SIZE];
}page_t;
```
* pagenum_t : datatype for byte range [0-7]
* page_t : datatype for in-memory page structure 

## header_t
```c
typedef struct header_t {
	pagenum_t first_free_page;
	pagenum_t page_num;
	char reserved[4080];
}header_t;
```
* header_t : datatype for Header Page Format
* byte range [0-7] : Free Page Number
* byte range [8-15] : Number of Pages
* byte range [16-4095] : reserved
## free_t
```c
typedef struct free_t {
	pagenum_t next_page_num;
	char reserved[4088];
}free_t;
```
* free_t : datatype for Free Page Format
* byte range [0-7] : Next Free Page Number
* byte range [8-4095] : reserved
## Declaration of APIs
```c
// Open existing database file or create on if not existed
int file_open_database_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int fd);

// Free an on-disk page to the free page list
void file_free_page(int fd,pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int fd, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void file_write_page(int fd, pagenum_t pagenum, const page_t* src);

// Stop referencing the database file
void file_close_database_file();
```
# Structure of 'file.cc'
## file_open_database_file
```c
// Open existing database file or create on if not existed
int file_open_database_file(const char* pathname) {
   header_t root;
   pagenum_t free_init_pages_num;
   
   fd = open(pathname, O_RDWR, 0644);
   if(fd!=-1)
      return fd;

   fd = open(pathname, O_RDWR|O_CREAT|O_EXCL , 0644);

   //initialize
   int init_free_num = INITIAL_DB_FILE_SIZE/PAGE_SIZE-1; 
   
   free_t * free_pages;
   free_pages = (free_t*)malloc(sizeof(page_t)*init_free_num);

   root.first_free_page = 0;
   root.page_num = 1;

   root.first_free_page = root.page_num;
   root.page_num = init_free_num+1;

   for(int i = 0; i<(init_free_num); i++){ 
      if(i == (init_free_num-1)){
         free_pages[i].next_page_num = 0; 
         pwrite(fd, (page_t *)&free_pages[i], PAGE_SIZE, PAGE_SIZE*(i+1));
         sync();
         break;
      }
      
      free_pages[i].next_page_num = i+2;  
      pwrite(fd, (page_t *)&free_pages[i], PAGE_SIZE, PAGE_SIZE*(i+1)); 
      sync();
   }

   pwrite(fd, (page_t*)&root, PAGE_SIZE, 0);
   sync();

   free(free_pages);


   return fd;
};
```
* I check whether file exists using **open(pathname, O_RDWR, 0644)**. It returns -1 when the file doesn't exist.   
Then used **'open(pathname, O_RDWR|O_CREAT|O_EXCL , 0644)'** to make a new file.
* Since I got problem with **Segmentation Fault**, I did dynamic memory allocation.   
By initializing 'init_free_num' to INITIAL_DB_FILE_SIZE/PAGE_SIZE-1, I got size of free pages except for header page.
* In for loop, I tried to link free page to free page using **free_pages[i].next_page_num**.   
Right after linking, I writed **data of free_pages[i]** to applicable **on-disk-memory**
* Finally, I write **data of root**(next free page, number of page) to off-set 0

## file_alloc_page
```c
// Allocate an on-disk page from the free page list
pagenum_t file_alloc_page(int fd) {
	
	header_t header_page;
	free_t free_page;
	pagenum_t first_free_page;

    pread(fd, (page_t*)&header_page, PAGE_SIZE, 0);
	sync();

	first_free_page = header_page.first_free_page;
	
	// if next page doesn't exist
	if (first_free_page == 0) {
		pagenum_t init_num = header_page.page_num;

		int tmp = header_page.page_num;

   		free_t * free_pages;
    	free_pages = (free_t*)malloc(sizeof(page_t)*init_num);

   		header_page.first_free_page = header_page.page_num;
   		header_page.page_num += init_num;

    	for(int i = 0; i<(init_num); i++){ 
     		if(i == (init_num-1)){
        	free_pages[i].next_page_num = 0; 
        	pwrite(fd, (page_t *)&free_pages[i], PAGE_SIZE, PAGE_SIZE*(tmp));
        	sync();
       	 	break;
     	 	}
      
			free_pages[i].next_page_num = tmp+1;
			pwrite(fd, (page_t *)&free_pages[i], PAGE_SIZE, PAGE_SIZE*(tmp));
			sync();
			tmp++;      
   		}
		pwrite(fd, (page_t*)&header_page, PAGE_SIZE, 0);

   		free(free_pages);

		first_free_page = header_page.first_free_page;
	}

    pread(fd, (page_t*)&free_page, PAGE_SIZE, PAGE_SIZE*(first_free_page));
	sync();
	header_page.first_free_page = free_page.next_page_num;
    pwrite(fd, (page_t*)&header_page, PAGE_SIZE, 0);
	sync();
	return first_free_page;
};
```
* First, get data of header page to check whether the **first_free_page** is 0.   
If **first_free_page** is **0**, then it means there are no free pages to allocate.
* So, If first_free_page is 0, I should extend my database size as double of existing size.    And process of extending db file is similar with code when I initialize database file.
* When header file is ready, I cut off the connection between **Last IN** two free pages and connected later one of **header page
* Then, I return new page #

## file_free_page
```c
// Free an on-disk page to the free page list
void file_free_page(int fd, pagenum_t pagenum) {
	header_t header_page;
	free_t free_page;

    pread(fd, (page_t*)&header_page, PAGE_SIZE, 0);
	sync();

	free_page.next_page_num = header_page.first_free_page;
	header_page.first_free_page = pagenum;

	pwrite(fd, (page_t*)&header_page, PAGE_SIZE, 0);
	sync();
	pwrite(fd, (page_t*)&free_page, PAGE_SIZE, pagenum*PAGE_SIZE);
	sync();
};
```
* To free an on-disk page, I put value of header.first_free_page to make free_page point the page which header was pointing     and connected **header page** with **on-disk-page_offset** which is location of free_page
* I wrote updated data(**header_page, free_page**) to on-disk-memory

## file_read_page
```c
// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int fd, pagenum_t pagenum, page_t* dest) {
	pread(fd, dest, PAGE_SIZE, pagenum * PAGE_SIZE);
	sync();
};
```
* Function **pread** is **(read() + lseek()).** 
* It **reads n-byte into buf from fd at the given position offset**.
* I did sync() after pread to synchronize disk and memory pages.

## file_write_page
```c
// Write an in-memory page(src) to the on-disk page
void file_write_page(int fd, pagenum_t pagenum, const page_t* src) {
	pwrite(fd, src, PAGE_SIZE, pagenum * PAGE_SIZE);
	sync();
};
```
* Function **pwrite** is **(write() + lseek()).** 
* It **write n-bytes of buf to fd at the given position offset**.
* I did sync() after pwrite to synchronize disk and memory pages.
# Unittest
##
```c
TEST(FileInitTest, HandlesInitialization) {
  int fd;                                 // file descriptor
  std::string pathname = "init_test111.db";  // customize it to your test file
  pagenum_t allocated_page, allocated_page2, freed_page;
  header_t header;
  free_t tmp;
  // Open a database file
  fd = file_open_database_file(pathname.c_str());

  // Check if the file is opened
  ASSERT_TRUE(fd > 0);  // change the condition to your design's behavior

  // Check the size of the initial file
  file_read_page(fd, 0 ,(page_t*)&header);
  int num_pages = header.page_num/* fetch the number of pages from the header page  2560 */;
  EXPECT_EQ(num_pages, INITIAL_DB_FILE_SIZE / PAGE_SIZE)
      << "The initial number of pages does not match the requirement: "
      << num_pages;

  // to check open_database initialize
  file_read_page(fd, 4, (page_t*)&tmp);
  EXPECT_EQ(tmp.next_page_num, 5) ;
```
* **EXPECT_EQ(num_pages, INITIAL_DB_FILE_SIZE / PAGE_SIZE)** : To check my db file initialization success
* EXPECT_EQ(tmp.next_page_num, 5) : To check my db file initialization success (more specific) 
```c
// Allocate the pages
  
  allocated_page = file_alloc_page(fd);
  EXPECT_EQ(allocated_page,1);

  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(header.first_free_page,2);

  allocated_page2 = file_alloc_page(fd);
  EXPECT_EQ(allocated_page2,2);

  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(header.first_free_page,3);

  // to test free page
  file_free_page(fd, allocated_page);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(header.first_free_page, 1);

  file_free_page(fd, allocated_page2);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(header.first_free_page, 2);

```
* I executed file_alloc_page to see return of file_alloc_page works well.
* I also checked header.first_free_page to see my **file_read_page** and **file_write_page**
* Without testing **file_read_page** and **file_write_page**, I can know that these two function works well   
since my two functions consists of (pread/pwrite + sync)
* I freed twice to see whether **file_free_page** works well with given parameters
```c
  // to test extend_func
  pagenum_t tmp2 = 0;
  for (int i = 0; i < 2558; i++)
  {
    tmp2 = file_alloc_page(fd);
  }
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(header.first_free_page,2559);

  //end of db
  tmp2 = file_alloc_page(fd);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(tmp2, 2559);
  EXPECT_EQ(header.first_free_page,0);
  EXPECT_EQ(header.page_num, 2560);

  // // extend
  tmp2 = file_alloc_page(fd);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(tmp2, 2560);
  EXPECT_EQ(header.first_free_page,2561);
  EXPECT_EQ(header.page_num, 5120);
  
  tmp2 = file_alloc_page(fd);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(tmp2, 2561);
  EXPECT_EQ(header.first_free_page,2562);
  EXPECT_EQ(header.page_num, 5120);

  tmp2 = file_alloc_page(fd);
  file_read_page(fd, 0, (page_t*)&header);
  EXPECT_EQ(tmp2, 2562);
  EXPECT_EQ(header.first_free_page,2563);
  EXPECT_EQ(header.page_num, 5120);
```
* To double check my **file_alloc_page**(when next file doesn't exist), I extend the db file size by keep using **file_alloc_page**
* Once I extend the db file, I could see my header's first_free_page and page_num growing up.


***


**Project_2 milestone_2**
=====================
Project 2's Main Object is to Implement a disk based b+ tree supporting a variable length field.    
In milestone_2, I implemented disk-based b+ tree.
1. **int init_db***
2. **int64_t open_table***
3. **int db_insert**
4. **int db_find**
5. **int db_delete**
6. **int shutdown_db**
# int init_db
* Use this function to initialize my database management system.
* In project2, I don't use this function. I initialize my database management system in other functions
# int64_t open_table
```c
int64_t open_table(char *pathname){
    fd = file_open_database_file(pathname);

    header_t header;

    file_read_page(fd, 0, (page_t*)&header);
    header.root_page_num = 0;
   
    file_write_page(fd, 0, (page_t*)&header);

    if(fd < 0)
        return -1;
    return fd;
}
```
* open the database file and do a basic initialization
# db_find
```c
int db_find(int64_t table_id, int64_t key, char *ret_val, uint16_t *val_size){
    header_t header;   
    internal_t node;
    leaf_t leaf;
    pagenum_t next_page_num, leaf_page_num;
    file_read_page(table_id, 0, (page_t*)&header);
    if(header.root_page_num == 0){
        return -1;
    }
        
    leaf_page_num = find_leaf(table_id, key, header.root_page_num);
    file_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    
    for(int i = 0; i<leaf.num_of_keys;i++){
        if(leaf.record[i].key == key){
            memcpy(ret_val, leaf.record[i].value, 112); // check problem
            *val_size = leaf.record[i].size;
            return 0;
        }
    }
    return -1;  
}
```
* Uses db_find to find the record containing input 'key'
* if root page number is 0, then it means there is no root page.
* check the leaf.record[i].key == key to find out matching record
# db_find - find_leaf
```c
pagenum_t find_leaf(uint64_t table_id, int64_t key, pagenum_t root_page_num){
    internal_t root;
    pagenum_t next_page_num = root_page_num;  

    file_read_page(table_id, root_page_num, (page_t*)&root);

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
        
        file_read_page(table_id, next_page_num, (page_t*)&root);
    }
    
    return next_page_num;
}
```
* Uses this function to go into tree
* With the loop, I can reach the leaf page which contains the target key

# db_insert
```c
int db_insert(int64_t table_id, int64_t key, char *value, uint16_t val_size){

    char * insert_value = (char*)malloc(val_size);    
    header_t header;
    leaf_t leaf;
    pagenum_t leaf_page_num;
    
    if(db_find(table_id, key, insert_value, &val_size) == 0){
        free(insert_value);
        return -1;
    }

    file_read_page(table_id, 0, (page_t*)&header);
    if(header.root_page_num == 0){
        start_new_tree(table_id, key, value, val_size);
        free(insert_value);
        return 0;
    }
    
    leaf_page_num = find_leaf(table_id, key, header.root_page_num);

    file_read_page(table_id, leaf_page_num, (page_t*)&leaf);

    if(leaf.num_of_keys < leaf_order ){
        insert_into_leaf(table_id, leaf_page_num, key, value, val_size);
        free(insert_value);
        return 0;
    }
    free(insert_value);
    
    return insert_into_leaf_after_splitting(table_id, leaf_page_num, key, value, val_size);
}
```
* Uses this function to insert input with its size to data file at the right place
* If the inserting value already exists, then stop
* root page number is 0, then make root page
* If page doesn't exceed, just put key in it
* In other case go into insert_into_leaf_after_splitting
## insert_into_leaf_after_splitting
```c
int insert_into_leaf_after_splitting(uint64_t table_id ,pagenum_t leaf_page_num, uint64_t key, char *value, uint16_t val_size){
    leaf_t leaf;
    leaf_t tmp;
    leaf_t new_leaf;
    pagenum_t new_leaf_page_num = file_alloc_page(table_id);

    file_read_page(table_id, new_leaf_page_num, (page_t*)&new_leaf);
    new_leaf.is_leaf = 1;
    
    int insertion_point = 0;

    int i, j;

    file_read_page(table_id, leaf_page_num, (page_t*)&leaf);    
    pagenum_t tmp_page_num = leaf.right_sibling_page_num;
    leaf.right_sibling_page_num = new_leaf_page_num;
    new_leaf.right_sibling_page_num = tmp_page_num;

    new_leaf.parent_page_num = leaf.parent_page_num;

    while(insertion_point < leaf.is_leaf && leaf.record[insertion_point].key < key){
        insertion_point++;
    }

    for(i = 0, j = 0; i<leaf.num_of_keys; i++, j++){
        if ( j == insertion_point)
            j++;
        tmp.record[j].key = leaf.record[i].key;
        tmp.record[j].offset = leaf.record[i].offset - val_size;
        tmp.record[j].size = leaf.record[i].size;
        memcpy(tmp.record[j].value, leaf.record[i].value, 112);
    }
    tmp.record[insertion_point].key = key;
    tmp.record[insertion_point].offset = leaf.record[i].offset - val_size;
    tmp.record[insertion_point].size = val_size;
    memcpy(tmp.record[insertion_point].value, value, 112);
    
    int split = cut(leaf_order);
 
    leaf.num_of_keys = 0;
    for(i=0; i<split; i++){
        leaf.record[i].key = tmp.record[i].key;
        leaf.record[i].offset = tmp.record[i].offset;
        leaf.record[i].size = tmp.record[i].size;
        memcpy(leaf.record[i].value, tmp.record[i].value, 112);
        leaf.num_of_keys++;
    }

    new_leaf.num_of_keys = 0;
    for(i=split ,j=0; i < leaf_order; i++,j++){
        new_leaf.record[j].key = tmp.record[i].key;
        new_leaf.record[j].offset = tmp.record[i].offset;
        new_leaf.record[j].size = tmp.record[i].size;
        memcpy(new_leaf.record[j].value, tmp.record[i].value, 112);
        new_leaf.num_of_keys++;
    }  

    uint64_t split_key = new_leaf.record[0].key;

    file_write_page(table_id, leaf_page_num, (page_t*)&leaf);
    file_write_page(table_id, new_leaf_page_num, (page_t*)&new_leaf);

    return insert_into_parent(table_id, split_key,leaf_page_num, new_leaf_page_num);
}
```
* In here, we divide exceeding page into two pages
* I should allocate new page to divide keys
* Then toss it to insert_into_parent
## insert_into_parent
```c
int insert_into_parent(uint64_t table_id, uint64_t key,pagenum_t leaf_page_num, pagenum_t new_leaf_page_num){
    uint64_t left_index;
    internal_t parent;
    leaf_t leaf;
    
    pagenum_t parent_page_num;


    file_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    parent_page_num = leaf.parent_page_num;
    if(parent_page_num == 0)
        return insert_into_new_root(table_id, key, leaf_page_num, new_leaf_page_num);
    
    left_index = get_left_index(table_id, parent_page_num, leaf_page_num);
    file_read_page(table_id, parent_page_num, (page_t*)&parent);
    
    if(parent.num_of_keys < inter_order-1){
        return insert_into_node(table_id, key, leaf_page_num, new_leaf_page_num, left_index);
    }
    return insert_into_node_after_splitting(table_id, key, leaf_page_num, new_leaf_page_num, left_index);
}
```
* Use this function to connect pages we made before
* If those page don't have parent then insert_into_new_root
* If parent page don't exceed then just insert_into_node
* If exceeds then I have to split parent node and put it in
# insert_into_node_after_splitting
```c
int insert_into_node_after_splitting(uint64_t table_id, uint64_t key,pagenum_t left_page_num,
                                        pagenum_t new_right_page_num, uint64_t left_index){
    int i, j;
    internal_t old_node, new_node, tmp,left;
    pagenum_t old_node_page_num, new_node_page_num;

    file_read_page(table_id, left_page_num, (page_t*)&left);
    old_node_page_num = left.parent_page_num;

    file_read_page(table_id, old_node_page_num, (page_t*)&old_node);

    for(i=0, j=0; i<old_node.num_of_keys; i++, j++){
        if(j == left_index+1)
            j++;
        tmp.record[j].key_page_num = old_node.record[i].key_page_num;
        tmp.record[j].key = old_node.record[i].key;
    }

    tmp.record[left_index+1].key_page_num = new_right_page_num; 
    tmp.record[left_index+1].key = key;

    int split = cut(inter_order);

    new_node_page_num = file_alloc_page(table_id);

    file_read_page(table_id, new_node_page_num, (page_t*)&new_node);
    new_node.is_leaf = 0;
    new_node.left_page_num = tmp.record[split].key_page_num;//?
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

    file_write_page(table_id, old_node_page_num, (page_t*)&old_node);
    file_write_page(table_id, new_node_page_num, (page_t*)&new_node);

    internal_t tmp_page;

    for(i=0; i<new_node.num_of_keys;i++){
        file_read_page(table_id, new_node.record[i].key_page_num, (page_t*)&tmp_page);
        tmp_page.parent_page_num = new_node_page_num;
        file_write_page(table_id, new_node.record[i].key_page_num, (page_t*)&tmp_page);
    }
        file_read_page(table_id, new_node.left_page_num, (page_t*)&tmp_page);
        tmp_page.parent_page_num = new_node_page_num;
        file_write_page(table_id, new_node.left_page_num, (page_t*)&tmp_page);
    return insert_into_parent(table_id, k_prime, old_node_page_num, new_node_page_num);
}
``` 
* Does similar function as insert_into_leaf_after_splitting
* This function ends with call insert_into_parent which I called before
* So that I can connect pages
## db_delete
```c
int db_delete(int64_t table_id, int64_t key){
    pagenum_t key_page_num;
    header_t header;
    leaf_t key_page;

    char * tmp_value = (char*)malloc(112);    
    uint16_t tmp_size;

    file_read_page(table_id, 0, (page_t*)&header);
    
    key_page_num =find_leaf(table_id, key, header.root_page_num);

    if(db_find(table_id, key, tmp_value, &tmp_size) != 0)
        return -1;
    
    delete_entry(table_id, header.root_page_num, key_page_num, key);
}
```
* Use this function to find the matching record and delete it if found
* First, check whether the matching record exists using db_find
* Then, go into delete_entry

# delete entry
```c
int delete_entry(uint64_t table_id, pagenum_t root_page_num, pagenum_t key_page_num, uint64_t key){
    header_t header;
    internal_t key_page;
    internal_t parent_page;
    internal_t neighbor_page;
    int neighbor_index;
    int k_prime_index;
    pagenum_t neighbor_page_num;
    int64_t k_prime;
    

    remove_entry_from_node(table_id, key_page_num, key);
    file_read_page(table_id, key_page_num, (page_t*)&key_page);

    file_read_page(table_id, 0, (page_t*)&header);

    if(key_page_num == header.root_page_num)
        adjust_root(table_id, header.root_page_num);

    file_read_page(table_id, key_page_num, (page_t*)&key_page);

    if(key_page.num_of_keys>0)
        return 0;
    
    neighbor_index = get_neighbor_index(table_id, key_page_num);
    
    file_read_page(table_id, key_page.parent_page_num, (page_t*)&parent_page);
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = parent_page.record[k_prime_index].key;
    neighbor_page_num = neighbor_index == -1 ? parent_page.record[1].key_page_num : parent_page.record[neighbor_index].key_page_num;
    int capacity = key_page.is_leaf ? leaf_order : inter_order-1;

    file_read_page(table_id, neighbor_page_num, (page_t*)&neighbor_page);

    if(neighbor_page.num_of_keys + key_page.num_of_keys < capacity)
        coalesce_nodes(table_id, root_page_num, key_page_num, neighbor_page_num, neighbor_index, k_prime, key);
    else
        redistribute_nodes(table_id, root_page_num, key_page_num, neighbor_page_num, 
                                        neighbor_index, k_prime_index, k_prime);
    return 0;
}
```
* Uses this function to remove entry or merge or redistribution if i need
* If neighbor page's keys + key page's keys is under capacity of page, then do coalesce
* else do some redistribution to maintain pages

## Process of Project_2
* I think I successfully finished milestone_1. I did some unit test with it and my api worked well on my disk-based b+tree. Since this was my first programming assignment in CS major, I think I had kind of hard time with this.
```
rootpagenum: 457
32
insert_into_leaf_after_splitting---------
insert_into_parent---------
insert_into_parent---------first
parentpage: 457
insert_into_parent---------second 
left_index: 2
parent.num_of_keys: 30
Insert_into_node---------
key: 7889
db.key: 7547
db.key: 7566
db.key: 7592
db.key: 7607
db.key: 7625
db.key: 7627
db.key: 7629
db.key: 7638
db.key: 7642
db.key: 7645
db.key: 7654
db.key: 7685
db.key: 7691
db.key: 7694
db.key: 7744
db.key: 7748
db.key: 7764
db.key: 7779
db.key: 7794
db.key: 7815
db.key: 7828
db.key: 7829
db.key: 7830
db.key: 7854
db.key: 7856
db.key: 7873
db.key: 7926
db.key: 7927
db.key: 7947
db.key: 7959
couldn't find
rootpagenum: 457
30
```
* I made some test file with files that TAs gave us. I saw that my insertion and find operation works when I test with individual operation. But when I executed with default file that TAs gave me, insertion worked well and find also worked well, but in many case, I couldn't find the key using db_find. Did some a lot of debugging using printf.
* I have to retest and do everything I can do to fix this...! 
* Before starting project_3, I have to fix operations of project2
