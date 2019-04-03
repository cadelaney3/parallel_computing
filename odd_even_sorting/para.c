#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <sys/time.h>

int comparator(const void *p, const void *q) {
    int l = *(const int *)p;
    int r = *(const int *)q;

    return (l-r);
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


int main(int argc, char *argv[]) {
    int n;
    if (argc >= 2) {
            n = atoi(argv[1]);
    }
    else {
            n = 40;
    }
    //printf("n = %d\n", n);

    int npes;
    int myrank;
    int nlocal;
    int *array;
    int *elements;
    int *relements;
    int oddrank;
    int evenrank;
    int *wspace;
    int i;
    MPI_Status status;
    double t1, t2;

    float timeuse;
    struct timeval tpstart, tpend;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &npes);

    nlocal = n/npes;

    array = (int *)malloc(n*sizeof(int));
    elements = (int *)malloc(nlocal*sizeof(int));
    relements = (int *)malloc(nlocal*sizeof(int));
    wspace = (int *)malloc(nlocal*sizeof(int));

    srandom(myrank);
    if (myrank == 0) {
        for (i=0; i<n; i++) {
            array[i] = random()%(n*10);
	   // printf("%d ", array[i]);
	}
        //printf("\n");
    }

    if (myrank == 0) {
        t1 = MPI_Wtime();
    }

    MPI_Scatter(array, nlocal, MPI_INT, elements, nlocal, MPI_INT, 0, MPI_COMM_WORLD);

    qsort(elements, nlocal, sizeof(int), comparator);
    //bubble_sort(elements, nlocal);

    if (myrank%2 == 0) {
        oddrank = myrank - 1;
        evenrank = myrank + 1;
    }
    else {
        oddrank = myrank + 1;
        evenrank = myrank - 1;
    }

    if (oddrank == -1 || oddrank == npes) {
        oddrank = MPI_PROC_NULL;
    }
    if (evenrank == -1 || evenrank == npes) {
        evenrank = MPI_PROC_NULL;
    }

    for (i=0; i<npes; i++) {
        if (i%2 == 1) {
            MPI_Sendrecv(elements, nlocal, MPI_INT, oddrank, 1, relements, nlocal, MPI_INT, oddrank,
                         1, MPI_COMM_WORLD, &status);
		if (myrank!=0 && myrank!=npes-1) {
			CompareSplit(nlocal, elements, relements, wspace, myrank<status.MPI_SOURCE);
		}
        }
        else {
            MPI_Sendrecv(elements, nlocal, MPI_INT, evenrank, 1, relements, nlocal, MPI_INT, evenrank,
                         1, MPI_COMM_WORLD, &status);
        	CompareSplit(nlocal, elements, relements, wspace, myrank<status.MPI_SOURCE);
        }
    }

    int *sorted_array = NULL;
    if (myrank == 0) {
        sorted_array = malloc(n*sizeof(int));
    }
    MPI_Gather(elements, nlocal, MPI_INT, sorted_array, nlocal, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (myrank == 0) {
        t2 = MPI_Wtime();
        printf("Elapsed time: %f\n", t2-t1);
    }

    if (myrank == 0 && n <= 40) {
        printf("after sorting: ");
        for (i=0; i<n; i++) {
            printf("%d ", sorted_array[i]);
        }
        printf("\n");
    }

    free(elements);
    free(relements);
    free(wspace);
    free(array);
    free(sorted_array);

    MPI_Finalize();

    return 0;
}
