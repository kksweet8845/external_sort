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

int new_runItem(run_item_ptr_t* item){

    run_item_ptr_t run = (run_item_ptr_t) malloc(sizeof(run_item_t));

    run->len = 0;
    run->records = (int64_t*) malloc(sizeof(int64_t) * MAX_RECORDS_SIZE);
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

    int64_t offt_times = (statbuf.st_size >> 30) + 1;
    // printf("%lld\n", statbuf.st_size >> 30);
    for(int64_t i=0;i<offt_times;i++){
        printf("%lld\n", i);
        char* ptr = mmap(NULL, GB, PROT_READ|PROT_WRITE, MAP_SHARED, fd, i << 30);
        if(ptr == MAP_FAILED){
            printf("mmap failed \n");
            exit(2);
        }
        off_t buf_size = GB;
        if(i == offt_times - 1){
            buf_size = (statbuf.st_size & 0x000000003fffffff);
        }

        off_t dx = 0;
        int64_t run_i = 0;
        run_item_ptr_t runItem;
        int64_t num;
        char* t_ptr = ptr;
        char name[20];
        while(dx < buf_size){
            if(run_i == 0){
                new_runItem(&runItem);
                list_add_tail(&runItem->list, head);
            }

            num = parse_int(&t_ptr, &dx, buf_size);
            // printf("%d\n", num);
            (runItem->records)[run_i] = num;
            if(run_i == MAX_RECORDS_INDEX || dx == buf_size){
                gen_random(&name, 20);
                // printf("%s\n", name);
                runItem->len = run_i+1;
                runItem->pathname = str_assign(name);
                store_run(runItem->pathname, runItem);
                // free(runItem->records);
                // printf("stored, dx: %lld, buf_size: %lld\n", dx, buf_size);
            }
            run_i = (run_i == MAX_RECORDS_INDEX || dx == buf_size) ? 0 : run_i + 1;

        }

        int err = munmap(ptr, buf_size);
        if(err != 0){
            printf("munmap is not suess\n");
        }
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

    item->in_mem = 0;

    write(fd, &item->len, sizeof(int64_t));
    write(fd, item->records, sizeof(int64_t) * item->len);
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

    int64_t len;
    char in_mem;
    int pathname_l = 0;
    char* pathname = NULL;

    read(fd, &len, sizeof(int64_t));
    int64_t* records = malloc(sizeof(int64_t) * len);
    read(fd, records, sizeof(int)* len);
    read(fd, &in_mem, sizeof(char));
    read(fd, &pathname_l, sizeof(int));
    pathname = malloc(sizeof(char) * pathname_l);
    read(fd, pathname, sizeof(char)* pathname_l);

    runItem->len = len;
    runItem->records = records;
    runItem->in_mem = 1;
    runItem->pathname = pathname;
    INIT_LIST_HEAD(&runItem->list);

    close(fd);
    return runItem;

}


int parse_int(char** ptr, off_t* dx, off_t st_size){
    char* t_ptr = *ptr;
    char buf[50];
    memset(buf, '\0', 50);
    char* t_buf = buf;
    while( *dx != st_size && *t_ptr != '\n' ){
        *t_buf = *t_ptr;
        t_buf++;
        t_ptr++;
        (*dx)++;
    }
    if(*dx != st_size && *t_ptr == '\n'){
        (*dx)++;
        *ptr = t_ptr+1;
    }else if(*dx == st_size){
        *ptr = t_ptr;
    }

    // printf("dx: %d, statbuf->st_size: %d ", *dx, statbuf->st_size);

    return atoi(buf);
}


struct list_head* external_sort(struct list_head* head){

    run_item_ptr_t item;
    int no_run = 0;
    list_for_each_entry(item, head, list){
        no_run++;
    }
    // printf("no_run %d\n", no_run);

    int64_t quo = no_run >> 1;
    int64_t r = no_run % 2;
    run_item_ptr_t tmp_run;
    list_for_each_entry(item, head, list){
        if(item->in_mem == 1){
            tmp_run = read_run(item->pathname);
            item->records = tmp_run->records;
            free(tmp_run);
        }
        time_t start, end;
        start = time(NULL);
        printf("Start qsort...\n");
        qsort((void*) &(item->records[0]), item->len, sizeof(int64_t), compare);
        end = time(NULL);
        printf("Cost %d sec\n", end - start);
        store_run(item->pathname, item);
        // for(int i=0;i<item->len;i++){
        //     printf("%d ", item->records[i]);
        // }
        // printf("\n");
    }


    struct list_head* final_head = external_merge(head, quo, r);
    return final_head;
}

int compare(const void* arg1, const void* arg2){
    return (*(int*)arg1 - *(int*)arg2);
}


struct list_head* external_merge(struct list_head* head, int64_t no_run, int64_t r){

    struct list_head* merged_head = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(merged_head);
    struct list_head *cur;
    cur = head->next;
    for(int i=0;i<no_run && no_run >= 1;i++){
        run_item_ptr_t m1 = list_entry(cur, run_item_t, list);
        run_item_ptr_t m2 = list_entry(cur->next, run_item_t, list);

        run_item_ptr_t m3 = merge_from_disk(m1->pathname, m2->pathname);

        list_add_tail(&m3->list, merged_head);
        cur = cur->next->next;
    }
    if(r == 1)
        list_add_tail(cur, merged_head);

    int64_t total = no_run + r;
    int64_t new_no_run = total >> 1;
    int64_t new_r = total % 2;
    if( total >= 2){
        // printf("total: %d\n", total);
        printf("total: %lld\n", total);
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

    chmod(path1, S_IRWXU);
    chmod(path2, S_IRWXU);
    int fd1 = open(path1, O_RDWR);
    int fd2 = open(path2, O_RDWR);

    char name[20];
    gen_random(name, 20);
    sprintf(path3, "./tmp/%s", name);

    struct stat statbuf1, statbuf2;
    int err1 = fstat(fd1, &statbuf1);
    int err2 = fstat(fd2, &statbuf2);

    int64_t* ptr1 = mmap(NULL, GB, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
    int64_t* ptr2 = mmap(NULL, GB, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);

    chmod(path3, S_IRWXU);
    int fd3 = open(path3, O_RDWR|O_CREAT);

    int64_t len1, len2;
    len1 = *ptr1;
    len2 = *ptr2;
    ptr1++;
    ptr2++;
    // read(fd1, &len1, sizeof(int64_t));
    // read(fd2, &len2, sizeof(int64_t));

    int64_t larger = (len1 >= len2) ? len1 : len2;
    int64_t smaller = (len1 >= len2) ? len2 : len1;

    int64_t total = len1 + len2;
    // printf("len1: %lld, len2: %lld, total:%lld\n", len1, len2, total);
    // printf("total: %lld\n", total);
    write(fd3, &total, sizeof(int64_t));

    // int v1, v2;
    // read(fd1, &v1, sizeof(int));
    // read(fd2, &v2, sizeof(int));


    int64_t *buffer = malloc(sizeof(int64_t) * 1024);
    memset(buffer, 0, sizeof(int64_t)*1024);
    int b_i = 0;
    while(1){
        if(b_i == 1024){
            write(fd3, buffer, sizeof(int64_t) * 1024);
            memset(buffer, 0, sizeof(int64_t) * 1024);
            b_i = 0;
        }
        if(*ptr1 < *ptr2){
            // write(fd3, ptr1, sizeof(int64_t));
            // printf("%lld\n", v1);
            buffer[b_i++] = *ptr1;
            --len1;
            if(len1 >= 1){
                ptr1++;
            }else if(len1 == 0){
                break;
            }
        }else {
            // write(fd3, ptr2, sizeof(int64_t));
            // printf("%lld\n", v2);
            buffer[b_i++] = *ptr2;
            --len2;
            if(len2 >= 1){
                // read(fd2, &v2, sizeof(int));
                ptr2++;
            }else if(len2 == 0){
                break;
            }
        }
    }
    if(b_i != 0){
        write(fd3, buffer, sizeof(int64_t) * (b_i+1));
        b_i = 0;
    }

    if(len1 != 0){
        for(int i=0;i<len1;i++){
            buffer[b_i++] = *ptr1;
            // write(fd3, ptr1, sizeof(int64_t));
            // printf("%lld\n", v1);
            // read(fd1, &v1, sizeof(int));
            ptr1++;
            if(b_i == 1024){
                write(fd3, buffer, sizeof(int64_t)*1024);
                b_i=0;
            }
        }
        if(b_i != 0){
            write(fd3, buffer, sizeof(int64_t)*(b_i+1));
            b_i=0;
        }
    }else{
        for(int i=0;i<len2;i++){
            buffer[b_i++] = *ptr2;
            // write(fd3, ptr2, sizeof(int64_t));
            // printf("%lld\n", v2);
            // read(fd2, &v2, sizeof(int));
            ptr2++;
            if(b_i == 1024){
                write(fd3, buffer, sizeof(int64_t)*1024);
                b_i=0;
            }
        }
        if(b_i != 0){
            write(fd3, buffer, sizeof(int64_t)*(b_i+1));
            b_i=0;
        }
    }

    run_item_ptr_t item = malloc(sizeof(run_item_t));

    item->len = total;
    item->pathname = str_assign(name);
    close(fd1);
    close(fd2);
    close(fd3);
    return item;
}


