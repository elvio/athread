/*
 * anahy/structures.h
 * ------------------
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
 */
#ifndef _structures_h
#define _structures_h 1

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <mpi/mpi.h>

#define __USE_XOPEN2K
#include <semaphore.h>
#include <time.h>

#include "list.h"

/** thread status */
enum {
	JOB_UNASSIGNED,
	JOB_ASSIGNED,
	JOB_EXECUTING,
	JOB_DONE,
	JOB_JOINED
};

/** thread attributes */
enum {
	ATHREAD_CREATE_JOINABLE,
	ATHREAD_CREATE_DETACHED
};

/** join algorithms */
enum {
	GRAPH_ROOT,
	GRAPH_CURR_THREAD,
	GRAPH_JOINED_THREAD
};

enum {
	MATCH_JOB_ID,
	UNASSIGNED_JOB,
	SCHEDULING_REMOTE
};


#define athread_t long long
typedef void *(*pfunc)(void *);


struct remote_job {
	int remote_weight;
	int executing;
	time_t started_at;
	int data_type;
	size_t data_size;
	int service_id;
	int return_data_type;
	int slave;
	size_t return_data_size;
	pthread_mutex_t lock;
};

/* -- structures -- */

/** Estrutura responsável pelos atributos de um job */
struct job_attributes {
       /** Determina o número máximo de joins que um job terá */
	unsigned char max_joins;
	   /** Determina o estado atual */
	char detach_state;
       /** Determina se o job já foi inicializado */
	char initialized;
	pfunc in_pack_func;
	pfunc in_unpack_func;
	pfunc out_pack_func;
	pfunc out_unpack_func;
	int execution_cost;
	int communication_cost;
	long input_len;
	long output_len;
	
	/** Informacao para gerenciar threads remotas */
	struct remote_job *remote_job;
	
 	int n_threads;
 	int splitfactor;
 	size_t inputsize;
  	size_t returnsize;
	void *(* split)(void *, int, size_t, int);
	void *(* merge)(void *, int, int, void *);
};

struct job_reply {	 
         athread_t job_id;	 
         int job_status;	 
         long output_len;	 
 };



/** Estrutura que guarda propriedades específicas de um job */
struct job {
       /** Lista que guarda os jobs do mesmo nível */
	struct list_head job_list;
	   /** Lista que guarda os jobs filhos */
	struct list_head child_list;
	struct list_head vp_stack;
	   /** Status atual do job */
	unsigned long status;
	   /** thread "dona" do job */
	pthread_t owner;
	pthread_cond_t cond, reply_cond, steal_cond;
	pthread_mutex_t mutex, reply_mutex, steal_mutex;
 

	   /** ID do job */
	athread_t id;
	   /** Ponteiro para os IDs das threads splitadas */
	athread_t *vetsplit;
	   /** Atributos do job */
	struct job_attributes attribs;
	   /** Ponteiro para a função que este job executa */
	pfunc function;
	   /** Ponteiro para os dados de entrada do job */
	void *data;
	   /** Ponteiro para os dados gerados por este job */
	void *retval;

};

/** Estrutura de um VP */
struct vp_node {
	struct list_head vp_list;
	struct list_head job_stack;

	int keepalive;
	int sleeping;
	sem_t sem;
	sem_t steal_sem;
	pthread_t id;
	struct job *startup_job;
};

/** Estrutura que descreve um ambiente de execução de Anahy. */
struct engine {
	struct list_head vp_list;
	struct list_head job_list;
	
	pthread_mutex_t job_list_mutex;
	/** número máximo de vps da arquitetura */
  int max_vps;
	/** responsavel por dar o thread id **/
	athread_t new_id;
  /** Se esta rodando ou não. */
	int is_running;
	/** indica qual o algoritmo de procura no grafo utilizado */
	char search_algorithm;
	/** contagem dos argumentos passados por linha de comando */
	int argc;
	/** argumentos passados */
	char **argv;
};

/** estrutura contendo as informacoes passadas para e de uma funcao do usuario */
struct user_func_parameters {
	void *user_data;
};

/* -- globals -- */
struct engine engine;

/* -- typedefs -- */
//typedef inline int (*cond_search)(struct job *, athread_t *, pthread_t *);
typedef struct job_attributes athread_attr_t;



/* -- prototypes -- */
struct job *search_jobs(int match_condition, struct list_head *job_list, athread_t *job_id, pthread_t *vp_id);
int         destroy_jobs(struct list_head *job_list);

struct vp_node *current_vp();
pthread_t       main_vp_id();
struct job *    current_job(struct vp_node *vp);
void            execute_job(struct job *job, struct vp_node *vp);
void            wakeup_vps();
pthread_t       create_vp(struct job *startup_job);
int             destroy_vps(struct list_head *vp_list);

/* -- gc prototypes -- */
void job_list_info();

int athread_attr_init_defaults(athread_attr_t *attr);
int athread_attr_pack_in_func(athread_attr_t *attr, pfunc func);
int athread_attr_unpack_in_func(athread_attr_t *attr, pfunc func);
int athread_attr_pack_out_func(athread_attr_t *attr, pfunc func);
int athread_attr_unpack_out_func(athread_attr_t *attr, pfunc func);
int athread_attr_execution_cost(athread_attr_t *attr, int cost);
int athread_attr_communication_cost(athread_attr_t *attr, int cost);
void av_init();
//
int athread_attr_set_splitfactor(athread_attr_t *attr, int n);
int athread_attr_set_inputsize(athread_attr_t *attr, size_t inputsize);
int athread_attr_set_returnsize(athread_attr_t *attr, size_t inputsize);
int athread_attr_set_splitfunction(athread_attr_t *attr, void *(* split)(void *, int, size_t, int));
int athread_attr_set_mergefunction(athread_attr_t *attr, void *(* merge)(void *, int, int, void *));
//


/* use by remote code */
int athread_attr_set_remote_ability(athread_attr_t *attr, int status);
int athread_remote_register_service(int service, void *(*function)(void *));

struct remote_job_input {
	struct job *job;
	int slave;
};

struct remote_service {
	int service_id;
	pfunc function;
};

int registered_services_index;
struct remote_service registered_services[100];

int athread_remote_size;
int athread_remote_rank;

int athread_remote_send_job(struct job *job);
int remote_master();
int remote_slave();
double request_result_from_slave(int slave);


pthread_t singer_th;
pthread_t listener_th;

MPI_Status athread_remote_status_send;
MPI_Status athread_remote_status_receive;

MPI_Request athread_remove_request_send;
MPI_Request athread_remove_request_receive;



#ifdef __cplusplus
}
#endif
	
#endif /* _structures_h */
