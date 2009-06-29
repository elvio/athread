/*
 * vps.c
 * -----
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
 * $Id: vps.c,v 1.2 2006/07/28 13:24:45 yolaf Exp $
 *
 */
#include "anahy/structures.h"
#include "anahy/macros.h"
#include "anahy/vps.h"
#include "anahy/list.h"
#include "anahy/mutexes.h"

/* -- vps -- */
/** Função que põe um VP pra dormir.
@param *vp  ponteiro para a estrutura que representa akele vp */
void
sleep_vp(struct vp_node *vp)
{
	struct list_head *pos;
	int suicide = 0;
	int acc=0;
	struct timespec timeout;
#ifdef DEBUG
	int valor;
#endif
	
	vp->sleeping = 1;
	pthread_mutex_lock(engine.vp_list.mutex);//<-Killed
	if ((list_size(engine.vp_list)) > engine.max_vps) {
		list_del(&vp->vp_list);
		sem_destroy(&vp->sem);
		free(vp);
		suicide = 1;
		//fprintf(stderr,"I'll be back!\n");
	}
	pthread_mutex_unlock(engine.vp_list.mutex);
	
	if (suicide){
		//printf("Adeus Mundo Cruel (sleep)!\n");
		pthread_exit(NULL);
	}
	
/*	if ( vp->id != main_vp_id()) {
		list_for_each(pos, &engine.vp_list) {
			struct vp_node *vp_test = list_entry(pos, struct vp_node, vp_list);
				acc+= vp_test->sleeping;
		}
	}*/
#ifdef DEBUG
	sem_getvalue(&vp->sem,&valor);
	printf("Stoping VP %d in semaphore. Current sem value is: %d\n", vp->id , valor);
#endif
	sem_wait(&vp->sem);
	vp->sleeping = 0;
}

/** Função que acorda todos os vps que estão dormindo*/
void
wakeup_vps()
{
	struct list_head *pos;
#ifdef DEBUG
	int valor;
#endif
	
	pthread_mutex_lock(engine.vp_list.mutex);
	list_for_each(pos, &engine.vp_list) {
		struct vp_node *vp = list_entry(pos, struct vp_node, vp_list);
		if (vp->sleeping == 1) {
			sem_post(&vp->sem);
#ifdef DEBUG
			sem_getvalue(&vp->sem, &valor);
			printf("VP %d sem is now: %d\n", vp->id, valor);
#endif
		}
	}
	pthread_mutex_unlock(engine.vp_list.mutex);
}

void
wakeup_main_vp()
{
	struct list_head *pos;
	
	pthread_mutex_lock(engine.vp_list.mutex);
	list_for_each(pos, &engine.vp_list) {
		struct vp_node *vp = list_entry(pos, struct vp_node, vp_list);
		if ( (vp->id == main_vp_id()) && (vp->sleeping == 1))
			sem_post(&vp->sem);
	}
	pthread_mutex_unlock(engine.vp_list.mutex);
}


/** Função que faz com que uma tarefa seja executada em um vp. 
@param *job ponteiro para o descritor da tarefa
@param *vp ponteiro para o descritor do vp */
void
execute_job(struct job *job, struct vp_node *vp)
{
	void *job_data, *job_retval;
	
	if ( job_has_remote_ability(job) ) {
		athread_remote_send_job(job);
		return;
	}
	
	Pthread_mutex_lock(job->job_list.mutex);
	job->status = JOB_EXECUTING;
	job_data = job->data;
	list_add(&job->vp_stack, &vp->job_stack);
	pthread_mutex_unlock(job->job_list.mutex);
	
	vp->startup_job = NULL;
	job_retval = job->function(job_data);
	
	Pthread_mutex_lock(job->job_list.mutex);
	job->status = JOB_DONE;
	job->retval = job_retval;
	list_del(&job->vp_stack);
	pthread_mutex_lock(&job->mutex);
	pthread_cond_broadcast(&job->cond);
	pthread_mutex_unlock(&job->mutex);
	pthread_mutex_unlock(job->job_list.mutex);
}

void *
vp_listen(void *data)
{
	struct job *job;
	struct vp_node *vp = (struct vp_node *) data;
	
	vp->id = pthread_self();
	while (2) {
		if (vp->startup_job){
#ifdef DEBUG
			printf("Executing pre-defined startup job.\n");
#endif
			execute_job(vp->startup_job, vp);
		}
		
		/* search for jobs on the local job_list */
		if (engine.is_running) {
#ifdef DEBUG
			printf("Searching local list for jobs\n");
#endif
			pthread_mutex_lock(&engine.job_list_mutex);
			job = search_jobs(UNASSIGNED_JOB, &engine.job_list, NULL, &(vp->id));
			pthread_mutex_unlock(&engine.job_list_mutex);
			if (job) {
#ifdef DEBUG
				printf("Job found, executing\n");
#endif
				execute_job(job, vp);
				continue;
			}
		}
#ifdef DEBUG
		printf("No jobs found anywhere, sleeping. zzzzz\n");
#endif
		/* if everything else fails, sleep for a while */
		sleep_vp(vp);
		if ((! vp->keepalive) || (! engine.is_running))
			break;
	}
//	printf("Adeus mundo cruel!\n");
	pthread_exit(NULL);
}
/** Função que cria um VP.
@param *startup_job ponteiro para o descritor da tarefa inicial do vp */
pthread_t
create_vp(struct job *startup_job)
{
	struct vp_node *vp;

	vp = (struct vp_node *) calloc (1, sizeof(struct vp_node));
	vp->keepalive   = 1;
	vp->sleeping    = 0;
	vp->startup_job = startup_job;
	sem_init(&vp->sem, 1, 0);
	sem_init(&vp->steal_sem, 1, 0);
	init_list_head(&vp->job_stack, 1);

	if ((list_size(engine.vp_list)) == 0) {
		/* main thread */
		vp->id = pthread_self();
		list_add_tail(&vp->vp_list, &engine.vp_list);
	} else {
		list_add_tail(&vp->vp_list, &engine.vp_list);
		pthread_create(&vp->id, NULL, vp_listen, vp);
	}

	return vp->id;
}
/** Função que destroi com a lista de vps.
@param *vp_list ponteiro para o cabeçalho da lista de vps */
int destroy_vps(struct list_head *vp_list)
{
	struct list_head *pos, *pos_aux;
	
	list_for_each_safe(pos, pos_aux, vp_list) {
		struct vp_node *vp = list_entry(pos, struct vp_node, vp_list);
		
		pthread_mutex_lock(vp_list->mutex);			//** locking
		vp->keepalive = 0;
		sem_post(&vp->sem);

		pthread_mutex_unlock(vp_list->mutex);
		pthread_join(vp->id, NULL);
		pthread_mutex_lock(vp_list->mutex);

		sem_destroy(&vp->sem);
		sem_destroy(&vp->steal_sem);
		destroy_list_head2(&vp->vp_list);
		free(vp);
		pthread_mutex_unlock(vp_list->mutex); //** unlocking
	}
	
	destroy_list_head(vp_list);
	return 0;
}

/** Função que retorna o descritor do vp corrente. */
struct vp_node *current_vp()
{
	pthread_t id;
	struct vp_node *retval = NULL;
	struct list_head *pos;
	
	id = pthread_self();
	
	Pthread_mutex_lock(engine.vp_list.mutex);
	list_for_each(pos, &engine.vp_list) {
		retval = list_entry(pos, struct vp_node, vp_list);
		if (retval->id == id)
			break;
	}
	pthread_mutex_unlock(engine.vp_list.mutex);

	return retval;
}

pthread_t main_vp_id()
{
	pthread_t vp_id = 0;
	struct vp_node *retval = NULL;
	struct list_head *pos;
	
	Pthread_mutex_lock(engine.vp_list.mutex);
	list_for_each(pos, &engine.vp_list) {
		retval = list_entry(pos, struct vp_node, vp_list);
		break;
	}
	pthread_mutex_unlock(engine.vp_list.mutex);

	if (retval)
		vp_id = retval->id;

	return vp_id;
}
/** Função que retorna o descritor da tarefa que esta sendo executada. */
struct job *current_job(struct vp_node *vp)
{
	struct list_head *pos;
	struct job *job = NULL;

	if (! vp)
		return NULL;
	
	list_for_each(pos, &vp->job_stack) {
		job = list_entry(pos, struct job, vp_stack);
		break;
	}
	return job;
}
// :-)
