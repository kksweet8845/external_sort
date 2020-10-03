// #include "external_sort.h"
// #include "list.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


int main(void){


    char* data = "large_data.txt";

    int fd = open(data, O_RDWR);

    struct stat statbuf;
    int err = fstat(fd, &statbuf);

    off_t r = (statbuf.st_size & 0x000000003fffffff);
    printf("%60lld\n%60lld\n", statbuf.st_size, r);


}