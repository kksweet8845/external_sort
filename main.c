#include <stdio.h>
#include <stdlib.h>
#include "external_sort.h"
#include "list.h"


int main(void){


    const char* filename = "data.txt";

    struct list_head head;
    read_file(filename, &head);

    run_item_ptr_t item;
    run_item_ptr_t tmp_run;
    list_for_each_entry(item, &head, list){

        printf("%s\n", item->pathname);
        printf("%s\n", item->len);


        printf("test=======\n");

        tmp_run = read_run(item->pathname);

        printf("%s\n", tmp_run->pathname);
        printf("%d\n", tmp_run->len);
    }

    return 0;


}