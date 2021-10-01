#include <stdio.h>
#include <fcntl.h>
#include "file.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int fd=-1;

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

// Read an on-disk page into the in-memory page structure(dest)
void file_read_page(int fd, pagenum_t pagenum, page_t* dest) {
	pread(fd, dest, PAGE_SIZE, pagenum * PAGE_SIZE);
	sync();
};

// Write an in-memory page(src) to the on-disk page
void file_write_page(int fd, pagenum_t pagenum, const page_t* src) {
	pwrite(fd, src, PAGE_SIZE, pagenum * PAGE_SIZE);
	sync();
};

// Stop referencing the database file
void file_close_database_file() {
    close(fd);
};