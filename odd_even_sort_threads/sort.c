#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>


int *arr;
int n, nlocal, nthreads;

typedef struct {
    pthread_mutex_t count_lock;
    pthread_cond_t ok_to_proceed;
    int count;
} mylib_barrier_t;

mylib_barrier_t *b;
mylib_barrier_t *b2;


void mylib_init_barrier(mylib_barrier_t *b) {
    b -> count = 0;
    pthread_mutex_init(&(b->count_lock), NULL);
    pthread_cond_init(&(b->ok_to_proceed), NULL);
}

void mylib_barrier(mylib_barrier_t *b, int num_threads) {
    pthread_mutex_lock(&(b->count_lock));
    b->count++;
    if (b->count == num_threads) {
        b->count = 0;
        pthread_cond_broadcast(&(b->ok_to_proceed));
    } else {
        while (pthread_cond_wait(&(b->ok_to_proceed), &(b->count_lock)) != 0);
    }
    pthread_mutex_unlock(&(b->count_lock));
}

int comparator(const void *p, const void *q) {
    int l = *(const int *)p;
    int r = *(const int *)q;

    return (l-r);
}

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

struct qsort_args {
    int *array;
    int lo;
    int hi;
    int flag;
};

int *randomArray(int n) {
    int i, r;
    int *array = (int *)calloc(n, sizeof(int));

    srand((unsigned)time(NULL));

    for (i=0; i<n; i++) {
        r = rand() % 1000 + 1;
        array[i] = r;
    }
    
    return array;
}

void CompareSplit(int nlocal, int *elements, int *relements, int *wspace, int keepsmall) {
    int i, j, k;

    for (i=0; i<nlocal; i++) {
        wspace[i] = elements[i];
    }

    if (keepsmall) {
        for (i=j=k=0; k<nlocal; k++) {
            if (j==nlocal || (i<nlocal && wspace[i] < relements[j])) {
                elements[k] = wspace[i++];
            }
            else {
                elements[k] = relements[j++];
            }
        }
    }
    else {
        for (i=k=nlocal-1, j=nlocal-1; k>=0; k--) {
            if (j == -1 || (i>=0 && wspace[i]>=relements[j])) {
                elements[k] = wspace[i--];
            }
            else {
                elements[k] = relements[j--];
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

void copyArray(int *a1, int *a2, int n) {
    int i = 0;
    for (i=0; i<n; i++) {
        a1[i] = a2[i];
    }
} 

void *odd_even(void *_threadid) {
    int threadid = (int)_threadid;
    int *elements;
    int *relements = (int *)calloc(nlocal, sizeof(int));
    int oddrank;
    int evenrank;
    int *wspace = (int *)malloc(nlocal*sizeof(int));
    int i;

    elements = arr + threadid*nlocal;

    qsort(elements, nlocal, sizeof(int), comparator);

    if (threadid%2 == 0) {
        oddrank = threadid - 1;
        evenrank = threadid + 1;
    } else {
        oddrank = threadid + 1;
        evenrank = threadid - 1;
    }
    if (oddrank == -1 || oddrank == nthreads) {
        oddrank = NULL;
    }
    if (evenrank == -1 || evenrank == nthreads) {
        evenrank = NULL;
    }
    mylib_barrier(b, nthreads);
 
    for (i=0; i<nthreads; i++) {
        mylib_barrier(b, nthreads);

        if (i%2 == 1) {
            copyArray(relements, arr + oddrank*nlocal, nlocal);
            if (threadid != 0 && threadid != nthreads-1) {
                mylib_barrier(b2, nthreads - 2);
                CompareSplit(nlocal, elements, relements, wspace, threadid<oddrank);
            } 
        }
        else {
            copyArray(relements, arr+nlocal*evenrank, nlocal);
            mylib_barrier(b, nthreads);
            CompareSplit(nlocal, elements, relements, wspace, threadid<evenrank);
        }
        mylib_barrier(b, nthreads);

    }
}

int main(int argc, char *argv[]) {

    if (argc > 1 && argc <=3) {
        n = atoi(argv[1]);
        nthreads = atoi(argv[2]);
    } else {
        n = 16;
        nthreads = 2;
    }

    pthread_t threads[nthreads];
    int rc, t, i, special_case;
    float timeuse;
	struct timeval tpstart, tpend;

    nlocal = n/nthreads;
    special_case = nlocal + (n%nthreads);

    arr = randomArray(n);
    if (n<=40) {
        printf("Array before: ");
        printArray(arr, n);
    }

    b = malloc(sizeof(mylib_barrier_t));
    b2 = malloc(sizeof(mylib_barrier_t));
    mylib_init_barrier(b);
    mylib_init_barrier(b2);

    gettimeofday(&tpstart, NULL);

    for (t=0; t<nthreads; t++) {
        rc = pthread_create(&threads[t], NULL, odd_even, (void *)t);
        if (rc) {
            printf("ERROR: return error from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (t=0; t<nthreads; t++) {
        rc = pthread_join(threads[t], NULL);
        if (rc) {
            printf("ERROR: return error from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    gettimeofday(&tpend, NULL);
    timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);
	printf("timeuse: %f\n", timeuse);

    if (n <= 40) {
        printf("Array after: ");
        for (i=0; i<n; i++) {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    free(arr);

    return 0;
}