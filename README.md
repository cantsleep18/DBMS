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

***

**Project_3**
=====================
The goal of Project_3 is to implement an in-memory buffer manager for 
caching on-disk pages.
1. **int buf_init**
2. **int buf_open_database_file**
3. **pagenum_t buf_alloc_page**
4. **void buf_free_page**
5. **void buf_read_page**
6. **void buf_write_page**
7. **void buf_close_database_file**
# Initalize global variable
```c
buffer_t * buf_pool = NULL;
buf_control_t * buf_control = NULL;

int buf_size_num = 0;
int left_buf = 0;
```
* Use buf_pool for buffer entries and buf_control for linked list 'head', 'tail'
* buf_size_num is a number of buf entries
* left_buf to check number of left buf entries 

# int buf_init
```c
int buf_init(int num_buf){
    buf_pool = (buffer_t*)malloc(sizeof(buffer_t)*num_buf);
    
    buf_size_num = num_buf;
    left_buf = num_buf;

    for(int i=0; i<num_buf; i++){
        buf_pool[i].frame = (page_t*)malloc(sizeof(page_t));
        buf_pool[i].table_id = -1;
        buf_pool[i].buffer_index = i;
        buf_pool[i].page_num = -1;
        buf_pool[i].is_dirty = 0;
        buf_pool[i].is_pinned = 0;
        buf_pool[i].prev = -1;
        buf_pool[i].next = -1;
    }

    buf_control = (buf_control_t*)malloc(sizeof(buf_control_t)*2);
    buf_control[start].buffer_index = -2;
    buf_control[start].next = -1;
    buf_control[start].prev = -1;

    buf_control[end].buffer_index = -3;
    buf_control[end].next = -1;
    buf_control[end].prev = -1;

    return 0;
}
```
* In buf_init, I allocate memory for buf_pool and buf_control
* Initialized buf_pool and buf_control
* And set up the value of buf_size_num, left_buf

# int buf_open_database_file
```c
int buf_open_database_file(const char* pathname){
    fd = file_open_database_file(pathname);
               
    return fd;
}
```
* Since there was no error with opening database file, I didn't touch open database file

# pagenum_t buf_alloc_page
```c
pagenum_t buf_alloc_page(int fd){ 
    pagenum_t alloc_page_num;
    header_t tmp;
    int i = 0;

    alloc_page_num = file_alloc_page(fd);
    for(i=0; i< buf_size_num; i++){
        if(buf_pool[i].page_num == 0){
            file_read_page(fd, 0, buf_pool[i].frame);
            break;
        }
    }
    return alloc_page_num;
}

```
* My file_alloc_page api does everything inside function, so all I have to do is
just allocate page and find buffer entry with header page and overwrite on it

# void buf_free_page
```c
void buf_free_page(int fd,pagenum_t pagenum){
    file_free_page(fd, pagenum);
}
```

# void buf_read_page
```c
void buf_read_page(int fd, pagenum_t pagenum, page_t* dest){
    int i = 0;
    int buffer_index = 0;
    int prev_buf=0, next_buf=0, start_next=0;
    page_t* tmp_page;
    for(i=0; i<buf_size_num;i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == 0 && pagenum == 0 ){
            
            memcpy(dest, buf_pool[i].frame, sizeof(page_t));

            return;
        }                
    }

    for(i=0; i<buf_size_num;i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == pagenum ){
            buf_pool[i].is_pinned += 1;

            memcpy(dest, buf_pool[i].frame, sizeof(page_t));

            return;
        }                
    }
    if(left_buf != 0){
        for(i=0; i<buf_size_num;i++){
   
            if(buf_pool[i].table_id == -1 && buf_pool[i].page_num == -1 ){
                buf_pool[i].table_id = fd;
                buf_pool[i].page_num = pagenum;
                buf_pool[i].is_pinned += 1;

                if(i==0){ 
                    buf_control[start].next = buf_pool[i].buffer_index;
                    buf_control[end].prev = buf_pool[i].buffer_index;

                    buf_pool[i].prev = buf_control[start].buffer_index;
                    buf_pool[i].next = buf_control[end].buffer_index;
                }else{
                    buf_pool[i-1].prev = buf_pool[i].buffer_index;
                    
                    buf_pool[i].next = buf_pool[i-1].buffer_index;
                    buf_pool[i].prev = buf_control[start].buffer_index;                  
                    
                    buf_control[start].next = buf_pool[i].buffer_index;
                }
                left_buf -= 1; 

                file_read_page(fd, pagenum, buf_pool[i].frame);
                memcpy(dest, buf_pool[i].frame, sizeof(page_t));
                
                return;                
            }
        }     
    }else{
        buffer_index = buf_control[end].prev;
        i = 0;

        int flag = 0;
        while(1){
            for(int j=0; j<buf_size_num; j++){
                if(buf_pool[j].is_pinned == 0){
                    flag = 1;
                    break;
                }
            }
            if(flag == 1)
                break;
        }

        while(buf_pool[buffer_index].is_pinned >= 1 && i<buf_size_num){ 
            buffer_index = buf_pool[buffer_index].prev;
            i++;
        }

        prev_buf = buf_pool[buffer_index].prev;
        next_buf = buf_pool[buffer_index].next;

        buf_pool[prev_buf].next = buf_pool[buffer_index].next;
        buf_pool[next_buf].prev = buf_pool[buffer_index].prev;

        start_next = buf_control[start].next;
        
        buf_control[start].next = buf_pool[buffer_index].buffer_index;
        buf_pool[start_next].prev = buf_pool[buffer_index].buffer_index;

        buf_pool[buffer_index].next = buf_pool[start_next].buffer_index;
        buf_pool[buffer_index].prev = buf_control[start].buffer_index;

        if(buf_pool[buffer_index].is_dirty == 1){
            file_write_page(fd, buf_pool[buffer_index].page_num, buf_pool[buffer_index].frame);
            buf_pool[buffer_index].is_dirty = 0;
        }
 
        buf_pool[buffer_index].table_id = fd;
        
        buf_pool[buffer_index].page_num = pagenum;
        buf_pool[buffer_index].is_pinned += 1; 

        file_read_page(fd, pagenum, buf_pool[buffer_index].frame);
        memcpy(dest, buf_pool[buffer_index].frame, sizeof(page_t));

        return;
    }
}
```
* 1. Check whether the needed pagenum is in buffer entries
* If there are matching page, then just copy the page data
* 2-1. If there are left pages then read pagenum from disk and store other data in buf_pool
* 2-2. If there is no left_buf available, then wait until one of the buf_pool's pin is 0.
* When we get the buffer_index of that buf_pool, put that buf_pool to the head of linked list. So that this system adheres to LRU Policy
* 3. If buf_pool[buffer_index] is dirty, then write that page to disk

# void buf_write_page
```c
void buf_write_page(int fd, pagenum_t pagenum, const page_t* src){
    for(int i=0; i<buf_size_num; i++){
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == 0 && pagenum == 0){
            memcpy(buf_pool[i].frame, src, sizeof(page_t));
            file_write_page(fd, 0, src);
            return;
        }
    }
    for(int i=0; i<buf_size_num; i++){
       
        if(buf_pool[i].table_id == fd && buf_pool[i].page_num == pagenum ){
            
            buf_pool[i].is_dirty = 1;
            
            buf_pool[i].is_pinned -= 1; 
            
            memcpy(buf_pool[i].frame, src, sizeof(page_t));
            return;
        }
    }
}
```
* If the needed pagenum is 0, it means it is header, so I just memcpy, and write data
to disk.
* I don't need to mark it as dirty or pinned
* If not, I find the pagenum in buf_pool and mark it as dirty and pinned.
* And update the frame in buf_pool

# void buf_close_database_file
```c
void buf_close_database_file(){ 
    for(int i=0; i<buf_size_num;i++){
        file_write_page(fd, buf_pool[i].page_num, buf_pool[i].frame);
        free(buf_pool[i].frame);
    }
    free(buf_pool);
    free(buf_control);
    
    file_close_database_file();
}
```
* I write all the pages in buffer to disk(flush)
* And freed allocated variables
* Then closed DB file

***

**Project_4**
=====================
The goal of Project_4 is to implement a lock table module that manages lock objects of
multiple threads. 
caching on-disk pages.
1. **int init_lock_table**
2. **lock_t\* lock_acquire**
3. **int lock_release**

# Structure
```c
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
```
* pair_t to table_id + key structure
* hash_table_t to represent head, tail
* UT_hash_handle to control hash table (uthash.h)
* lock_t to represent lock structure

# init_lock_table
```c
hash_table_t* hash_table = NULL;
pthread_mutex_t lock_table_latch = PTHREAD_MUTEX_INITIALIZER;

int init_lock_table(){
  return 0;
}
```
* I didn't put anything in init_lock_table function
* I couldn't find reason why it gets error when I did
* pthread_mutex_t lock_table_latch;
* lock_table_latch = PTHREAD_MUTEX_INITIALIZER
* So, I just initialized hash_table & lock_table_latch above init_lock_table

# lock_acquire
```c
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
```
* 1. When table_id, key are not in hash table
* 2. When table_id, key are in hash table
* 2-1. When there are no head
* 2-2. When there are head
* I used HASH_ADD & HASH_DEL to manage hash_table
* When new lock is added, then I re-link head and tail of hash_table_entry
* In 2-2, corresponding lock wait until released lock gives signal

# lock_release
```c
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
```
* 1. When parameter lock's hash_table_entry doesn't have head
* 2. When parameter lock's hash_table_entry has only one lock
* 3. When parameter lock's hash_table_entry has more than one lock
* In 1, just return -1
* In 2, free the allocated head and corresponding hash_table_entry
* In 3, Make head linked to next lock and send signal to wake up sleeping process


***

**Project_5**
=====================
The goal of Project_5 is to implement a lock table to bpt module that manages lock objects of
multiple threads. 
My lock manager should provide:
- Conflict-serializable schedule for transactions
- Strict-2PL
- Deadlock detection (abort the transaction if detected)
- Record-level locking with Shared(S)/Exclusive(X) mode

# About Lock mode (shared & exclusive)
* There are some conditions these two lock has to keep. And with this condition, I made sequence.
![캡처](/uploads/c891547b5840e34ea20cc7f77d5c054c/캡처.PNG)

# deadlock
* 수업에서 들을 때는 구현할 수 있을 것이라고 생각했는데... 막상 직접 구현해보니 좀 많이 어렵다고 느꼈습니다.
* deadlock이 발생하는 원인도 알고 있고 해결 방법도 알고 있는데, 제 코드구현 능력이 부족해서 시간이 더 필요할 것 같습니다.
* Test server에서 segment fault가 계속 떠서 원인을 찾느라 현재 큰그림으로 생각중인 deadlock 코드를 테스트 못하였습니다.
* Project6가 나온 시점에서 Project5를 구현하지 못하면 Project6 를 진행할 수 없다고 생각되서 제 능력 범위 내에서 최선을 다할 생각입니다.

# abort and roll back
* deadlock 부분이 구현이 되야지 어떤 부분에서 deadlock detection 을 해야될지 결정할 수 있을 것 같습니다.
* 현재 생각 중인 그림은 deadlock detection 된 시점에서 타겟이 된 trx을 모두 abort하고 trx_hash_table에서 제거하는 것 입니다.
* abort의 과정에서 기존에 백업해두었던 정보들을 다시 복구하고 buf write를 진행할 예정입니다.
* 아마 lock_acquire 함수의 리턴값으로 abort인지 아닌지를 판단하게 될 것 같습니다.

# Structure
1. **int trx_begin(void)**
2. **int trx_commit(int trx_id)**
3. **int db_find(int64_t table_id, int64_t key, char\* ret_val, uint16_t \* val_size, int trx_id)**
4. **int db_update(int64_t table_id, int64_t key, char\* ret_val, uint16_t * val_size, int trx_id)**
5. **lock_t \* lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode)**
6. **int lock_release(lock_t * lock_obj)**

# int trx_begin(void)
```c
int trx_begin(){
    pthread_mutex_lock(&trx_table_latch);

    trx_t *trx = (trx_t*)malloc(sizeof(trx_t));
    
    if(trx_id <= 0 ){
        trx_id = 1;
    }
    
    trx->trx_id = trx_id;

    HASH_ADD(hh, trx_table, trx_id, sizeof(int), trx);

    pthread_mutex_unlock(&trx_table_latch);

    trx_id++;

    return trx->trx_id;
}
```
* I allocated memory for trx.
* Using global variable trx_id, i gave transaction different id.
* I initialized trx with information and added to trx_hash table.
* In here, I put mutex to prevent is process

# int trx_commit(int trx_id)
```c
int trx_commit(int trx_id){
    pthread_mutex_lock(&trx_table_latch);

    lock_t* lock;
    trx_t * trx;
    trx_t * find_result;
    
    trx->trx_id = trx_id;
    
    HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), find_result);
    lock = find_result->next_lock;
    lock_t* tmp_lock = find_result->next_lock;
    lock_release(lock);

    while(tmp_lock != NULL){
        tmp_lock = tmp_lock->trx_next_lock;
        lock = tmp_lock;
        lock_release(lock);
    }

    HASH_DEL(trx_table, find_result);
    free(find_result);
    pthread_mutex_unlock(&trx_table_latch);

    return trx_id;
}
```
* Find the trx_hash table using trx_id and save it to find_result
* release all of the lock in find_result lock linked list using while loop
* Since everything is commited, I have to Delete hash table

# int db_find(int64_t table_id, int64_t key, char\* ret_val, uint16_t \* val_size, int trx_id)
```c
int db_find(int64_t table_id, int64_t key, char *ret_val, uint16_t *val_size, int trx_id){
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
    
    lock_acquire(table_id, leaf_page_num, key, trx_id, 0);

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
```
* Since, buffer, trx, lock have mutex, I don't have to add any mutex.
* So, I just call lock_acquire after I get the pagenum of leaf_page_num

# int db_update(int64_t table_id, int64_t key, char\* ret_val, uint16_t * val_size, int trx_id)
```c
int db_update(int64_t table_id , int64_t key, char* values , uint16_t new_val_size , uint16_t* old_val_size , int trx_id){
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
    
    lock_acquire(table_id, leaf_page_num, key, trx_id, 1);


    for(int i=0; i<leaf.num_of_keys; i++){
        if(leaf.record[i].key == key){
            memcpy(leaf.record[i].value, values, 112);
        }
    }
    
    buf_read_page(table_id, leaf_page_num, (page_t*)&leaf);
    buf_write_page(table_id, leaf_page_num, (page_t*)&leaf);

    return 0;
}
```
* I write the value in parameter on leaf_record with matching keys

# lock_t \* lock_acquire(int64_t table_id, pagenum_t page_id, int64_t key, int trx_id, int lock_mode)
```c
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
    
    trx_t* trx_find_result;

    HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx_find_result);

    lock->trx_next_lock = trx_find_result->next_lock;
    trx_find_result->next_lock = lock;

    pthread_mutex_unlock(&trx_table_latch);

    HASH_ADD(hh, hash_table, pair ,sizeof(pair_t),hash_table_entry);

    pthread_mutex_unlock(&lock_table_latch);

    return lock;
  }else{ // hash_table pair exists
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
        
        trx_t* trx_find_result;

        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx_find_result);

        lock->trx_next_lock = trx_find_result->next_lock;
        trx_find_result->next_lock = lock;

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
        
        trx_t* trx_find_result;

        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx_find_result);

        lock->trx_next_lock = trx_find_result->next_lock;
        trx_find_result->next_lock = lock;

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
        
        trx_t* trx_find_result;

        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx_find_result);

        lock->trx_next_lock = trx_find_result->next_lock;
        trx_find_result->next_lock = lock;

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
        
        trx_t* trx_find_result;

        HASH_FIND(hh, trx_table, &trx->trx_id, sizeof(int), trx_find_result);

        lock->trx_next_lock = trx_find_result->next_lock;
        trx_find_result->next_lock = lock;

        pthread_mutex_unlock(&trx_table_latch);

        pthread_mutex_unlock(&lock_table_latch);

        return lock;
      }
    }
  }
}
```

# int lock_release(lock_t * lock_obj)
```c
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
            
            free(lock_obj);
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
```
