#include <stdio.h>
#include <mpi/mpi.h>
#include "anahy/structures.h"
#include "anahy/macros.h"

#define MESSAGE_SIZE 3000
#define MASTER_ID 0
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
	for (i=0; i < (athread_remote_size-1); i++) {
		if (slave_status[i] == FRESH) {
			return i+1;
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
		//
	}
}


int aRemoteTerminate() {
	return 0;
}


/*
	Send task desc to slave
	Registred task
	Input data type
	Input data size
	Input data return type
	Input data return size
*/
void *athread_remote_master_task_desc(void *in) {
	int op_buf;
	int op_rec;
	struct remote_job_input *input;
	
	input = (struct remote_job_input *) in;
	pthread_t *task_desc_thread;
	
	MPI_Status status;
	op_buf = M_OP_NEW_TASK;

	printf("Sending TASK_DESC to %d...\n", input->slave);
	MPI_Send(&op_buf, 1, MPI_INT, input->slave, 0, MPI_COMM_WORLD);

	printf("Waiting OKS from %d...\n", input->slave);
	MPI_Recv(&op_rec, 1, MPI_INT, input->slave, 0, MPI_COMM_WORLD, &status);
	
	printf("Waiting TASK_RESULT from %d...\n", input->slave);
	MPI_Recv(&op_rec, 1, MPI_INT, input->slave, 0, MPI_COMM_WORLD, &status);
	
	// update value locally
	// update job status
	// update slave status
}

/*
	Send a M_OP_NEW_TASK message to slave defined trhough *(int *) in
	Wait for an OK. Fail if ok was not sent
	MPI_Recv(&op_rec, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
	MPI_Send(&int_buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	struct remote_job_input {
		struct job *job;
		int slave;
	};
	
*/
void *athread_remote_master_new_task_thread(void *in) {
	int op_buf;
	int op_rec;
	struct remote_job_input *input;
	
	input = (struct remote_job_input *) in;
	pthread_t *task_desc_thread;
	
	MPI_Status status;
	op_buf = M_OP_NEW_TASK;

	printf("Sending M_OP_NEW_TASK to %d...\n", input->slave);
	MPI_Send(&op_buf, 1, MPI_INT, input->slave, 0, MPI_COMM_WORLD);

	printf("Waiting OKS from %d...\n", input->slave);
	MPI_Recv(&op_rec, 1, MPI_INT, input->slave, 0, MPI_COMM_WORLD, &status);
	
	if (op_rec != OKS) {
		printf("Waiting %d from slave but got a %d. Aborting...\n", OKS, op_rec);
		exit(1);
	}
	
	printf("Got OKS from %d\n", input->slave);
	
	// now we now slave is ready, time to send the task desc to it
	task_desc_thread = malloc(sizeof(pthread_t));
	pthread_create(task_desc_thread, NULL, athread_remote_master_task_desc, in);
	
	// sleep for awhile and then kill this thread
	sleep(1);
	
	// arakiri
	//printf("ARAKIRI -- new_task_thread\n");
	//pthread_cancel(pthread_self());
}

void *athread_remote_slave_send_oks(void *in) {
	int op_buf;
	int op_rec;
	MPI_Status status;
	
	op_buf = OKS;
	printf("Waiting NEW_TASK from master --- slave #%d...\n", athread_remote_rank);
	MPI_Recv(&op_rec, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD, &status);

	if (op_rec != M_OP_NEW_TASK) {
		printf("Waiting NEW_TASK operation but got %d\n", op_rec);
		return;
	}
	
	printf("Sending OKS to master...\n");
	MPI_Send(&op_buf, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD);
	
	sleep(1);
	
	printf("Waiting NEW_TASK_DESC from master --- slave #%d...\n", athread_remote_rank);
	MPI_Recv(&op_rec, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD, &status);
	
	printf("Got TASK_DESC. Sending OKS to master...\n");
	MPI_Send(&op_buf, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD);
	
	// execute job
	// return value
}

int athread_remote_send_job(struct job *job) {
	int slave;
	struct remote_job_input *rinput;
	pthread_t *new_task;
	
	// return if process is a slave
	if (remote_slave()) return 0;
	
	printf("Start process to send remote job..\n");
	Pthread_mutex_lock(job->job_list.mutex);
	job->status = JOB_EXECUTING;
	pthread_mutex_unlock(job->job_list.mutex);
	slave = get_available_slave();
	if (slave == -1) {
		printf("Could not find a available slave to execute remote thread\n");
		return 1;
	}
	
	rinput = malloc(sizeof(struct remote_job_input));
	rinput->job = job;
	rinput->slave = slave;
	
	printf("Creating NEW_TASK thread\n");
	new_task = malloc(sizeof(pthread_t));
	pthread_create(new_task, NULL, athread_remote_master_new_task_thread, (void*) rinput);
	pthread_join(*new_task, (void *) NULL);
	
	while (1);
	return 0;
}

int aRemoteInit(int argc, char **argv) {
	int i;
	pthread_t *slave_oks;
	
	#ifdef DEBUG
		printf("MPI configuration is done. I guess it was the hard part ;)\n");
	#endif

	if (remote_master()) {
		printf("Starting slave status...\n");
		slave_status = malloc(sizeof(int) * (athread_remote_size-1));
		for (i=0; i < athread_remote_size; i++){
			slave_status[i] = FRESH;
		}
	} else if (remote_slave()) {
		printf("Starting slave OKS thread...\n");
		slave_oks = malloc(sizeof(pthread_t));
		pthread_create(slave_oks, NULL, athread_remote_slave_send_oks, (void*) NULL);
	}
	
	#ifdef DEBUG
		printf("Slave status has been created with no sigfault. We are so lucky today...\n");
	#endif
		
	return 0;
}
