
#include <stdlib.h>
#include <assert.h>
#include "list.h"

/*
 * Allocate
 */
list * 
list_alloc ()
{
  return (list *)malloc (sizeof (list));
}

/*
 * Initialize.
 */
int 
list_init (list * l)
{
  assert (l);
  l->begin = (list_node *)l;
  l->rbegin = (list_node *)l;
  l->end = (list_node *)l;
  l->rend = (list_node *)l;
  return 0;
}

/*
 * Allocate and initialize.
 */
list * 
list_new ()
{
  list * l = list_alloc ();
  if (l && list_init (l))
    {
      list_delete (l);
      return NULL;
    }
  return l;
}

/*
 * Terminate.
 */ 
void 
list_end (list * l)
{
}

/*
 * Terminate and free.
 */
void 
list_delete (list * l)
{
  free (l);
}


/**
 * @brief Clear the list.
 */
void 
list_clear (list * l)
{
  list_end (l);
  list_init (l);
}

/*
 * Return true if empty.
 */
int
list_empty (const list * l)
{
  return l->begin == l->end;
}

/*
 * Push element at the front.
 */
void 
list_push_front (list * l, list_node * x)
{
  x->prev = l->end; 
  x->next = l->begin; 
  l->begin->prev = x; 
  l->begin = x; 
}

/*
 * Push element at the back.
 */
void
list_push_back (list * l, list_node * x)
{
  x->next = l->rend; 
  x->prev = l->rbegin; 
  l->rbegin->next = x; 
  l->rbegin = x; 
}

/*
 * Pop element at the front.
 */
void
list_pop_front (list * l)
{
  if (!list_empty (l)) 
    { 
      l->begin = l->begin->next; 
      l->begin->prev = l->end; 
    } 
}

/*
 * Pop element at the back.
 */
void
list_poph_back (list * l)
{
  if (!list_empty (l)) 
    { 
      l->rbegin = l->rbegin->prev; 
      l->rbegin->next = l->rend; 
    } 
}

/*
 * Remove element from list.
 */
void
list_remove (list * l, list_node * x)
{
  x->prev->next = x->next; 
  x->next->prev = x->prev; 
}
