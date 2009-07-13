#include <stdio.h>
#include <stdlib.h>
#include "athread.h"

#define MAX 5


// tags
#define REMOTE_SERVICE_ID 1
#define REMOTE_SERVICE_ID_2 2
#define MAXY 3000


void *remote_th(void *in) {
	printf("inside 1\n");
	double *result;
	
	result = malloc(sizeof(double));
	*result = *(double *) in;
	*result += 15;
	return (void *) result;
}

void *remote_th_2(void *in) {
	printf("inside 2\n");
	return (void *) NULL;
}

int main(int argc, char *argv[]) {
	athread_t remote_thread;
	athread_attr_t remote_thread_attr;
	
	athread_t remote_thread_2;
	athread_attr_t remote_thread_attr_2;
	
	
	double *input_value = malloc(sizeof(double));
	double *result = malloc(sizeof(double));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);

	aInit(&argc, &argv);
	
	// create remote threads and let ir runn.. too..
	*input_value = 10;
	
	// register service
	athread_remote_register_service(REMOTE_SERVICE_ID, remote_th);
	athread_remote_register_service(REMOTE_SERVICE_ID_2, remote_th_2);

	aRemoteInit(argc, argv);
	
	if (athread_remote_rank == 0) {
		athread_attr_init(&remote_thread_attr);
		athread_attr_set_remote_ability(&remote_thread_attr, 1);
		athread_attr_set_remote_service(&remote_thread_attr, REMOTE_SERVICE_ID);
	
		// athread_attr_init(&remote_thread_attr_2);
		// athread_attr_set_remote_ability(&remote_thread_attr_2, 1);
		// athread_attr_set_remote_service(&remote_thread_attr_2, REMOTE_SERVICE_ID_2);
	
		athread_create(&remote_thread, &remote_thread_attr, remote_th, (void *) input_value);

		// athread_create(&remote_thread_2, &remote_thread_attr_2, remote_th_2, (void *) input_value);

		athread_join(remote_thread, (void*) result);
		// printf("Depois do join 1 = %2.2f\n", *result);
		
		// athread_join(remote_thread_2, (void*) result);
		// printf("Depois do join 2 = %2.2f\n", *result);
	}
	
	printf("[ALOHHA] - Alohha before terminate from rank == %d\n", athread_remote_rank);
	aRemoteTerminate();
	
	aTerminate();
	return 0;
}
