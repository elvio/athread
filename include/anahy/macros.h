/*
 * macros.h
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
 */
#ifndef _macros_h
#define _macros_h 1

#ifdef __cplusplus
extern "C" {
#endif
	
#include "anahy/structures.h"
#include "config.h"

#ifdef CONFIG_SMP
#define Pthread_mutex_lock(m) pthread_mutex_lock(m)
/*#define Pthread_mutex_lock(m) ({ \
	int ret; \
	do { ret = pthread_mutex_trylock(m); } while (ret == -EBUSY); \
})*/
#else
#define Pthread_mutex_lock(m) pthread_mutex_lock(m)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _macros_h */
