#ifndef EXTERNAL_SORT_H
#define EXTERNAL_SORT_H

#include "list.h"
#include <stdio.h>
#include <sys/stat.h>


#define MAX_RECORDS_SIZE (1<<3)
#define MAX_RECORDS_INDEX (MAX_RECORDS_SIZE-1)

struct run_ele {
    int* records;
    struct list_head list;
    long long int len;
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

int parse_int(char** ptr, off_t* dx, struct stat* statbuf);
















#endif