#include <stdlib.h>
#include <stdio.h>



int main(void){


    const char* filename = "data.txt";


    long long int ONE_GB_b = 1 << 30;

    long long int cur_size = 0;

    FILE* fp = fopen(filename, "w");

    while( cur_size < ONE_GB_b>>1 ){

        cur_size += sizeof(long long int);

        fprintf(fp, "%ld\n", random());

        if(cur_size >> 10 % 500 == 0){
            printf("%lldMB\n", cur_size >> 10);
        }
    }

    fclose(fp);

    return 0;
}




