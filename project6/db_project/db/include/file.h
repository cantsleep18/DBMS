#ifndef DB_FILE_H_
#define DB_FILE_H_

#include <stdint.h>
#include "page.h"

extern int fd;


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

#endif  // DB_FILE_H_