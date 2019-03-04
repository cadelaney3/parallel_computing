#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <sys/time.h>

//int n = 1024;

int main(int argc, char *argv[]) {
	int n;
	if (argc >= 2) {
		n = atoi(argv[1]);
	}
	else {
		n = 512;
	}
	printf("n = %d\n", n);
	
	int namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int rank, size;
	int i, j, k;
	float timeuse;
	struct timeval tpstart, tpend;
//	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Get_processor_name(processor_name, &namelen);
	//printf("Hello world! I am %d of %d on %s\n", rank, size, processor_name);
	
	//	printf("%f ", matA[i]);
	if (rank == 0) {
	
		double *matA = (double *)calloc(n*n, sizeof(double));
		double *matB = (double *)calloc(n*n, sizeof(double));
		double *matC = (double *)calloc(n*n, sizeof(double));
		MPI_Status status;

		int block_size = n/(size - 1);
		int special_case = (n/(size-1)) + n%(size-1);
		for (i=0; i<n*n; i++) {
			matA[i] = 2.0;
			matB[i] = 2.0;
		}
		int t = 0;
		int c = 0;
		gettimeofday(&tpstart, NULL);

		for (i=1; i<size; i++) {
			MPI_Send(matB, n*n, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
			double *matA_block = (double *)calloc(n*(n/(size-1)), sizeof(double));
			if (t < n*(n-special_case)+1) {
				for (j=0; j<n*(n/(size-1)); j++) {
					matA_block[j] = matA[t];
					t += 1;
				}
				MPI_Send(&block_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(matA_block, n*block_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
				double *matC_part = (double *)calloc(n*block_size, sizeof(double)); 
				MPI_Send(matC_part, n*block_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
				MPI_Recv(matC_part, n*block_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);
				for(k=0; k<n*block_size; k++) {
					matC[c] = matC_part[k];
					c++;
				}
			}
			else {
				double *matA_special = (double *)calloc(n*special_case, sizeof(double));
				for (k=0; k<n*special_case; k++) {
					matA_special[k] = matA[t];
					t++;
				}
				MPI_Send(&special_case, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(matA_special, n*special_case, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
				double *matC_special = (double *)calloc(n*special_case, sizeof(double));
				MPI_Send(matC_special, n*special_case, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
				for(j=0; j<n*special_case; j++) {
					matC[c] = matC_special[j];
					c++;
				}
			}
		}
		gettimeofday(&tpend, NULL);
		
		timeuse = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + (tpend.tv_usec - tpstart.tv_usec);
		printf("timeuse: %f\n", timeuse);
		if (n <= 16) {
			printf("Matrix A:\n");
			for (i=0; i<n*n; i++) {
				printf("%f ", matA[i]);
			}
			printf("\nMatrix B:\n");
			for (j=0; j<n*n; j++) {
				printf("%f ", matB[j]);
			}
			printf("\nMatrix C:\n");
			for (k=0; k<n*n; k++) {
				printf("%f ", matC[k]);
			}
		}
	}
	else {
		double *matA;// = (double *)calloc(n*n, sizeof(double));
		double *matB = (double *)calloc(n*n, sizeof(double));
		double *matC;// = (double *)calloc(n*n, sizeof(double));
		int size;
		MPI_Status status;

		MPI_Recv(matB, n*n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		matA = (double *)calloc(n*size, sizeof(double));
		matC = (double *)calloc(n*size, sizeof(double));
		
		MPI_Recv(matA, n*size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(matC, n*size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);

		for (i=0; i<size; i++) {
			for (j=0; j<n; j++) {
				for (k=0; k<n; k++) {
					matC[i*n+j] += matA[i*n+k]*matB[k*n+j];
				}
			}
		}
	/*	
		if (rank == 2) {
			for (i=0; i<n; i++) {
				printf("%f ", matC[i]);
			}
		}
	*/	
		MPI_Send(matC, n*size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	}
	
	MPI_Finalize();

	return 0;
}
