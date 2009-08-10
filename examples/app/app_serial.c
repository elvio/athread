#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "time_log.h"

#define INPUT 99999999

int calculate(double weight) {
	int div_times = 0;
	int i, j;
	double sqrt_result;
	double input = INPUT;
	
	for (j=0; j < weight; j++) {
		div_times = 0;
		for (i=input; i > 0; i--) {
			sqrt_result = tan(sin(acos(sqrt(input * i))));
			if (fmod(input, i) == 0) {
				div_times++;
			}
		}
	}
	
	return div_times;
}

int main(int argc, char *argv[]) {
	int result;
	double weight;
	char tag_log[100];

	weight = atof(argv[1]);
	
	set_file_log_path("logs/serial_version.log");
	sprintf(tag_log, "input:%d, weight:%2.0f", INPUT, weight);
	time_log_init(tag_log);
	result = calculate(weight);
	time_log_stop();
	compute_time_and_flush();
	return 0;
}