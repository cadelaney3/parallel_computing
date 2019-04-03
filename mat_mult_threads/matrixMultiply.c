#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>


double *matA;
double *matB;
double *matC;

struct thread_args {
    int start_index;
    int end_index;
    int n;
    int nlocal;
};

void *matMult(void *_args) {
    struct thread_args *args = (struct thread_args *) _args;
    int i, j, k;
    for (i=args->start_index; i<args->end_index; i++) {
        for (j=0; j<(args->n); j++) {
            for (k=0; k<(args->n); k++) {
                matC[(i*(args->n)+j)] += matA[i*(args->n)+k]*matB[k*(args->n)+j];
            }
        }
    }
    pthread_exit( NULL );
}

int main(int argc, char *argv[]) {
    int n, nthreads;
    if (argc > 1 && argc <=3) {
        n = atoi(argv[1]);
        nthreads = atoi(argv[2]);
    } else {
        n = 16;
        nthreads = 2;
    }

    pthread_t threads[nthreads];
    int rc, t, i, nlocal, special_case;
    float timeuse;
	struct timeval tpstart, tpend;

    nlocal = n/nthreads;
    special_case = nlocal + (n%nthreads);

    matA = (double *)calloc(n*n, sizeof(double));
    matB = (double *)calloc(n*n, sizeof(double));
    matC = (double *)calloc(n*n, sizeof(double));

    for (i=0; i<n*n; i++) {
        matA[i] = i;
        matB[i] = 1.0;
        matC[i] = 0;
    }

    gettimeofday(&tpstart, NULL);

    for (t=0; t<nthreads; t++) {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->n = n;
        args->nlocal = nlocal;
        args->start_index = t * nlocal;
        if (t == nthreads-1 && nlocal != special_case) {
            args->end_index = (t+1) * nlocal + (special_case - nlocal);
        } else {
            args->end_index = (t+1) * nlocal;
        }
        rc = pthread_create(&threads[t], NULL, matMult, args);
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
        for (i=0; i<n*n; i++) {
            if (i%n == 0) {
                printf("\n");
            }
            printf("%.2f ", matC[i]);
        }
        printf("\n");
    }

    free(matA);
    free(matB);
    free(matC);

    return 0;
}