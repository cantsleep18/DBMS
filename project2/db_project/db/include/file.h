#ifndef DB_FILE_H_
#define DB_FILE_H_

#include <stdint.h>

#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)// 10 MiB
#define PAGE_SIZE (4 * 1024)// 4 KiB

extern int fd;

typedef uint64_t pagenum_t;
typedef struct page_t {
	// in-memory page structure
	char page_size[PAGE_SIZE];
}page_t;

typedef struct header_t {
	pagenum_t first_free_page;
	pagenum_t page_num;
	char reserved[4080];
}header_t;

typedef struct free_t {
	pagenum_t next_page_num;
	char reserved[4088];
}free_t;

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