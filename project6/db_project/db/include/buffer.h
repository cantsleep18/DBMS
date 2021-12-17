#include "file.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lock_table.h"

#define start 0
#define end 1

typedef struct buffer_t{
    page_t* frame;
    int table_id;
    int buffer_index;
    pagenum_t page_num;
    int is_dirty;
    int is_pinned;
    pthread_mutex_t buf_pool_latch;
    int prev;
    int next;
}buffer_t;

typedef struct buf_control_t{
    int buffer_index;
    int prev;
    int next;
}buf_control_t;

static pthread_mutex_t buf_latch;

int buf_init(int num_buf);

// Open existing database file or create on if not existed
int buf_open_database_file(const char* pathname);

// Allocate an on-disk page from the free page list
pagenum_t buf_alloc_page(int fd);

// Free an on-disk page to the free page list
void buf_free_page(int fd,pagenum_t pagenum);

// Read an on-disk page into the in-memory page structure(dest)
void buf_read_page(int fd, pagenum_t pagenum, page_t* dest);

// Write an in-memory page(src) to the on-disk page
void buf_write_page(int fd, pagenum_t pagenum, const page_t* src);

// Stop referencing the database file
void buf_close_database_file();
