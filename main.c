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
#include <stdint.h>
#include <time.h>

#define INT_LEN 10
#define STR_BUF 20

void gen_output(int32_t* buf, int64_t len, char* str_buf){

    memset(str_buf, '\0', sizeof(char) << STR_BUF);
    static char i2abuf[100];
    char* str_cur = str_buf;
    for(int64_t i=0;i<len;i++){
        memset(i2abuf, '\0', 100);
        sprintf(i2abuf, "%d\n", buf[i]);
        str_cur = strcat(str_cur, i2abuf);
    }
    return;
}


int main(int argc, char* argv[]){


    // const char* filename = "./data/test4_data.txt";

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    printf("Read from : %s\n", input_file);
    printf("Write to :%s\n", output_file);

    struct list_head head;
    INIT_LIST_HEAD(&head);

    suseconds_t read_start, read_end;
    read_start = time(NULL);
    int64_t t = read_file(input_file, &head);
    read_end = time(NULL);
    run_item_ptr_t item;
    run_item_ptr_t tmp_run;
    long long int i=0;
    int64_t total = 0;
    list_for_each_entry(item, &head, list){
        i++;
        total += item->len;
    }
    printf("\n%lld\n", i);
    printf("read total%ld\n", t);
    printf("total : %d\n", total);

    printf("============\n");
    suseconds_t start, end;
    start = time(NULL);
    struct list_head* final_head = external_sort(&head);
    end = time(NULL);

    // printf("Cost : %d\n", end - start);

    run_item_ptr_t final_run = list_entry(final_head->next, run_item_t, list);

    printf("final head: %s", final_run->pathname);

    char name[30];
    memset(name, '\0', 30);
    sprintf(name, "./tmp/%s", final_run->pathname);
    int ffd = open(name, O_RDWR);

    int64_t len;
    int32_t num;

    FILE* fp = fopen(output_file, "w");
    FILE* ferr = fopen("err.txt", "w");
    read(ffd, &len, sizeof(int64_t));
    suseconds_t write_s, write_e;
    write_s = time(NULL);
    printf("len : %ld\n", len);
    int32_t prev, cur;
    int32_t* num_buf = malloc(sizeof(int32_t) << INT_LEN);
    int32_t quo = len >> INT_LEN;
    printf("%d\n", len);
    int32_t r = len % (1 << INT_LEN);
    printf("quo: %d, r: %d\n", quo, r);
    char* str = malloc(sizeof(char) << STR_BUF);
    for(int64_t i=0;i< quo;i++){
        read(ffd, num_buf, sizeof(int32_t) << INT_LEN);
        gen_output(num_buf, 1 << INT_LEN, str);
        fwrite(str, 1, strlen(str), fp);
    }
    memset(num_buf, 0, sizeof(int32_t) << INT_LEN);
    read(ffd, num_buf, sizeof(int32_t) * r);
    gen_output(num_buf, r, str);
    fwrite(str, 1, strlen(str), fp);
    // for(int64_t i=0;i<len;i++){
    //     num = 0;
    //     read(ffd, num_buf, sizeof(int32_t) << 10);
    //     // printf("%ld\n", num);
    //     fprintf(fp, "%ld\n", num);
    //     // if(i == 0){
    //     //     prev = num;
    //     // }else{
    //     //     if(num < prev){
    //     //         fprintf(ferr, "%ld >= %d\n", prev, num);
    //     //     }else {
    //     //         fprintf(fp, "%ld\n", num);
    //     //     }
    //     //     prev = num;
    //     // }
    // }
    write_e = time(NULL);

    printf("Read time cost: %d\n", read_end - read_start);
    printf("Write time cost: %d\n", write_e - write_s);
    printf("Sorting time cost: %d\n", end - start);

    close(ffd);
    fclose(fp);

    remove(final_run->pathname);
    return 0;


}