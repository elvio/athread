/*
 * jobs.c
 * ------
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
 */
#include "anahy/structures.h"
#include "anahy/macros.h"

int remove_job = 1;

/* -- jobs -- */
#define wait_job_execution(job) \
	pthread_mutex_lock(&job->mutex); \
	while ((job->status == JOB_ASSIGNED) || (job->status == JOB_EXECUTING) || (job->status == JOB_UNASSIGNED)) { \
		pthread_cond_wait(&job->cond, &job->mutex); \
	} \
	pthread_mutex_unlock(&job->mutex);

#define wait_to_be_taken(job) \
	pthread_mutex_lock(&job->steal_mutex); \
	while (! job->node.is_remote) { \
		pthread_cond_wait(&job->steal_cond, &job->steal_mutex); \
	} \
	pthread_mutex_unlock(&job->steal_mutex);

struct job *search_jobs(int match_condition, struct list_head *job_list, athread_t *job_id, pthread_t *vp_id)
{
	struct list_head *pos;
	struct job *retval;
	int condicao = 0;
	
	pthread_mutex_lock(job_list->mutex);		

	if( *(job_list->len) == 0){
		pthread_mutex_unlock(job_list->mutex);//
		return NULL;
	}

	list_for_each_prev(pos, job_list) {
		struct job *entry = list_entry(pos, struct job, job_list);

		if ( ! entry ) {
			pthread_mutex_unlock(job_list->mutex);
			return NULL;
		}
		
		//pthread_mutex_lock(job_list->mutex);
		
		switch (match_condition){
		case MATCH_JOB_ID:
			 //pthread_mutex_unlock(job_list->mutex);
			 condicao = (entry->id == *job_id);
			 break;
	 	case UNASSIGNED_JOB:
			 if (entry->status == JOB_UNASSIGNED) {
					entry->status = JOB_ASSIGNED;
					entry->owner  = *vp_id;
					condicao = 1;
			 }
			 //pthread_mutex_unlock(job_list->mutex);
			 break;
		case SCHEDULING_REMOTE:
			 	if (entry->status == JOB_UNASSIGNED && entry->attribs.input_len != -1 && ( (entry->attribs.in_pack_func != NULL) && 
					(entry->attribs.in_unpack_func != NULL) && (entry->attribs.out_pack_func != NULL) && (entry->attribs.out_unpack_func != NULL))) {
					entry->status = JOB_ASSIGNED;
					entry->owner  = *vp_id;
					condicao = 1;
				}
				//pthread_mutex_unlock(job_list->mutex);
			 break;
		}
		
		if (condicao) {
			pthread_mutex_unlock(job_list->mutex);
			return entry;
		}

		if ((list_size(entry->child_list)) > 0) {
			retval = search_jobs(match_condition, &entry->child_list, job_id, vp_id);
			if ( retval ) {
				pthread_mutex_unlock(job_list->mutex);
				return retval;
			}
		}
	}
	pthread_mutex_unlock(job_list->mutex);
	return NULL;
}

void help_to_execute(struct job *job, struct vp_node *vp)
{
	struct job *new_job = NULL;
	struct job *curr_job = NULL;

	switch (engine.search_algorithm) {
		case GRAPH_CURR_THREAD:
			curr_job = current_job(vp);
			if (! curr_job)
				break;
			new_job = search_jobs(UNASSIGNED_JOB, &curr_job->child_list, NULL, &(vp->id));
			break;	
		case GRAPH_JOINED_THREAD:
			new_job = search_jobs(UNASSIGNED_JOB, &job->child_list, NULL, &(vp->id));
			break;
		case GRAPH_ROOT:
		default:
			new_job = search_jobs(UNASSIGNED_JOB, &engine.job_list, NULL, &(vp->id));
			break;
	}
	
	if (new_job) {
		execute_job(new_job, vp);
		return;
	}
	
	/* fallback to default: search from the graph's root again */
	new_job = search_jobs(UNASSIGNED_JOB, &engine.job_list, NULL, &(vp->id));
	if (new_job)
		execute_job(new_job, vp);
}

int destroy_jobs(struct list_head *job_list)
{
	static int retval = 0;
	static int n_jobs = 0;
	struct list_head *pos, *n;
	
//	Pthread_mutex_lock(job_list->mutex);
	list_for_each_prev_safe(pos, n, job_list) {
		struct job *entry = list_entry(pos, struct job, job_list);
		if (! entry)
			break;
		
		if ((list_size(entry->child_list)) > 0) {
			destroy_jobs(&entry->child_list);
		}

		list_del(&entry->job_list);
		pthread_mutex_destroy(&entry->mutex);
		pthread_cond_destroy(&entry->cond);
		if (entry->status != JOB_JOINED) {
			retval = -1;
			printf("[%ld] destroy_jobs => status %03lld = %ld\n", pthread_self(), entry->id, entry->status);
		}
		n_jobs++;
		free(entry);
	}
//	pthread_mutex_unlock(job_list->mutex);
	destroy_list_head(job_list);
	return retval;
}

athread_t new_job_id()
{
	athread_t retval;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	
	Pthread_mutex_lock(&mutex);
	retval = engine.new_id++;
	pthread_mutex_unlock(&mutex);

	return retval;
}

void store_on_joblist(struct job *job)
{
	struct list_head *pos;
	struct vp_node *vp;
	
	vp = current_vp();
	if (! vp) {
		list_add_tail(&job->job_list, &engine.job_list);
		return;
	}
	
	list_for_each(pos, &vp->job_stack) {
		struct job *entry = list_entry(pos, struct job, vp_stack);
		list_add_tail(&job->job_list, &entry->child_list);
		return;
	}
	
	list_add_tail(&job->job_list, &engine.job_list);
}

/** Função que cria uma nova tarefa anahy.
@param *id ponteiro para um identificador de tarefa.
@param *attribs ponteiro para um identificador de atributos pertencentes a tarefa.
@param function função que será executada pela tarefa.
@param *data ponteiro para os dados de entrada da tarefa.
*/
int athread_create(athread_t *id, athread_attr_t *attribs, pfunc function, void *data)
{
	struct job *job;
	athread_t *vsplit;
	int i, nt;

	if (athread_remote_rank != 0 && attribs && attribs->remote_job) {
		printf("preventing slave to create a remote job\n");
		return 0;
	}
	
	if ((!attribs) || (attribs->splitfactor <= 0)) nt = 0;
	else nt = attribs->splitfactor - 1;

	for (i=0; i <= nt; i++){
		job = (struct job *) calloc (1, sizeof(struct job));
		if (! job) {
			perror ("calloc");
			return -ENOMEM;
		}

		if (attribs) {
			if (attribs->initialized != 1) {
				fprintf(stderr, "error: attributes must be initialized with athread_attr_init\n");
				athread_attr_init_defaults(&job->attribs);
			} else {
				memcpy(&job->attribs, attribs, sizeof(athread_attr_t));
			}
		} else {
			athread_attr_init_defaults(&job->attribs);
		}
		
		job->id = new_job_id();
		job->function = function;

		if ((attribs) && (attribs->splitfactor > 0)){
			if (i == 0){
				vsplit = (athread_t *) malloc (attribs->splitfactor*sizeof(athread_t));
				*id = job->id;
			}
			vsplit[i] = job->id;
			job->vetsplit = vsplit;
		}
		else *id = job->id;

		if ((attribs) && (attribs->splitfactor > 0)) job->data = attribs->split(data, attribs->splitfactor, attribs->inputsize, i);
		else job->data = data;

		job->owner = -1;
		pthread_mutex_init(&job->mutex, NULL);
		pthread_mutex_init(&job->reply_mutex, NULL);
		pthread_mutex_init(&job->steal_mutex, NULL);
		pthread_cond_init(&job->cond, NULL);
		pthread_cond_init(&job->reply_cond, NULL);
		pthread_cond_init(&job->steal_cond, NULL);
		init_list_head(&job->child_list, 1);

		if (attribs->remote_job) {
			job->status = JOB_ASSIGNED;
			store_on_joblist(job);
			athread_remote_send_job(job);
		} else if (list_size(engine.vp_list) < engine.max_vps) {
			job->status = JOB_ASSIGNED;
			store_on_joblist(job);
			job->owner = create_vp(job);
		} else {
			job->status = JOB_UNASSIGNED;
			job->owner  = 0;
			store_on_joblist(job);
		}
	}

	// mostra informacoes dos jobs criados
	//job_list_info();

	return 0;
}

athread_t athread_self()
{
	struct vp_node *vp = current_vp();
	struct job *job = current_job(vp);
	if (! job) {
		printf("error: could not get current job from vp %ld\n", pthread_self());
		return -1;
	}
	return job->id;
}

/** Função que da merge em duas listas */
void merge_lists(struct list_head *new_list, struct list_head *old_list)
{
	struct list_head *pos;
	struct list_head *newlist_last  = new_list->prev;
	struct list_head *oldlist_first = old_list->next;
	
	list_for_each(pos, old_list) {
		pos->mutex = new_list->mutex;
		pos->head = new_list->head;
		pos->len = new_list->len;
	}

	newlist_last->next = oldlist_first;
	new_list->prev = old_list->prev;
	oldlist_first->prev = newlist_last;
	old_list->prev = new_list;
}

/** Função que remove uma tarefa em específico da lista.
@param *job ponteiro para o descritor da tarefa */
void remove_from_list(struct job *job)
{
	//pthread_mutex_t *mutex;
	
	//mutex = job->job_list.mutex;
	//pthread_mutex_lock(mutex);

	Pthread_mutex_lock(job->child_list.mutex);
	if ((list_size(job->child_list)) > 0) {
		//pthread_mutex_lock(job->job_list.mutex);
		merge_lists(&job->job_list, &job->child_list);
		//pthread_mutex_unlock(job->job_list.mutex);
	}
	pthread_mutex_unlock(job->child_list.mutex);
//
//mutex = job->child_list.mutex;
//
	destroy_list_head(&job->child_list);//AQUI!

//pthread_mutex_unlock(mutex);//

	pthread_mutex_lock(&job->mutex);//1
	pthread_cond_broadcast(&job->cond);
	pthread_mutex_unlock(&job->mutex);//2

	pthread_mutex_destroy(&job->mutex);
	pthread_cond_destroy(&job->cond);
	list_del(&job->job_list);

	free(job);
	//pthread_mutex_unlock(mutex);
}
/** Função que sincroniza com uma tarefa.
@param id descritor da tarefa com a qual se quer sincronizar.
@param **return_data ponteiro para onde o resultado obtido será armazenado */
int athread_join(athread_t id, void **return_data)
{
	struct vp_node *vp;
	struct job *stacked_job;
	struct job *job;
	pthread_t vp_id;
	int count, nt;
	pthread_mutex_t *mutex;
  pthread_mutex_t merge_lock = PTHREAD_MUTEX_INITIALIZER;

	count = 0;
	do {
		job = NULL;

		vp = current_vp();
		if (! vp) {
			vp_id = pthread_self();
		} else {
			vp_id = vp->id;
		}

		stacked_job = current_job(vp);
		if ((stacked_job != NULL) && (vp != NULL))
			job = search_jobs(MATCH_JOB_ID, &stacked_job->child_list, &id, &vp_id);

		if (job == NULL)
			job = search_jobs(MATCH_JOB_ID, &engine.job_list, &id, &vp_id);

		if ( job == NULL) {
			fprintf(stderr, "could not find job %lld on the job list\n", id);
			return -ESRCH;
		}
		
		// Wow.. what about join a remote job Beavis?
		if (athread_remote_rank == 0 && job->attribs.remote_job) {
			printf("[s] slave --- got a join call. Time to request some data hun?\n");
			
			result = request_result_from_slave((job->attribs.remote_job)->slave);
			printf("[m] master --- got result from slave #%d --- result == %2.2f\n", remote_job_input->slave, result);
			mark_slave_as_fresh((job->attribs.remote_job)->slave);
			return 0;
		}

		Pthread_mutex_lock(job->job_list.mutex);
		switch (job->status) {
			case JOB_UNASSIGNED:
				job->status = JOB_ASSIGNED;
				job->owner = vp_id;
				pthread_mutex_unlock(job->job_list.mutex);
				execute_job(job, vp);
				break;
			case JOB_ASSIGNED:
				pthread_mutex_unlock(job->job_list.mutex);
				if (job->owner == vp_id) {
					execute_job(job, vp);
					break;
				}
			case JOB_EXECUTING:
				pthread_mutex_unlock(job->job_list.mutex);
				help_to_execute(job, vp);
				wait_job_execution(job);
				break;
			case JOB_DONE:
				pthread_mutex_unlock(job->job_list.mutex);
				break;
			case JOB_JOINED:
				pthread_mutex_unlock(job->job_list.mutex);
				break;
			default:
				pthread_mutex_unlock(job->job_list.mutex);
		}

		Pthread_mutex_lock(job->job_list.mutex);
		job->status = JOB_JOINED;
		if (return_data){
			nt = job->attribs.splitfactor;
			if (nt){
				int i = 0;
				
        while ((job->vetsplit[i] != job->id) && (i < job->attribs.splitfactor)) {
          i++; 
        }

				if (i < job->attribs.splitfactor) {
          pthread_mutex_lock(&merge_lock);
          //*return_data = job->attribs.merge(job->retval, job->attribs.splitfactor, i, return_data);
          *return_data = job->attribs.merge(job->retval, job->attribs.splitfactor, i, return_data);
          pthread_mutex_unlock(&merge_lock);
        }

				if (count == job->attribs.splitfactor){
					id = job->vetsplit[0]; //restaura id.
					free(job->vetsplit);
				}
				else id = job->vetsplit[i+1];

      // if not using scm, then return job->retval
      } else { 
        printf("Retornando job->retval\n");
        *return_data = job->retval;
      }
		}

		mutex = job->job_list.mutex;

		job->attribs.max_joins--;
		if (job->attribs.max_joins == 0 && remove_job) {
			remove_from_list(job);
		}

	pthread_mutex_unlock(mutex);

		count++;

	} while (count < nt);
	
	return 0;
}

/* ------- */
/** Função que busca o resultado de uma tarefa.
@param id descritor da tarefa com a qual se quer sincronizar.
@param **return_data ponteiro para onde o resultado obtido será armazenado */
int
athread_fetch(athread_t id, void **return_data)
{
	struct vp_node *vp;
//	struct job *stacked_job;
	struct job *job;
	pthread_t vp_id;
	int count, nt;

	count = 0;

	do{
		job = NULL;

		//vp = current_vp();
		//if (! vp) {
		//	printf("opa nao eh um vp\n");
			//return -EINVAL;
			vp_id = pthread_self();
		//	printf("retornando id: %d\n",vp_id);
		//} else {
			vp_id = vp->id;
		//}

		//stacked_job = current_job(vp);
		//if ((stacked_job != NULL) && (vp != NULL))
		//	job = search_jobs(&match_job_id, &stacked_job->child_list, id, vp_id);
		if ( job == NULL)
		job = search_jobs(MATCH_JOB_ID, &engine.job_list, &id, &vp_id);
		if ( job == NULL) {
			fprintf(stderr, "could not find job %lld on the job list\n", id);
			return -ESRCH;
		}

		Pthread_mutex_lock(job->job_list.mutex);
		switch (job->status) {
			case JOB_UNASSIGNED:
				pthread_mutex_unlock(job->job_list.mutex);
				wait_job_execution(job);
				break;
			case JOB_ASSIGNED:
			case JOB_EXECUTING:
				pthread_mutex_unlock(job->job_list.mutex);
				wait_job_execution(job);
				break;
			case JOB_DONE:
				pthread_mutex_unlock(job->job_list.mutex);
				break;
			case JOB_JOINED:
				pthread_mutex_unlock(job->job_list.mutex);
				break;
			default:
				pthread_mutex_unlock(job->job_list.mutex);
		}

		// locking
		Pthread_mutex_lock(job->job_list.mutex);

		job->status = JOB_JOINED;
		if (return_data){
			nt = job->attribs.splitfactor;
			if (nt){
				int i = 0;
				
				while ((job->vetsplit[i] != job->id) && (i < job->attribs.splitfactor)){ i++; }

				if (i < job->attribs.splitfactor)
					*return_data = job->attribs.merge(job->retval, job->attribs.splitfactor, i, *return_data);
				if (count == job->attribs.splitfactor){
					id = job->vetsplit[0]; //restaura id.
					free(job->vetsplit);
				}
				else id = job->vetsplit[i+1];
			}
			else *return_data = job->retval;
		}

		// unlocking
		pthread_mutex_unlock(job->job_list.mutex);

		job->attribs.max_joins--;
		if (job->attribs.max_joins == 0) {
			//remove_from_list(job);
		}

		count++;
	} while (count < nt);
	
	return 0;
}

int job_has_remote_ability(struct job *job) {
	if (job && job->attribs.remote_job != NULL) {
		return 1;
	}
		
	return 0;
}
