#include <stdint.h>

#define INITIAL_DB_FILE_SIZE (10 * 1024 * 1024)// 10 MiB
#define PAGE_SIZE (4 * 1024)// 4 KiB

typedef uint64_t pagenum_t;

typedef struct page_t {
	// in-memory page structure
	char page_size[PAGE_SIZE];
}page_t;

typedef struct header_t {
	pagenum_t first_free_page;
	pagenum_t page_num;
	pagenum_t root_page_num;
	char reserved[4072];
}header_t;

typedef struct free_t {
	pagenum_t next_page_num;
	char reserved[4088];
}free_t;

typedef struct internal_t_record{
	uint64_t key;
	pagenum_t key_page_num;
}internal_t_record;

typedef struct internal_t {
	pagenum_t parent_page_num;
	uint32_t is_leaf;
	uint32_t num_of_keys;

    char reserved[104];

	pagenum_t left_page_num;

	internal_t_record record[248];
}internal_t;

typedef struct leaf_t_record{
	uint64_t key;
    uint16_t size;
    uint16_t offset;
	char value[112];	
}leaf_t_record;

typedef struct leaf_t{
	pagenum_t parent_page_num;
	uint32_t is_leaf;
	uint32_t num_of_keys;

	char reserved[104];

	uint64_t right_sibling_page_num;

    leaf_t_record record[64];
}leaf_t;