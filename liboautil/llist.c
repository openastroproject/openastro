/*****************************************************************************
 *
 * llist.c -- double linked list
 *
 * Copyright 2015 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <pthread.h>

#include <oa_common.h>
#include <openastro/util.h>

struct dl_list_object;
typedef struct dl_list_object	DL_LIST_OBJECT;

struct dl_list_object {
  DL_LIST_OBJECT*	prev;
  DL_LIST_OBJECT*	next;
  void*			data;
};


/*
 * Create the list meta object and zero the contents to indicate an
 * empty list
 *
 * Returns the list meta object or 0 if memory could not be allocated
 */

DL_LIST
oaDLListCreate ( void )
{
  DL_LIST	list;

  if (!( list = malloc ( sizeof ( struct dl_list )))) {
    return 0;
  }

  list->head = list->tail = 0;
  list->length = 0;
  pthread_mutex_init ( &list->listMutex, 0 );
  return list;
}


/*
 * Add an item to the head of the list.
 *
 * Returns 0 for success or an OA_ERR_* value for an error
 */

int
oaDLListAddToHead ( DL_LIST list, void* data )
{
  DL_LIST_OBJECT*	object;
  DL_LIST_OBJECT*	prevHead;

  if (!( object = malloc ( sizeof ( DL_LIST_OBJECT )))) {
    return -OA_ERR_MEM_ALLOC;
  }

  object->data = data;
  object->prev = 0;
  pthread_mutex_lock ( &list->listMutex );
  if ( list->head ) {
    prevHead = list->head;
    prevHead->prev = object;
  }
  object->next = list->head;
  list->head = object;
  if ( !list->length ) {
    list->tail = list->head;
  }
  list->length++;
  pthread_mutex_unlock ( &list->listMutex );
  return 0;
}
  

/*
 * Add an item to the tail of the list.
 *
 * Returns 0 for success or an OA_ERR_* value for an error
 */

int
oaDLListAddToTail ( DL_LIST list, void* data )
{
  DL_LIST_OBJECT*	object;
  DL_LIST_OBJECT*	prevTail;

  if (!( object = malloc ( sizeof ( DL_LIST_OBJECT )))) {
    return -OA_ERR_MEM_ALLOC;
  }

  object->data = data;
  object->prev = list->tail;
  object->next = 0;
  pthread_mutex_lock ( &list->listMutex );
  if ( list->tail ) {
    prevTail = list->tail;
    prevTail->next = object;
  }
  list->tail = object;
  if ( !list->length ) {
    list->head = list->tail;
  }
  list->length++;
  pthread_mutex_unlock ( &list->listMutex );
  return 0;
}


/*
 * Remove an item from the head of the list.  Frees the linked list
 * item at the same time.
 *
 * Returns a pointer to the data or 0 if there is none
 */

void*
oaDLListRemoveFromHead ( DL_LIST list )
{
  DL_LIST_OBJECT*	object;
  DL_LIST_OBJECT*	newHead;
  void*			data = 0;

  pthread_mutex_lock ( &list->listMutex );
  if ( list->length ) {
    object = list->head;
    if (( list->head = object->next )) {
      newHead = list->head;
      newHead->prev = 0;
    }
    if (!( --list->length )) {
      list->tail = 0;
    }
    data = object->data;
    free ( object );
  }
  pthread_mutex_unlock ( &list->listMutex );
  return data;
}


/*
 * Remove an item from the tail of the list.  Frees the linked list
 * item at the same time.
 *
 * Returns a pointer to the data or 0 if there is none
 */

void*
oaDLListRemoveFromTail ( DL_LIST list )
{
  DL_LIST_OBJECT*	object;
  DL_LIST_OBJECT*	newTail;
  void*                 data = 0;

  pthread_mutex_lock ( &list->listMutex );
  if ( list->length ) {
    object = list->tail;
    if (( list->tail = object->prev )) {
      newTail = list->tail;
      newTail->next = 0;
    }
    if (!( --list->length )) {
      list->head = 0;
    }
    data = object->data;
    free ( object );
  }
  pthread_mutex_unlock ( &list->listMutex );
  return data;
}


/*
 * Delete a list -- free the list meta object, having freed all the list
 * objects if there are any.  If freeData is non-zero then the data pointer
 * is freed too.
 */

void
oaDLListDelete ( DL_LIST list, int freeData )
{
  DL_LIST_OBJECT*	object;
  DL_LIST_OBJECT*	next;

  if ( list->length ) {
    object = list->head;
    while ( object ) {
      next = object->next;
      if ( freeData ) {
        free ( object->data );
      }
      free ( object );
      object = next;
    }
  }
  free ( list );
}


/*
 * Check to see if a list is empty.
 */

int
oaDLListIsEmpty ( DL_LIST list )
{
  int		len;

  pthread_mutex_lock ( &list->listMutex );
  len = list->length;
  pthread_mutex_unlock ( &list->listMutex );

  return len ? 0 : -1;
}


/*
 * Peek at an item at a given position in the list.  Doesn't remove
 * anything from the list, just returns a pointer to the data.
 *
 * Returns a pointer to the data or 0 if there is none
 */

void*
oaDLListPeekAt ( DL_LIST list, int position )
{
  DL_LIST_OBJECT*       object;
  void*                 data = 0;

  pthread_mutex_lock ( &list->listMutex );
  if ( list->length ) {
    object = list->head;
    while ( position-- && object ) {
      object = object->next;
    }

    if ( object ) {
      data = object->data;
    }
  }
  pthread_mutex_unlock ( &list->listMutex );

  return data;
}


/*
 * Remove an item from a given position in the list.  Frees the
 * linked list item at the same time.
 *
 * Returns a pointer to the data or 0 if there is none
 */

void*
oaDLListRemoveAt ( DL_LIST list, int position )
{
  DL_LIST_OBJECT*       object;
  void*                 data = 0;

  pthread_mutex_lock ( &list->listMutex );
  if ( list->length ) {
    object = list->head;
    while ( position-- && object ) {
      object = object->next;
    }

    if ( object ) {
      if ( object->next ) {
        object->next->prev = object->prev;
      }
      if ( object->prev ) {
        object->prev->next = object->next;
      }
      if ( list->head == object ) {
        list->head = object->next;
      }
      if ( list->tail == object ) {
        list->tail = object->prev;
      }
      list->length--;

      data = object->data;
      free ( object );
    }
  }
  pthread_mutex_unlock ( &list->listMutex );
  return data;
}
