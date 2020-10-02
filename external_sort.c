#include "external_sort.h"
#include "list.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int new_runItem(run_item_ptr_t* item){

    run_item_ptr_t run = (run_item_ptr_t) malloc(sizeof(run_item_t));

    run->len = 0;
    run->records = (int*) malloc(sizeof(int) * MAX_RECORDS_SIZE);
    run->in_mem = 0;
    run->pathname = NULL;
    INIT_LIST_HEAD(&run->list);

    *item = run;

    return 0;
}

int read_file(const char* pathname, struct list_head *head) {

    int fd = open(pathname, O_RDWR);
    if(fd < 0){
        printf("open failed");
        exit(0);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        printf("fstat");
        exit(1);
    }

    char* ptr = mmap(NULL, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED){
        printf("mmap failed \n");
        exit(2);
    }

    off_t dx = 0;
    long long int run_i = 0;
    run_item_ptr_t runItem;
    int num;
    char* t_ptr = ptr;
    char name[20];
    while(dx < statbuf.st_size){
        if(run_i == 0){
            new_runItem(&runItem);
            list_add_tail(&runItem->list, head);
        }

        num = parse_int(&t_ptr, &dx, &statbuf);
        printf("%d\n", num);
        (runItem->records)[run_i] = num;
        if(run_i == MAX_RECORDS_INDEX || dx == statbuf.st_size){
            gen_random(&name, 20);
            printf("%s\n", name);
            runItem->len = run_i+1;
            runItem->pathname = str_assign(name);
            store_run(runItem->pathname, runItem);
            printf("stored\n");
        }
        run_i = (run_i == MAX_RECORDS_INDEX || dx == statbuf.st_size) ? 0 : run_i + 1;

    }

    return 0;

}


void gen_random(char* name, const int len){
    static const char alphanum[] =  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    name[0] = '_';
    for (int i = 1; i < len; ++i) {
        name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    name[len] = 0;
}

char* str_assign(const char* src){
    char* dst = (char*) malloc(sizeof(char) * strlen(src));
    memset(dst, '\0', strlen(src));
    strcpy(dst, src);
    return dst;
}


void store_run(const char* filename, run_item_ptr_t item){

    char buf[100];
    memset(buf, '\0', 100);
    sprintf(buf, "./tmp/%s", filename);
    int fd = open(buf, O_RDWR|O_CREAT);

    write(fd, &item->len, sizeof(long long int));
    write(fd, item->records, sizeof(int) * item->len);
    write(fd, &item->in_mem, sizeof(char));
    int pathname_l = strlen(filename);
    write(fd, &pathname_l, sizeof(int));
    write(fd, item->pathname, sizeof(char) * strlen(item->pathname));

}

run_item_ptr_t read_run(const char* filename){
    char buf[100];
    memset(buf, '\0', 100);
    sprintf(buf, "./tmp/%s", filename);

    chmod(buf, S_IRWXU);
    int fd = open(buf, O_RDWR);

    run_item_ptr_t runItem = (run_item_ptr_t)malloc(sizeof(run_item_t));

    long long int len;
    char in_mem;
    int pathname_l = 0;
    char* pathname = NULL;

    read(fd, &len, sizeof(long long int));
    int* records = malloc(sizeof(int) * len);
    read(fd, records, sizeof(int)* len);
    read(fd, &in_mem, sizeof(char));
    read(fd, &pathname_l, sizeof(int));
    pathname = malloc(sizeof(char) * pathname_l);
    read(fd, pathname, sizeof(char)* pathname_l);

    runItem->len = len;
    runItem->records = records;
    runItem->in_mem = in_mem;
    runItem->pathname = pathname;
    INIT_LIST_HEAD(&runItem->list);

    return runItem;

}


int parse_int(char** ptr, off_t* dx, struct stat* statbuf){
    char* t_ptr = *ptr;
    char buf[50];
    memset(buf, '\0', 50);
    char* t_buf = buf;
    while( *t_ptr != '\n' && *dx != statbuf->st_size ){
        *t_buf = *t_ptr;
        t_buf++;
        t_ptr++;
        (*dx)++;
    }
    if(*t_ptr == '\n'){
        (*dx)++;
        *ptr = t_ptr+1;
    }else if(*dx == statbuf->st_size){
        *ptr = t_ptr;
    }

    // printf("dx: %d, statbuf->st_size: %d ", *dx, statbuf->st_size);

    return atoi(buf);
}