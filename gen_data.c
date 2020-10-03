#include <stdlib.h>
#include <stdio.h>

void shuffle(long long int *array, long long int n)
{
    if (n > 1)
    {
        long long int i;
        for (i = 0; i < n - 1; i++)
        {
          size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
          int t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}


int main(void){


    const char* filename = "large_data.txt";


    long long int ONE_GB_b = 1 << 30;

    long long int cur_size = 0;

    FILE* fp = fopen(filename, "w");

    // while( cur_size < ONE_GB_b>>1 ){

    //     cur_size += sizeof(long long int);

    //     fprintf(fp, "%ld\n", random());

    //     if(cur_size >> 10 % 500 == 0){
    //         printf("%lldMB\n", cur_size >> 10);
    //     }
    // }
    long long int len = 1<<20 + 7;
    long long int* arr = malloc(sizeof(long long int) * len);
    if(arr == NULL){
        printf("Error\n");
    }
    for(long long int i=0;i<len;i++){
        arr[i] = i;
    }
    shuffle(&arr[0], len);
    for(long long int i=0;i<len;i++){
        fprintf(fp, "%ld\n", random());
    }

    fclose(fp);

    return 0;
}




