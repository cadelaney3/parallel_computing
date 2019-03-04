#include <stdio.h>
#include <mpi.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int rank, size, namelen;
	pid_t pid;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Get_processor_name(processor_name, &namelen);

	pid = getpid();
	int return_pid;
	int smallest_id = 9999999;
	int smallest_rank = 9999999;
	int i;

	for (i=0; i<size; i++) {
		MPI_Sendrecv(&pid, 1, MPI_INT, (rank+i+1)%size, 1, &return_pid, 1, MPI_INT, (rank-1-i+size)%size, 1, MPI_COMM_WORLD, &status);
		if (return_pid < smallest_id) {
			smallest_id = return_pid;
			smallest_rank = status.MPI_SOURCE;
		}
	}
	
	printf("Process %d: my ID is %d; the smallest process ID "
	       "%d is the one from Process %d\n", rank, pid, smallest_id, smallest_rank);
	MPI_Finalize();
	return 0;
}	
