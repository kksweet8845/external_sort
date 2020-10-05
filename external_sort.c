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
#include <time.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

static const char* final_res = "final_result.txt";
static FILE* final_fp;

int new_runItem(run_item_ptr_t* item){

    run_item_ptr_t run = (run_item_ptr_t) malloc(sizeof(run_item_t));

    run->len = 0;
    run->records = (int32_t*) malloc(sizeof(int32_t) * MAX_RECORDS_SIZE);
    run->in_mem = 0;
    run->pathname = NULL;
    INIT_LIST_HEAD(&run->list);

    *item = run;

    return 0;
}

int read_file(const char* pathname, struct list_head *head) {

    int fd = open(pathname, O_RDWR);
    if(fd < 0){
        printf("open failed, errno : %d", errno);
        exit(0);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        printf("fstat");
        exit(1);
    }

    int32_t offt_times = (statbuf.st_size >> 30) + 1;
    // printf("%lld\n", statbuf.st_size >> 30);
    char* num_head = NULL;
    int64_t g_total = 0;
    for(int64_t i=0;i<offt_times;i++){
        printf("%lld\n", i);
        char* ptr = mmap(NULL, GB, PROT_READ|PROT_WRITE, MAP_SHARED, fd, i << 30);
        if(ptr == MAP_FAILED){
            printf("mmap failed \n");
            exit(2);
        }
        off_t buf_size = GB;
        if(i == offt_times - 1 || GB > statbuf.st_size){
            buf_size = (statbuf.st_size & 0x000000003fffffff);
        }

        off_t dx = 0;
        int64_t run_i = 0;
        run_item_ptr_t runItem;
        int32_t num;
        char* t_ptr = ptr;
        char name[20];
        while(dx < buf_size){
            if(run_i == 0){
                new_runItem(&runItem);
                list_add_tail(&runItem->list, head);
            }
            num = parse_int(&t_ptr, &dx, buf_size, &num_head);
            // printf("%d\n", num);
            if(num_head == NULL){
                (runItem->records)[run_i] = num;
            }
            if((run_i == MAX_RECORDS_INDEX || dx == buf_size)){
                gen_random(&name, 20);
                // printf("%s\n", name);
                runItem->len = num_head == NULL ? run_i + 1 : run_i;
                g_total += runItem->len;
                runItem->pathname = str_assign(name);
                time_t start, end;
                start = time(NULL);
                qsort((void*) &(runItem->records[0]), runItem->len, sizeof(int32_t), compare);
                end = time(NULL);
                printf("Cost %d sec\n", end - start);
                store_run(runItem->pathname, runItem);
            }
            run_i = (run_i == MAX_RECORDS_INDEX || dx == buf_size) ? 0 : (num_head == NULL) ? run_i + 1 : run_i;

        }

        int err = munmap(ptr, buf_size);
        if(err != 0){
            printf("munmap is not suess\n");
        }
    }
    close(fd);
    return g_total;

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
    int fd = open(buf, O_RDWR|O_CREAT, S_IRWXU);
    if(fd < 0){
        printf("error : %d\n", errno);
    }
    item->in_mem = 0;
    int32_t len = item->len;
    write(fd, &item->len, sizeof(int64_t));
    int err = write(fd, item->records, sizeof(int32_t)*item->len);
    if(err == -1){
        printf("err occurred %d\n", errno);
    }
    // printf("%d\n", item->records[0]);
    // write(fd, &item->in_mem, sizeof(char));
    // int pathname_l = strlen(filename);
    // write(fd, &pathname_l, sizeof(int));
    // write(fd, item->pathname, sizeof(char) * strlen(item->pathname));
    close(fd);
}

run_item_ptr_t read_run(const char* filename){
    char buf[100];
    memset(buf, '\0', 100);
    sprintf(buf, "./tmp/%s", filename);

    chmod(buf, S_IRWXU);
    int fd = open(buf, O_RDWR, S_IRWXU);

    run_item_ptr_t runItem = (run_item_ptr_t)malloc(sizeof(run_item_t));

    int64_t len;

    read(fd, &len, sizeof(int64_t));
    int32_t* records = malloc(sizeof(int32_t) * len);
    read(fd, records, sizeof(int32_t)* len);

    runItem->len = len;
    runItem->records = records;
    INIT_LIST_HEAD(&runItem->list);

    close(fd);
    return runItem;

}


int parse_int(char** ptr, off_t* dx, off_t st_size, char** suspend_str){
    char* t_ptr = *ptr;
    char buf[50];
    memset(buf, '\0', 50);
    char* t_buf = buf;
    char* sus_str = *suspend_str;
    int flag = 0;
    if(*suspend_str != NULL){
        while(*sus_str != '\0')
            *t_buf++ = *sus_str++;
        // free(suspend_str);
        suspend_str == NULL;
        printf("truncating\n");
        flag = 1;
    }
    while( *dx != st_size && *t_ptr != '\n' ){
        *t_buf = *t_ptr;
        t_buf++;
        t_ptr++;
        (*dx)++;
    }
    if(*dx != st_size && *t_ptr == '\n'){
        (*dx)++;
        *ptr = t_ptr+1;
        *suspend_str = NULL;
        if(flag == 1){
            printf("restore %s\n", buf);
        }
    }else if(*dx == st_size){
        *ptr = strdup(buf);
        *suspend_str = strdup(buf);
        printf("prefix %s\n", buf);
    }

    char** end_p;
    long ret = strtol(buf, end_p, 10);
    return ret;
    
}


struct list_head* external_sort(struct list_head* head){

    run_item_ptr_t item;
    int no_run = 0;
    list_for_each_entry(item, head, list){
        no_run++;
    }
    int32_t quo = no_run >> 1;
    int32_t r = no_run % 2;
    run_item_ptr_t tmp_run;
    int32_t index =0;
    struct list_head* final_head = external_merge(head, quo, r);
    return final_head;
}

int compare(const void* arg1, const void* arg2){
    int32_t a = *((int32_t*)arg1);
    int32_t b = *((int32_t*)arg2);
    return (a > b) - (a < b);
}

struct list_head* external_merge(struct list_head* head, int32_t no_run, int32_t r){

    struct list_head* merged_head = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(merged_head);
    struct list_head *cur;
    cur = head->next;
    printf("no_run : %d\n", no_run);
    for(int i=0;i<no_run && no_run >= 1;i++){
        run_item_ptr_t m1 = list_entry(cur, run_item_t, list);
        run_item_ptr_t m2 = list_entry(cur->next, run_item_t, list);

        run_item_ptr_t m3 = merge_from_disk(m1->pathname, m2->pathname);

        list_add_tail(&m3->list, merged_head);
        cur = cur->next->next;
    }
    printf("finish merge\n");
    if(r == 1)
        list_add_tail(cur, merged_head);

    int32_t total = no_run + r;
    int32_t new_no_run = total >> 1;
    int32_t new_r = total % 2;
    if( total >= 2){
        // printf("total: %d\n", total);
        printf("total: %ld\n", total);
        return external_merge(merged_head,  new_no_run, new_r);
    }else{
        return merged_head;
    }

}


run_item_ptr_t merge_from_disk(const char* file1, const char* file2){


    char path1[30], path2[30], path3[30];
    memset(path1, '\0', 30);
    memset(path2, '\0', 30);
    memset(path3, '\0', 30);

    sprintf(path1, "./tmp/%s", file1);
    sprintf(path2, "./tmp/%s", file2);

    int err = chmod(path1, S_IRWXU);
    if(err == -1){
        printf("merge from disk: %d\n", errno);
    }
    err = chmod(path2, S_IRWXU);
    if(err == -1){
        printf("merge from disk: %d\n", errno);
    }
    int fd1 = open(path1, O_RDWR);
    if(fd1 < 0){
        printf("fd1 errno: %d\n", errno);
    }
    int fd2 = open(path2, O_RDWR);
    if(fd2 < 0){
        printf("fd2 errno: %d\n", errno);
    }

    char name[20];
    gen_random(name, 20);
    sprintf(path3, "./tmp/%s", name);

    struct stat statbuf1, statbuf2;
    int err1 = fstat(fd1, &statbuf1);
    if(err1 == -1){
        printf("fstat fd1 err1 : %d\n", errno);
    }
    int err2 = fstat(fd2, &statbuf2);
    if(err2 == -1){
        printf("fstat fd2 err2 : %d\n", errno);
    }

    int fd3 = open(path3, O_RDWR|O_CREAT, S_IRWXU);
    if(fd3 < 0){
        printf("fd3 open error : %d\n",errno);
    }
    int64_t len1, len2;
    err = read(fd1, &len1, sizeof(int64_t));
    if(err == -1){
        printf("read1 errno: %d\n", errno);
    }
    err = read(fd2, &len2, sizeof(int64_t));
    if(err == -1){
        printf("read2 errno: %d\n", errno);
    }

    int64_t larger = (len1 >= len2) ? len1 : len2;
    int64_t smaller = (len1 >= len2) ? len2 : len1;

    int64_t total = len1 + len2;

    int32_t *buf1, *buf2, *bufo;

    int32_t _32_KB = sizeof(int32_t) << 23;
    int32_t length = 1 << 23;

    buf1 = malloc(_32_KB);
    if(buf1 == NULL){
        printf("buf1 alloc error\n");
    }
    buf2 = malloc(_32_KB);
    if(buf2 == NULL){
        printf("buf2 alloc error\n");
    }
    bufo = malloc(_32_KB);
    if(bufo == NULL){
        printf("bufo alloc error\n");
    }

    int32_t *b1_p, *b2_p, *bo_p, *b1_end, *b2_end, *bo_end;
    b1_p = buf1;
    b2_p = buf2;
    bo_p = bufo;


    err = write(fd3, &total, sizeof(int64_t));
    if(err == -1){
        printf("write 0 error: %d\n", errno);
    }

    // buffer read
    int32_t file1_r, file2_r;
    file1_r = statbuf1.st_size;
    file2_r = statbuf2.st_size;

    int32_t* b1_re, *b2_re;

    b1_re = sread(fd1, buf1, _32_KB, &file1_r);
    b2_re = sread(fd2, buf2, _32_KB, &file2_r);

    // printf("herr\n");
    b1_end = buf1 + (length);
    b2_end = buf2 + (length);
    bo_end = bufo + (length);

    memset(bufo, 0, _32_KB);
    while(1){
        if(bo_p == bo_end){
            write(fd3, bufo, _32_KB);
            memset(bufo, 0, _32_KB);
            bo_p = bufo;
        }
        if(b1_p == b1_re){
            b1_re = sread(fd1, buf1, _32_KB, &file1_r);
            b1_p = buf1;
        }
        if(b2_p == b2_re){
            b2_re = sread(fd2, buf2, _32_KB, &file2_r);
            b2_p = buf2;
        }

        if(*b1_p < *b2_p){
            *bo_p++ = *b1_p++;
            --len1;
        }else {
            *bo_p++ = *b2_p++;
            --len2;
        }
        if(len1 == 0 || len2 == 0)
            break;
    }

    if(len1 != 0){
        for(int64_t i=0;i<len1;i++){
            if(bo_p == bo_end){
                write(fd3, bufo, _32_KB);
                bo_p = bufo;
            }
            if(b1_p == b1_re){
                b1_re = sread(fd1, buf1, _32_KB, &file1_r);
                b1_p = buf1;
            }
            *bo_p++ = *b1_p++;
        }
    }else{
        for(int64_t i=0;i<len2;i++){
            if(bo_p == bo_end){
                write(fd3, bufo, _32_KB);
                bo_p = bufo;
            }
            if(b2_p == b2_end){
                b2_re = sread(fd2, buf2, _32_KB, &file2_r);
                b2_p = buf2;
            }
            *bo_p++ = *b2_p++;
        }
    }
    write(fd3, bufo, (bo_p - bufo)<<2);
    run_item_ptr_t item = malloc(sizeof(run_item_t));

    item->len = total;
    item->pathname = str_assign(name);
    close(fd1);
    close(fd2);
    close(fd3);
    free(buf1);
    free(buf2);
    free(bufo);

    remove(path1);
    remove(path2);

    return item;
}

void* sread(int fd, void* ptr, size_t size, size_t *rsize){

    int err;
    if(*rsize <= size){
        err = read(fd, ptr, *rsize);
        size_t tmp = *rsize;
        *rsize = 0;
        return ptr + tmp;
    }else{
        read(fd, ptr, size);
        *rsize -= size;
        return ptr + size;
    }

}

