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

int main(void){


    const char* filename = "./data/test_data.txt";

    struct list_head head;
    INIT_LIST_HEAD(&head);
    read_file(filename, &head);
    run_item_ptr_t item;
    run_item_ptr_t tmp_run;
    long long int i=0;
    list_for_each_entry(item, &head, list){

        // printf("%s\n", item->pathname);
        // printf("%lld\n", item->len);


        // printf("test=======\n");

        // tmp_run = read_run(item->pathname);
        i++;

        // printf("%s\n", tmp_run->pathname);
        // printf("%d\n", tmp_run->len);
        // for(int i=0;i<tmp_run->len;i++){
        //     printf("%d\n", tmp_run->records[i]);
        // }
    }
    printf("\n%lld\n", i);

    printf("============\n");
    struct list_head* final_head = external_sort(&head);

    run_item_ptr_t final_run = list_entry(final_head->next, run_item_t, list);

    printf("final head: %s", final_run->pathname);

    char name[30];
    memset(name, '\0', 30);
    sprintf(name, "./tmp/%s", final_run->pathname);
    int ffd = open(name, O_RDONLY);

    int64_t len;
    int32_t num;

    FILE* fp = fopen("final_result", "w");
    FILE* ferr = fopen("err.txt", "w");
    read(ffd, &len, sizeof(int64_t));
    printf("len : %lld\n", len);
    for(int i=0;i<len;i++){
        read(ffd, &num, sizeof(int32_t));
        // printf("%lld\n", num);
        // if(num != i){
        //     fprintf(ferr, "%ld != %d\n", num, i);
        // }
        fprintf(fp, "%ld\n", num);
    }

    close(ffd);
    fclose(fp);

    return 0;


}