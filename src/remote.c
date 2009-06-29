#include <stdio.h>
#include <mpi/mpi.h>
#include "anahy/structures.h"
#include "anahy/macros.h"
#include "anahy/vps.h"
#include "anahy/list.h"
#include "anahy/mutexes.h"

#define MESSAGE_SIZE 3000

enum master_status {FRESH, WAITING_SLAVE, SLAVE_IS_DONE, DESCRIPTION_SENT, 
	SLAVE_WANT_DATA, DATA_SENT, SLAVE_COMPLETED_COMPUTATION, 
	REQUESTED_TASK_DESCRIPTION, TASK_DESCRIPTION_RECEIVED, 
	REQUESTED_TASK_DATA, TASK_DATA_RECEIVED};
	
enum operations {R_OP_READY};
enum answers {R_A_READY};
	
	

// int athread_remote_size;
// int athread_remote_rank;
// MPI_Status athread_remote_status_send;
// MPI_Status athread_remote_status_receive;
// MPI_Request athread_remove_request_send;
// MPI_Request athread_remove_request_receive;


// hold status of each slave
int *slave_status;

void athread_remote_slave_status() {
	int i;
	
	for (i=0; i<athread_remote_size; i++) {
		printf("Status to slave #%d => ", i);
		switch (slave_status[i]) {
			case FRESH: printf("FRESH"); break;
			case WAITING_SLAVE: printf("WAITING_SLAVE"); break;
			case SLAVE_IS_DONE: printf("SLAVE_IS_DONE"); break;
			case DESCRIPTION_SENT: printf("DESCRIPTION_SENT"); break;
			case SLAVE_WANT_DATA: printf("SLAVE_WANT_DATA"); break;
			case DATA_SENT: printf("DATA_SENT"); break;
			case SLAVE_COMPLETED_COMPUTATION: printf("SLAVE_COMPLETED_COMPUTATION"); break;
			case REQUESTED_TASK_DESCRIPTION: printf("REQUESTED_TASK_DESCRIPTION"); break;
			case TASK_DESCRIPTION_RECEIVED: printf("TASK_DESCRIPTION_RECEIVED"); break;
			case REQUESTED_TASK_DATA: printf("REQUESTED_TASK_DATA"); break;
			case TASK_DATA_RECEIVED: printf("TASK_DATA_RECEIVED"); break;
		}
		printf("\n");
	}
	printf("\n");
}


int should_i_act_as_master() {
	return ((athread_remote_rank == 0) ? 1 : 0);
}


int should_i_act_as_slave() {
	return ((athread_remote_rank != 0) ? 1 : 0);
}

void *active_thread(void *in) {
	int i;
	int int_buf;
	int op_rec;
	MPI_Request request;

	
	if (should_i_act_as_slave()) {
		printf("Sending R_A_READY to master from %d\n", athread_remote_rank);
		int_buf = R_A_READY;
		MPI_Send(&int_buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
}

void *passive_thread(void *in) {
	int i;
	int int_buf;
	int op_rec;
	MPI_Status status;
	
	if (should_i_act_as_master()) {
		printf("Waiting all slaves get ready...\n");
		for (i=1; i < athread_remote_size; i++) {
			printf("Waiting #%d get ready...\n", i);
			MPI_Recv(&op_rec, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
		}
	}
}

int aRemoteInit(int argc, char **argv) {
	int i;
	
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
	
	#ifdef DEBUG
		printf("Creating active thread...\n");
	#endif
	pthread_create(&active_thread_th, NULL, active_thread, (void *)NULL);

	#ifdef DEBUG
		printf("Creating passive thread...\n");
	#endif
	pthread_create(&passive_thread_th, NULL, passive_thread, (void *)NULL);
	
	pthread_join(active_thread_th, (void *) NULL);
	pthread_join(passive_thread_th, (void *) NULL);
	
	
	return 0;
}

int aRemoteTerminate() {
	return 0;
}

int athread_remote_send_job(struct job *job) {
	Pthread_mutex_lock(job->job_list.mutex);
	job->status = JOB_EXECUTING;
	pthread_mutex_unlock(job->job_list.mutex);
}
