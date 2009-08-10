#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "time_log.h"
#include "athread.h"

#define INPUT 999999

void *calculate(void *in) {
	double *div_times;
	int i;
	double sqrt_result;
	double input = INPUT;
	
	div_times = malloc(sizeof(double));
	
	for (i=input; i > 0; i--) {
		sqrt_result = tan(sin(acos(sqrt(input * i))));
		if (fmod(input, i) == 0) {
			*div_times = *div_times + 1;
		}
	}
	
	return (void *) div_times;
}


void split(double weight) {
	int j;
	double *result;
	athread_t *threads;
	
	threads = malloc(sizeof(athread_t) * (int) weight);
	
	for (j=0; j < weight; j++) {
		athread_create(&threads[j], NULL, calculate, (void *) NULL);
	}
	
	result = malloc(sizeof(double));
	for (j=0; j < weight; j++) {
		printf("-- join #%d\n", j);
		athread_join(threads[j], (void *) result);
	}
}


int main(int argc, char *argv[]) {
	double weight;
	char tag_log[100];
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &athread_remote_rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &athread_remote_size);

	weight = atof(argv[1]);
	aInit(&argc, &argv);
	aRemoteInit(argc, argv);
	
	set_file_log_path("logs/athread_version.log");
	sprintf(tag_log, "input:%d, weight:%2.0f", INPUT, weight);
	time_log_init(tag_log);
	split(weight);
	time_log_stop();
	compute_time_and_flush();
	
	aRemoteTerminate();
	aTerminate();
	return 0;
}
