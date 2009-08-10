#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "time_log.h"

struct timeval start, stop;
unsigned int diffsec, diffusec;
char tag[100];
static char file_log_path[] = "timelog.log";

void time_log_init(char *ptag) {
	strcpy(tag, ptag);
	gettimeofday(&start, NULL);
}

void time_log_stop() {
	gettimeofday(&stop, NULL);
	compute_time_and_flush();
}

void compute_time_and_flush() {
	FILE *fp;
	char content[300];
	if ((fp = fopen(file_log_path, "a")) == NULL) {
		fprintf(stderr, "Erro na abertura no arquivo de log...\n");
		exit(EXIT_FAILURE);
	}
	else {
		diffsec = stop.tv_sec - start.tv_sec ;
		diffusec = (stop.tv_usec - start.tv_usec) >= 0 ? (stop.tv_usec - start.tv_usec) : 1000000 - stop.tv_usec;
		
		sprintf(content, "%s -- %3d.%-6d\n", tag, diffsec, diffusec);	
		fprintf(fp, content);
		
		fclose(fp);
	}
}

void set_file_log_path(char *path){
	strcpy(file_log_path, path);
}
