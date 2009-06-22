/*
 * engine.c
 * --------
 * 
 * Copyright (C) 2004 by the Anahy Project
 *
 * Anahy is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * Anahy is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Anahy; if not, write to the Free Software Foundation, 
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * $Id: engine.c,v 1.2 2006/07/28 13:31:10 yolaf Exp $
 *
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <time.h>
#include "anahy/mutexes.h"
#include "anahy/structures.h"

/** Função que seta o tipo de algoritmo que o engine irá utilizar para varrer
o grafo. */
void
aSearchFrom(int algorithm)
{
	switch (algorithm) {
		case GRAPH_ROOT:
			engine.search_algorithm = GRAPH_ROOT;
			break;
		case GRAPH_CURR_THREAD:
			engine.search_algorithm = GRAPH_CURR_THREAD;
			break;
		case GRAPH_JOINED_THREAD:
		default:
			engine.search_algorithm = GRAPH_JOINED_THREAD;
			break;
	}
}
/** Função que inicializa o engine de Anahy. */

int
init_engine()
{
	memset(&engine, 0, sizeof(struct engine));
	init_list_head(&engine.vp_list, 1);
	init_list_head(&engine.job_list, 1);

	engine.search_algorithm = GRAPH_JOINED_THREAD;
	engine.max_vps = 2;
	engine.new_id = 0;
	srand ( time(NULL) );
	pthread_mutex_init (&steal_flag_mutex, NULL);
	pthread_mutex_init (&engine.job_list_mutex, NULL);
	return 0;
}
/** Função que encerra a execução do engine de anahy. */
int
aTerminate()
{
	int ret;
	int i;
#ifdef DEBUG
	printf("Terminating Anahy runtime\n");
#endif
	
	engine.is_running = 0;
#ifdef DEBUG
	printf("Destroying VPs\n");
#endif
	ret =  destroy_vps(&engine.vp_list);
#ifdef DEBUG
	printf("Destroying Jobs\n");
#endif
	ret += destroy_jobs(&engine.job_list);


	for (i=0; i<engine.argc; ++i)
		free(engine.argv[i]);

	if (engine.argv)
		free(engine.argv);
#ifdef DEBUG
	printf("Anahy runtime has become one with the force\n");
#endif
	return ret;
}
/** Função que inicia a execução do engine de anahy. */
int
aInit(int *user_argc, char ***user_argv)
{
	int i;
	int argc = *user_argc;
	int _optind = optind;
	int _opterr = opterr;
	int _optopt = optopt;
	char *_optarg = optarg;
	char **argv = *user_argv;
	static struct option long_options[] = {
		{"hostfile", 1, 0,  'f'},
		{"nhosts",   1, 0,  'n'},
		{"port",     1, 0,  'p'},
		{"vps",      1, 0,  'v'},
		{0, 0, 0, 0}
	};

	init_engine();
	
	// configure remote content
	athread_remote_init(user_argc, user_argv);
	athread_remote_slave_status();

	while (2) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "f:n:p:v:", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'v':
				engine.max_vps = atoi(optarg);
				if (engine.max_vps == 0) {
					fprintf(stderr, "The number of VPs must be greater than 0!\n");
					return -1;
				}
				fprintf(stderr, "Using %d VPs\n", engine.max_vps);
				break;
			case '?':
			default:
				break;
		}
	}

	engine.is_running = 1;
	engine.argc = argc - optind + 1;
	engine.argv = (char **) calloc(engine.argc, sizeof(char *));
	if (! engine.argv) {
		perror("calloc");
		return -ENOMEM;
	}
	
	engine.argv[0] = strdup(*user_argv[0]);
	for (i=1; optind < argc; optind++, i++)
		engine.argv[i] = strdup(argv[optind]);
	
	*user_argc = engine.argc;
	*user_argv = engine.argv;
	
	optind = _optind;
	opterr = _opterr;
	optopt = _optopt;
	optarg = _optarg;
	
	
	return 0;
}
