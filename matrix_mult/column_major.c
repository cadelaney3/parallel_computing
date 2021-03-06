#include "sys/time.h"
#include "stdlib.h"
#include "stdio.h"

int n = 10000;
float **array;
float *vec;

void main() {
	// initialize variables
        struct timeval tpstart, tpend;
        float timeuse;
        int i,j;

	// dynamically allocate array and vec
        array = (float **)calloc(n, sizeof(float *));
        for (i=0; i<n; i++) {
                array[i] = (float *)calloc(n, sizeof(float));
        }
        vec = (float *)calloc(n, sizeof(float));

        // initialization values for the matrix
        for (i=0; i<n; i++) {
		for (j=0; j<n; j++) {
			array[i][j] = (float)j;
		}
	}

        gettimeofday(&tpstart, NULL);
        
        // computate the sum of the rows 
        for (i=0; i<n; i++) {
		vec[i] = 0.0;
		for (j=0; j<n; j++) {
			vec[i] += array[i][j];
		}
	}

        gettimeofday(&tpend, NULL);

        timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);

	// print out the time of computation
        printf("timeuse: %f\n", timeuse);
	
	// print out the first ten indices to check if computation
	// was correct
	for (i=0; i<10; i++) {
		printf("row %i = %f\n", i, vec[i]);
	}

        // free up memory
        free(vec);
        for(i=0; i<n; i++) {
        	free(array[i]);
	}
	free(array);
}
