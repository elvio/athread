#include <mpi/mpi.h>
#include "anahy/structures.h"

#define MESSAGE_SIZE 3000

enum master_status {FRESH, WAITING_SLAVE, SLAVE_IS_DONE, DESCRIPTION_SENT, 
	SLAVE_WANT_DATA, DATA_SENT, SLAVE_COMPLETED_COMPUTATION, 
	REQUESTED_TASK_DESCRIPTION, TASK_DESCRIPTION_RECEIVED, 
	REQUESTED_TASK_DATA, TASK_DATA_RECEIVED};
	

// int athread_remote_size;
// int athread_remote_rank;
// MPI_Status athread_remote_status_send;
// MPI_Status athread_remote_status_receive;
// MPI_Request athread_remove_request_send;
// MPI_Request athread_remove_request_receive;


// hold status of each slave
int *slave_status;

void *__recv_thread(void *in) {
}

void *__send_thread(void *in) {
}


int athread_remote_init(int argc, char *argv[]) {
	int i;
	
	#ifdef DEBUG
		printf("remote_init called, here we go :|\n");
	#endif
		
	MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);

	#ifdef DEBUG
		printf("MPI configuration is done. I guess it was the hard part ;)\n");
	#endif

	// init status of slaves
	slave_status = malloc(sizeof(int) * athread_remote_size);
	for (i=0; i < athread_remote_size; i++){
		slave_status[i] = FRESH;
	}
	
	#ifdef DEBUG
		printf("Slave status has been created with no sigfault. We are so lucky today...\n");
	#endif
}

int athread_remote_terminate() {
	return 0;
}