#ifndef EXTERNAL_SORT_H
#define EXTERNAL_SORT_H

#include "list.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>


#define MAX_RECORDS_SIZE (1<<23)
#define MAX_RECORDS_INDEX (MAX_RECORDS_SIZE-1)

// typedef long long int64_t;
// typedef long int rec_t;

#define GB ((int64_t)1 << 30)
#define MB ((int64_t)1 << 20)

struct run_ele {
    int32_t* records;
    struct list_head list;
    int64_t len;
    char in_mem; // there is a array stored in disk
    const char* pathname;
};

typedef struct run_ele run_item_t;
typedef run_item_t* run_item_ptr_t;

/* New a run item*/
int new_runItem(run_item_ptr_t*);

int read_file(const char* pathname, struct list_head* head);


void gen_random(char*, const int);
char* str_assign(const char*);


void store_run(const char*, run_item_ptr_t);
run_item_ptr_t read_run(const char*);

int parse_int(char** ptr, off_t* dx, off_t st_size);


struct list_head* external_sort(struct list_head*);

struct list_head* external_merge(struct list_head*, int32_t, int32_t);
run_item_ptr_t merge_from_disk(const char*, const char*);


int compare(const void*, const void*);



// #define plus1(origin, cur, offset, type) (origin + ((type)(cur-origin+1)%offset)  )

void* sread(int , void* , size_t, size_t*);














#endif