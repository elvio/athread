#include <stdio.h>
#include <stdlib.h>
#include "athread.h"

#define MAX 5


// tags
#define REMOTE_SERVICE_ID 1
#define REMOTE_SERVICE_ID_2 2


void *remote_th(void *in) {
	double *result = malloc(sizeof(double));
	
	printf("remote_th --- athread_remote_rank == %d\n", athread_remote_rank);
	*result = *(double *) in;
	*result += 10;
	
	return (void *) result;
}

void *remote_th_2(void *in) {
	double *result = malloc(sizeof(double));
	
	printf("remote_th --- athread_remote_rank == %d\n", athread_remote_rank);
	*result = *(double *) in;
	*result += 12;
	
	return (void *) result;
}


int main(int argc, char *argv[]) {
	athread_t remote_thread;
	athread_attr_t remote_thread_attr;
	
	athread_t remote_thread_2;
	athread_attr_t remote_thread_attr_2;
	
	
	double *input_value = malloc(sizeof(double));
	double *result;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);
	printf("started rank ==> %d\n", athread_remote_rank);

	aRemoteInit(argc, argv);
	aInit(&argc, &argv);
	
	// create remote threads and let ir runn.. too..
	*input_value = 10;
	
	// register service
	athread_remote_register_service(REMOTE_SERVICE_ID, remote_th);
	athread_remote_register_service(REMOTE_SERVICE_ID_2, remote_th_2);
	
	athread_attr_init(&remote_thread_attr);
	athread_attr_set_remote_ability(&remote_thread_attr, 1);
	athread_attr_set_remote_service(&remote_thread_attr, REMOTE_SERVICE_ID);
	
	athread_attr_init(&remote_thread_attr_2);
	athread_attr_set_remote_ability(&remote_thread_attr_2, 1);
	athread_attr_set_remote_service(&remote_thread_attr_2, REMOTE_SERVICE_ID_2);
	
	
	athread_create(&remote_thread, &remote_thread_attr, remote_th, (void *) input_value);
	athread_create(&remote_thread_2, &remote_thread_attr_2, remote_th_2, (void *) input_value);
	//athread_join(remote_thread, (void*) result);
	
	printf("waiting...");
	while(1);
	
	aTerminate();
	return 0;
}
