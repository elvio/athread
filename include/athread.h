/*
 * athread.h
 * ---------
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
#ifndef _athread_h
#define _athread_h 1

#ifdef __cplusplus
extern "C" {
#endif

#include "anahy/structures.h"

/* engine control */
int aInit(int *argc, char ***argv);
int aRemoteInit(int argc, char **argv);
int aRemoteTerminate();
int aTerminate();
void aSearchFrom(int algorithm);

/* basic thread handling functions */
int athread_create(athread_t *id, athread_attr_t *attribs, pfunc function, void *data);
int athread_join(athread_t id, void **return_data);
double athread_join_double(athread_t id);
#define athread_exit(data) return((void *)data)
athread_t athread_self();

/* access to thread attributes */
athread_attr_t *athread_get_attr();
int athread_attr_init(athread_attr_t *attr);
int athread_attr_destroy(athread_attr_t *attr);
int athread_attr_setdetachstate(athread_attr_t *attr, int detach_state);
int athread_attr_getdetachstate(athread_attr_t *attr, int *detach_state);
int athread_attr_setjoinnumber(athread_attr_t *attr, int max_joins);
int athread_attr_getjoinnumber(athread_attr_t *attr, int *max_joins);
int athread_attr_setinputlen(athread_attr_t *attr, long input_len);
int athread_attr_getinputlen(athread_attr_t *attr, long *input_len);
int athread_attr_setoutputlen(athread_attr_t *attr, long output_len);
int athread_attr_getoutputlen(athread_attr_t *attr, long *output_len);
int athread_attr_pack_in_func(athread_attr_t *attr, pfunc func);
int athread_attr_unpack_in_func(athread_attr_t *attr, pfunc func);
int athread_attr_pack_out_func(athread_attr_t *attr, pfunc func);
int athread_attr_unpack_out_func(athread_attr_t *attr, pfunc func);

#ifdef __cplusplus
}
#endif
	
#endif /* _athread_h */
