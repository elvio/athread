#include <stdio.h>
#include <mpi/mpi.h>
#include "anahy/structures.h"
#include "anahy/macros.h"

#define MESSAGE_SIZE 3000
#define MASTER_ID 0
static pthread_mutex_t remote_mutex_slave = PTHREAD_MUTEX_INITIALIZER;

enum shared_oper {FRESH, EXECUTING, OKS};	
enum master_oper {NEW_TASK, NEW_TASK_DATA};
	
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


void *athread_remote_slave_wait_master_task(void *in) {
	int op_buf;
	int op_rec;
	MPI_Status *status;
	
	status = NULL;
	status = malloc(sizeof(MPI_Status));
	free(status);
}

/*
	Receive a INT == operation
*/
int receive_op_from_slave(int slave) {
	int op_rec;
	MPI_Status status;
	MPI_Recv(&op_rec, 1, MPI_INT, slave, 0, MPI_COMM_WORLD, &status);
	return op_rec;
}

/*
	Send operation code to slave
*/
void send_op_to_slave(int operation, int slave) {
	MPI_Send(&operation, 1, MPI_INT, slave, 0, MPI_COMM_WORLD);
}


/*
	Receive a INT == operation from MASTER
*/
int receive_op_from_master() {
	int op_rec;
	MPI_Status status;
	MPI_Recv(&op_rec, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD, &status);
	return op_rec;
}

/*
	Send operation code to MASTER
*/
void send_op_to_master(int operation) {
	MPI_Send(&operation, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD);
}

/*
	request ok from slave and abort if it fails
*/
void request_ok_from_slave(int slave) {
	int op_rec;
	
	printf("Waiting OKS from slave %d\n", slave);
	op_rec = receive_op_from_slave(slave);
	printf("Got it\n");
	
	if (op_rec != OKS) {
		printf("(SLAVE %d) -- We were want a OKS operation but got other... Aborting...\n", slave);
		exit(1);
	}
}

/*
	create a thread to handle slave content
*/
void *athread_remote_slave_execute_job(void *in) {
	/*
		send ok to master
		recv new task
		send ok
		send result
	*/
	send_op_to_master(OKS);
	return (void *)NULL;
}



/*
	create a thread to handle the new remote job
	input:
		rinput->job = job;
		rinput->slave = slave;
*/
void *athread_remote_master_execute_job(void *in) {
	int op_buf;
	int op_rec;
	MPI_Status *status;
	struct remote_job_input *remote_job_input = (struct remote_job_input *) in;
	
	/*
		**** steps to take ***
		----------------------
		recv ok from slave
		send new task
		recv ok
		send task
		recv ok
		recv result
		update status
		mark job as done
	*/

	request_ok_from_slave(remote_job_input->slave);
	
	printf("Send new task request to slave %d\n", remote_job_input->slave);
	send_op_to_slave(NEW_TASK, remote_job_input->slave);
	printf("Sent done. Slave %d got it\n", remote_job_input->slave);
	
	request_ok_from_slave(remote_job_input->slave);
	
	printf("Sending task to slave %d\n", remote_job_input->slave);
	printf("Task was sent. Slave %d got it. Waiting result\n", remote_job_input->slave);
	
}

int athread_remote_send_job(struct job *job) {
	int slave;
	struct remote_job_input *rinput;
	pthread_t *remote_job;
	
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

	// create input to new job
	rinput = malloc(sizeof(struct remote_job_input));
	rinput->job = job;
	rinput->slave = slave;
	
	// create thread to handle new job
	remote_job = malloc(sizeof(pthread_t));
	pthread_create(remote_job, NULL, athread_remote_master_execute_job, (void *) rinput);

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
		pthread_create(slave_oks, NULL, athread_remote_slave_execute_job, (void*) NULL);
	}
	
	#ifdef DEBUG
		printf("Slave status has been created with no sigfault. We are so lucky today...\n");
	#endif
		
	return 0;
}


int athread_remote_register_service(int service, void *(*function)(void *)) {
	registred_services[registred_services_index].service_id = service;
	registred_services[registred_services_index].function = function;
	registred_services_index+=1;
}