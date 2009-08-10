#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "time_log.h"
#include "athread.h"

#define INPUT 99999999

void *calculate(void *in) {
	int div_times = 0;
	int i;
	double sqrt_result;
	double input = INPUT;
	
	for (i=input; i > 0; i--) {
		sqrt_result = tan(sin(acos(sqrt(input * i))));
		if (fmod(input, i) == 0) {
			div_times++;
		}
	}
	
	return (void *) div_times;
}


void split(double weight) {
	int j;
	athread_t *threads;
	
	threads = malloc(sizeof(athread_t) * (int) weight);
	
	for (j=0; j < weight; j++) {
		athread_create(&threads[j], NULL, calculate, (void *) NULL);
	}
	
	for (j=0; j < weight; j++) {
		athread_join(threads[j], (void *) NULL);
	}
}


int main(int argc, char *argv[]) {
	double weight;
	char tag_log[100];

	weight = atof(argv[1]);
	aInit(&argc, &argv);
	set_file_log_path("logs/athread_version.log");
	sprintf(tag_log, "input:%d, weight:%2.0f", INPUT, weight);
	time_log_init(tag_log);
	split(weight);
	time_log_stop();
	compute_time_and_flush();
	aTerminate();
	return 0;
}
