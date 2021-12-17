#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdint.h>

typedef struct log_t log_t;

struct log_t{
    int log_size;
    int64_t lsn;
    int64_t prev_lsn;
    int trx_id;
    int log_type;
    int64_t table_id;
    int64_t page_num;
    int offset;
    int data_len;
    char old_img[80];
    char new_img[80];
};


#endif /* __LOG_H__ */