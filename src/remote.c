#include <stdio.h>
#include <mpi/mpi.h>
#include "anahy/structures.h"
#include "anahy/macros.h"

#define MESSAGE_SIZE 3000
static pthread_mutex_t remote_mutex_slave = PTHREAD_MUTEX_INITIALIZER;

enum shared_oper {FRESH, EXECUTING, OKS};	
enum master_oper {M_OP_NEW_TASK, M_OP_TASK_DESC, M_OP_TASK_DATA};
enum slave_oper {S_OP_WTN_TASK_DESC, S_OP_WTN_TASK_DATA};
	
// int athread_remote_size;
// int athread_remote_rank;
// MPI_Status athread_remote_status_send;
// MPI_Status athread_remote_status_receive;
// MPI_Request athread_remove_request_send;
// MPI_Request athread_remove_request_receive;

/*
	MPI_Recv(&op_rec, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
	MPI_Send(&int_buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
*/

// hold status of each slave
int *slave_status;


int remote_master() {
	return ((athread_remote_rank == 0) ? 1 : 0);
}


int remote_slave() {
	return ((athread_remote_rank > 0) ? 1 : 0);
}


int get_available_slave() {
	int i;
	for (i=0; i<athread_remote_size; i++) {
		if (slave_status[i] == FRESH) {
			return i;
		}
	}
	return -1;
}

void *listener_thread(void *in) {
	int i;
	int int_buf;
	int received_op[athread_remote_size];
	int handle_index;
	MPI_Status status;
	MPI_Request *requests;
	
	printf("Starting other process == %d\n", athread_remote_rank);
	
	if (remote_slave()) {
		printf("starting as slave\n");

		for (i=0; i<10; i++) {
			printf("sending op\n");
			athread_remote_send_operation_to_master(OKS);
		}
	}
	
	
	// wait message form any slaves
	if (remote_master()) {
		requests = malloc(sizeof(MPI_Request) * athread_remote_size);
		
		printf("starting as master\n");
		for (i = 1; i < athread_remote_size; ++i) {
			printf("receiving...\n");
			MPI_Irecv(&received_op[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, &requests[i]);
		}

		// while (1) {
		// 	MPI_Waitany(athread_remote_size, requests, &handle_index, &status);
		// 	// got a weired message buddy... i guess we have to ignore it. don't you?
		// 	if (handle_index <= 0 || handle_index > athread_remote_size) {
		// 		printf("cleaned -- waiting for any again...\n");
		// 		free(requests);
		// 		requests = malloc(sizeof(MPI_Request) * athread_remote_size);
		// 		sleep(1);
		// 	} else {
		// 		printf("got message from #%d\n", handle_index);
		// 		sleep(1);
		// 	}
		// }
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
	
	// #ifdef DEBUG
	// 	printf("Creating active thread...\n");
	// #endif
	// pthread_create(&singer_th, NULL, singer_thread, (void *)NULL);
	// pthread_join(singer_th, (void *) NULL);

	#ifdef DEBUG
		printf("Creating listener thread...\n");
	#endif
	pthread_create(&listener_th, NULL, listener_thread, (void *)NULL);
	pthread_join(listener_th, (void *) NULL);
	return 0;
}

int aRemoteTerminate() {
	return 0;
}

int athread_remote_send_job(struct job *job) {
	int slave;
	Pthread_mutex_lock(job->job_list.mutex);
	job->status = JOB_EXECUTING;
	pthread_mutex_unlock(job->job_list.mutex);
	slave = get_available_slave();
	if (slave == -1) {
		printf("Could not find a available slave to execute remote thread\n");
		return 1;
	}
	return 0;
}

int athread_remote_send_operation(int operation, int rank) {
	MPI_Request request;
	//MPI_Isend(&operation, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &request);	
	return 0;
}

int athread_remote_send_operation_to_master(int operation) {
	athread_remote_send_operation(operation, 0);
	return 0;
}


