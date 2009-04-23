/*
 * anahy/list.h
 * ------------
 * 
 * Copyright (C) 2004 by the Anahy Project
 * Copyright (C) -infinitum-2003 by the Linux Kernel
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
#ifndef _MODINITTOOLS_LIST_H
#define _MODINITTOOLS_LIST_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @param ptr	 the pointer to the member.
 * @param type	 the type of the container struct this is embedded in.
 * @param member the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *next, *prev, *head;
	pthread_mutex_t *mutex;
	unsigned int *len;
};


#define LIST_HEAD_INIT(name) { &(name), &(name), NULL }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define list_size(name) (*name.len)

static inline void init_list_head(struct list_head *ptr, char initialize_mutex)
{
	ptr->next = ptr; 
	ptr->prev = ptr;
	ptr->head = ptr;
	ptr->len = (unsigned int *) malloc (sizeof (unsigned int));
	*ptr->len = 0;

	if (initialize_mutex) {
		ptr->mutex = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
		pthread_mutex_init (ptr->mutex, NULL);
	//	printf ("inicializado mutex na lista %p para %p\n", ptr, ptr->mutex);
	} else {
		ptr->mutex = NULL;
	}
}

static inline void destroy_list_head(struct list_head *ptr)
{
	/*if (ptr->mutex) {
		pthread_mutex_destroy (ptr->mutex);
		free (ptr->mutex);
		ptr->mutex = NULL;
	}*/
	ptr->next = ptr->prev = ptr->head = NULL;
	if (ptr->len) {
		free (ptr->len);
		ptr->len = NULL;
	}
//
	if (ptr->mutex) {
		pthread_mutex_destroy (ptr->mutex);
		free (ptr->mutex);
		ptr->mutex = NULL;
	}
//
}

static inline void destroy_list_head2(struct list_head *ptr)
{
	ptr->next = ptr->prev = ptr->head = NULL;
}


/**
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new_entry,
			      struct list_head *prev,
			      struct list_head *next)
{
	if (next->mutex)
		pthread_mutex_lock(next->mutex);
	next->prev = new_entry;
	new_entry->next  = next;
	new_entry->prev  = prev;
	new_entry->mutex = next->mutex;
	new_entry->head  = next->head;
	new_entry->len = next->len;
	(*new_entry->len)++;
	prev->next = new_entry;
	if (next->mutex)
		pthread_mutex_unlock(next->mutex);
}

/**
 * list_add - add a new entry
 * @param *new_entry new entry to be added
 * @param *head list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new_entry, struct list_head *head)
{
	__list_add(new_entry, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @param *new_entry the new entry to be added
 * @param *head list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *new_entry, struct list_head *head)
{
	__list_add(new_entry, head->prev, head);
}

/**
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	(*prev->len)--;
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @param *entry the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @param *entry the element to delete from the list.
 * @param initialize_mutex    the mutex value 
 */
static inline void list_del_init(struct list_head *entry, int initialize_mutex)
{
	__list_del(entry->prev, entry->next);
	init_list_head(entry, initialize_mutex); 
}

/**
 * list_move - delete from one list and add as another's head
 * @param *list the entry to move
 * @param *head the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @param *list the entry to move
 * @param *head the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}

/**
 * list_empty - tests whether a list is empty
 * @param *head the list to test.
 */
static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}
#if 0
static inline void __list_splice(struct list_head *list,
				 struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;
	
	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

/**
 * list_splice - join two lists
 * @param *list  the new list to add.
 * @param *head  the place to add it in the first list.
 */
static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @param *list  the new list to add.
 * @param *head  the place to add it in the first list.
 *
 * The list at *list is reinitialised
 */
static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		init_list_head(list);
	}
}
#endif

/**
 * list_entry - get the struct for this entry
 * @param ptr 	    the &struct list_head pointer.
 * @param type 	    the type of the struct this is embedded in.
 * @param member 	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_for_each	-	iterate over a list
 * @param pos 	the &struct list_head to use as a loop counter.
 * @param head 	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @param pos 	the &struct list_head to use as a loop counter.
 * @param n 	another &struct list_head to use as temporary storage
 * @param head 	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @param pos 	the &struct list_head to use as a loop counter.
 * @param head 	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)
        	
/**
 * list_for_each_prev_safe - iterate over a list backwards safe against 
 * removal of list entry
 *
 * @param pos 	the &struct list_head to use as a loop counter.
 * @param n 	another &struct list_head to use as temporary storage
 * @param head 	the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; pos != (head); \
		pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @param pos 	    the type * to use as a loop counter.
 * @param head 	    the head for your list.
 * @param member 	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#ifdef __cplusplus
}
#endif
	
#endif
