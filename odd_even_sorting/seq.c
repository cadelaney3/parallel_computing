#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void quick_sort(int *array, int lo, int hi, int flag) {
    int i = lo - 1;
    int j = lo;
    int pivot = array[hi];
    int temp;
    //printf("got here");
    
    if (lo <= hi) {
        for (j=lo; j<=hi; j++) {
            if (array[j] <= pivot) {
                i++;
                temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
        array[j] = array[i];
        array[i] = pivot;
        array[hi] = temp;

        quick_sort(array, lo, i-1, flag);
        quick_sort(array, i+1, hi, flag);
    }
}

void bubble_sort(int *array, int n) {
    int i, j, temp;
    for (i=0; i<n-1; i++) {
        for (j=0; j<n-i-1; j++) {
            if (array[j] > array[j+1]) {
                temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
            }
        }
    }
}

void printArray(int *array, int n) {
    int i = 0;
    for (i=0; i<n; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int* randomArray(int n) {
    int i, r;
    int *array = (int *)calloc(n, sizeof(int));

    srand((unsigned)time(NULL));

    for (i=0; i<n; i++) {
        r = rand() % 1000 + 1;
        array[i] = r;
    }
    
    return array;
}

int main(int argc, char *argv[]) {
    int n;
    if (argc >= 2) {
        n = atoi(argv[1]);
    }
    else {
        n = 40;
    }
    
    struct timeval tpstart, tpend;
    float timeuse;
    int i, j;

    //int n = 20;
    int flag = 1;
    int *arr = randomArray(n);

    gettimeofday(&tpstart, NULL);
   // quick_sort(arr, 0, n-1, flag);
    bubble_sort(arr, n);
    gettimeofday(&tpend, NULL);

    timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) +
        (tpend.tv_usec - tpstart.tv_usec);

        printf("timeuse: %f\n", timeuse);

    if (n <= 16) {
        for (i=0; i<n; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    return 0;
}