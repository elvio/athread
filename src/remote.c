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
	printf("athread_remote_rank => %d and athread_remote_size => %d\n", athread_remote_rank, athread_remote_size);
	for (i=0; i < (athread_remote_size-1); i++) {
		printf("slave #%d == %d and FRESH == %d\n", i+1, slave_status[i], FRESH);
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
	request a service id
*/
int request_service_id() {
	int service_id;
	MPI_Status status;

	printf("slave #%d is requesting service id..\n", athread_remote_rank);
	MPI_Recv(&service_id, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD, &status);
	printf("slave #%d got service id..\n", athread_remote_rank);

	return service_id;
}


/*
	request input data
*/
double request_input_data() {
	double input_data;
	MPI_Status status;

	printf("slave #%d is requesting input data..\n", athread_remote_rank);
	MPI_Recv(&input_data, 1, MPI_DOUBLE, MASTER_ID, 0, MPI_COMM_WORLD, &status);
	printf("slave #%d got input data..\n", athread_remote_rank);

	return input_data;
}

/*
	Send operation code to MASTER
*/
void send_op_to_master(int operation) {
	printf("Slave %d sending op to master\n", athread_remote_rank);
	MPI_Send(&operation, 1, MPI_INT, MASTER_ID, 0, MPI_COMM_WORLD);
	printf("slave %d -- Operation sent\n", athread_remote_rank);
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
	int service_id;
	double input_data;
	/*
		send ok to master
		recv task service_id
		send ok
		recv task data input
		send ok
		send result
	*/
	send_op_to_master(OKS);
	service_id = request_service_id();
	send_op_to_master(OKS);
	input_data = request_input_data();
	send_op_to_master(OKS);
	printf("Time to execute...\n");

	while(1);
	
	return (void *)NULL;
}


void send_service_id_to_slave(int service_id, int slave) {
	printf("Sending service_id = %d to slave = %d\n", service_id, slave);
	MPI_Send(&service_id, 1, MPI_INT, slave, 0, MPI_COMM_WORLD);
	printf("Service sent --- service_id = %d, slave = %d\n", service_id, slave);
}

void send_service_data_input_to_slave(double job_input_data, int slave) {
	printf("Sending input data(%f) to slave = %d\n", job_input_data, slave);
	MPI_Send(&job_input_data, 1, MPI_DOUBLE, slave, 0, MPI_COMM_WORLD);
	printf("Input data sent --- input data = %f	 to slave = %d\n", job_input_data, slave);
}

double request_result_from_slave(int slave) {
	MPI_Status status;
	double result;
	
	printf("Requesting result from slave #%d\n", slave);
	MPI_Recv(&result, 1, MPI_DOUBLE, MASTER_ID, 0, MPI_COMM_WORLD, &status);
	printf("Got result from slave #%d. Time to finish\n", slave);
	
	return result;
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
	double job_data;
	double result;
	MPI_Status *status;
	struct remote_job_input *remote_job_input = (struct remote_job_input *) in;
	struct job *job = remote_job_input->job;
	
	/*
		**** steps to take ***
		----------------------
		recv ok from slave
		send task service_id
		recv ok
		send task data input
		recv ok
		recv result
		update status
		mark job as done
	*/

	request_ok_from_slave(remote_job_input->slave);
	send_service_id_to_slave(job->attribs.remote_job.service_id, remote_job_input->slave);
	request_ok_from_slave(remote_job_input->slave);
	
	job_data = *(double*) job->data;
	send_service_data_input_to_slave(job_data, remote_job_input->slave);
	request_ok_from_slave(remote_job_input->slave);
	result = request_result_from_slave(remote_job_input->slave);

	return (void*) NULL;
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
	printf("Creating thread to send job\n");
	remote_job = malloc(sizeof(pthread_t));
	pthread_create(remote_job, NULL, athread_remote_master_execute_job, (void *) rinput);
	printf("Thread was created buddy...\n");
	
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