#include <stdio.h>
#include <stdlib.h>
#include "athread.h"

#define MAX 5

void *local1(void *input) {
	int i;
	for (i=0; i < MAX; i++) {
		printf("Alohha from local 1...\n");
		sleep(1);
	}
}

void *local2(void *input) {
	int i;
	for (i=0; i < MAX; i++) {
		printf("Alohha from local 2...\n");
		sleep(1);
	}
}

void *remote1(void *input) {
	int max_i, max_j;
	int i, j;
	int *total;
	
	total = malloc(sizeof(int));
	*total = 0;
	
	max_i = *(int *) input;
	max_j = max_i;
	
	printf("Starting remote 1 calc\n");
	for (i=0; i < max_i; i++) {
		for (j=0; j < max_j; j++) {
			*total += 1;
		}
	}
}

void *remote2(void *input) {
	int max_i, max_j;
	int i, j;
	int *total;
	
	total = malloc(sizeof(int));
	*total = 0;
	
	printf("Starting remote 2 calc\n");
	
	max_i = *(int *) input;
	max_j = max_i;
	
	for (i=0; i < max_i; i++) {
		for (j=0; j < max_j; j++) {
			*total += 2;
		}
	}
}



int main(int argc, char *argv[]) {
	athread_t remote_thread1, remote_thread2;
	athread_t local_thread1, local_thread2;
	athread_attr_t remote1_attr;
	athread_attr_t remote2_attr;
	
	int *input_value = malloc(sizeof(int));
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);

	printf("started rank ==> %d\n", athread_remote_rank);

	aRemoteInit(argc, argv);
	aInit(&argc, &argv);
	
	
	// // create local threads and let it runnn...
	// 	athread_create(&local_thread1, NULL, local1, (void *) NULL);
	// 	athread_create(&local_thread2, NULL, local2, (void *) NULL);
	
	// create remote threads and let ir runn.. too..
	*input_value = 10;
	
	athread_attr_init(&remote1_attr);
	athread_attr_init(&remote2_attr);
	athread_attr_set_remote_ability(&remote1_attr, 1);
	athread_attr_set_remote_ability(&remote2_attr, 1);
	
	printf("create 1\n");
	athread_create(&remote_thread1, &remote1_attr, remote1, (void *) input_value);
	printf("create 2\n");
	athread_create(&remote_thread2, &remote2_attr, remote2, (void *) input_value);
	
	
	// athread_join(local_thread1, (void*)NULL);
	// athread_join(local_thread2, (void*)NULL);
	
	printf("join 1\n");
	athread_join(remote_thread1, (void*)NULL);
	
	printf("join 2\n");
	athread_join(remote_thread2, (void*)NULL);
	
	aTerminate();
	return 0;
}
