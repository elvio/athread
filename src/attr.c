/*
 * attr.c
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
 * $Id: attr.c,v 1.2 2006/07/28 13:32:43 yolaf Exp $
 *
 */
#include "anahy/structures.h"
#include <pthread.h>

/** Função que retorna um ponteiro para os atributos da tarefa corrente. 
Retorna nulo caso estrutura inválida.*/
athread_attr_t *athread_get_attr()
{
	struct vp_node *vp = current_vp();
	struct job *job = current_job(vp);
	
	if (job)
		return &job->attribs;
	else
		return NULL;
}

/** Função que inicializa uma estrutura de atributos de uma tarefa com os valores padrão. 
Retorna zero caso sucesso.
@param *attr ponteiro para um athread_attr_t que será inicializado.*/
int athread_attr_init(athread_attr_t *attr)
{
	attr->max_joins = 1;
	attr->initialized = 1;
	attr->detach_state = ATHREAD_CREATE_JOINABLE;
	attr->input_len = -1;
	attr->output_len = -1;
	attr->in_pack_func = NULL;
	attr->in_unpack_func = NULL;
	attr->out_pack_func = NULL;
	attr->out_unpack_func = NULL;
	attr->communication_cost = 0;
	attr->execution_cost = 0;
	attr->splitfactor = 0;
	attr->split = NULL;
	attr->merge = NULL;
	attr->remote_job = NULL;
	return 0;
}

int athread_attr_init_defaults(athread_attr_t *attr)
{
	return athread_attr_init(attr);
}

/** Função que destrói uma estrutura de atributos de uma tarefa. Retorna zero caso sucesso.
@param *attr Ponteiro para a estrutura a ser destruida. */
int athread_attr_destroy(athread_attr_t *attr)
{
	attr->initialized = 0;
	return 0;
}

/** Função que seta o estado detached de uma tarefa. Retorna inválido caso o estado
a ser setado não for válido, zero caso sucesso.
@param *attr ponteiro para a estrutura a ser modificada
@param detach_state estado que será setado */
int athread_attr_setdetachstate(athread_attr_t *attr, int detach_state)
{
	if (! (detach_state & (ATHREAD_CREATE_JOINABLE | ATHREAD_CREATE_DETACHED)))
		return -EINVAL;
	attr->detach_state = detach_state;
	return 0;
}

/** Função que pega o estado detach de uma tarefa. Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura.
@param *detach_state ponteiro para váriavel que armazenará o valor buscado.*/
int
athread_attr_getdetachstate(athread_attr_t *attr, int *detach_state)
{
	*detach_state = attr->detach_state;
	return 0;
}

/** fUNÇÃo que seta o número máximo de joins que serão executados em uma tarefa. 
@param *attr ponteiro para a estrutura a ser modificada. Retorna zero caso sucesso.
@param max_joins número de joins a ser setado. */
int
athread_attr_setjoinnumber(athread_attr_t *attr, int max_joins)
{
	if (max_joins < 1)
		return -EINVAL;
	attr->max_joins = max_joins;
	return 0;
}

/** Função que busca o número máximo de joins que uma tarefa pode receber. 
Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura
@param *max_joins ponteiro para a variável que armazenará o valor buscado. */
int
athread_attr_getjoinnumber(athread_attr_t *attr, int *max_joins)
{
	*max_joins = attr->max_joins;
	return 0;
}

/** Função que seta input length , sei la q q isso faz.
Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura a ser modificada.
@param input_len o valor a ser setado. */
int
athread_attr_setinputlen(athread_attr_t *attr, long input_len)
{
	if (input_len < 1)
		return -EINVAL;
	attr->input_len = input_len;
	return 0;
}

/** Função que busca qual o input length de uma tarefa.
Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura.
@param *input_len ponteiro para a variável que armazenará o resultado buscado. */
int
athread_attr_getinputlen(athread_attr_t *attr, long *input_len)
{
	*input_len = attr->input_len;
	return 0;
}

/** Função que seta o output length de uma tarefa. idem ao input length.
Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura a ser modificada.
@param output_len valor a ser setado. */
int
athread_attr_setoutputlen(athread_attr_t *attr, long output_len)
{
	if (output_len < 1)
		return -EINVAL;
	attr->output_len = output_len;
	return 0;
}

/** Função que busca qual o output length atual de uma tarefa.
Retorna zero caso sucesso.
@param *attr ponteiro para a estrutura.
@param *output_len ponteiro para a variável que armazenará o valor buscado. */
int
athread_attr_getoutputlen(athread_attr_t *attr, long *output_len)
{
	*output_len = attr->output_len;
	return 0;
}

/** funcao que seta qual a funcao de usuario sera usada para empacotar os dados de entrada.
retorna zero quando bem sucedida.
@param *attr ponteiro para a estrutura.
@param func ponteiro para a funcao */
int
athread_attr_pack_in_func(athread_attr_t *attr, pfunc func){
	attr->in_pack_func = func;
	return 0;
}

/** funcao que seta qual a funcao de usuario sera usada para desempacotar os dados de entrada.
retorna zero quando bem sucedida.
@param *attr ponteiro para a estrutura.
@param func ponteiro para a funcao */
int
athread_attr_unpack_in_func(athread_attr_t *attr, pfunc func){
	attr->in_unpack_func = func;
	return 0;
}

/** funcao que seta qual a funcao de usuario sera usada para empacotar os dados de saida.
retorna zero quando bem sucedida.
@param *attr ponteiro para a estrutura.
@param func ponteiro para a funcao */
int
athread_attr_pack_out_func(athread_attr_t *attr, pfunc func){
	attr->out_pack_func = func;
	return 0;
}

/** funcao que seta qual a funcao de usuario sera usada para desempacotar os dados de saida.
retorna zero quando bem sucedida.
@param *attr ponteiro para a estrutura.
@param func ponteiro para a funcao */
int
athread_attr_unpack_out_func(athread_attr_t *attr, pfunc func){
	attr->out_unpack_func = func;
	return 0;
}

/** funcao que seta o tempo estimado de computacao de uma tarefa
@param *attr ponteiro para a estrutura
@param cost  custo estimado */
int
athread_attr_execution_cost(athread_attr_t *attr, int cost){
	attr->execution_cost = cost;
	return 0;
}

/** funcao que seta o tempo estimado de comunicacao de uma tarefa
@param *attr ponteiro para a estrutura
@param cost  custo estimado */
int 
athread_attr_communication_cost(athread_attr_t *attr, int cost){
	attr->communication_cost = cost;
	return 0;
}

//
/** funcao que seta o número de threads em que vai ser splitada uma função
@param *attr ponteiro para a estrutura
@param n número de threads */

int
athread_attr_set_splitfactor(athread_attr_t *attr, int n){
	attr->splitfactor = n;
	return 0;
}

/** funcao que seta o ponteiro para a função split
@param *attr ponteiro para a estrutura
@param split ponteiro pra função split */
int
athread_attr_set_splitfunction(athread_attr_t *attr, void *(* split)(void *, int, size_t, int)){
	attr->split = split;
	return 0;
}


/** funcao que seta o ponteiro para a função merge
@param *attr ponteiro para a estrutura
@param merge ponteiro pra função merge */
int
athread_attr_set_mergefunction(athread_attr_t *attr, void *(* merge)(void *, int, int, void *)){
	attr->merge = merge;
	return 0;
}

/** funcao que seta o tamanho do retorno do job
@param *attr ponteiro para a estrutura
@param inputsize tamanho da entrada */
int athread_attr_set_inputsize(athread_attr_t *attr, size_t inputsize){
	attr->inputsize = inputsize;
	return 0;
}


/** funcao que seta o tamanho do retorno do job
@param *attr ponteiro para a estrutura
@param returnsize tamanho do retorno */
int athread_attr_set_returnsize(athread_attr_t *attr, size_t returnsize){
	attr->returnsize = returnsize;
	return 0;
}




// remote content
// ====================
// struct remote_job {
// 	int remote_weight;
// 	int executing;
// 	time_t started_at;
// 	int data_type;
// 	size_t data_size;
// 	char *label;
// 	int return_data_type;
// 	size_t return_data_size;
// 	pthread_mutex_t lock;
// };


/** funcao para habilitar uma thread para executar remotamente **/
int athread_attr_set_remote_ability(athread_attr_t *attr, int status) {
	struct remote_job *new_remote_job;
	if (status == 1) {
		new_remote_job = malloc(sizeof(struct remote_job));
		new_remote_job->remote_weight = 0;
		new_remote_job->executing = 0;
		pthread_mutex_init(&new_remote_job->lock, NULL);
		attr->remote_job = new_remote_job; 
	}
}
