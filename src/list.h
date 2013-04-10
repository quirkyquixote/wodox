/*
 * Dot engime.
 * Copyright (c) Luis Sanz 2010 
 */

/**
 * @addtogroup Game 
 * @{
 *
 * @defgroup list List
 *
 * @brief A doubly linked list.
 *
 * This list is made to work with objects whose first field is a list_node.
 *
 * Note that no function in this file will neither allocate nor free a
 * list_node. That must be done by the user.
 *
 * @{
 *
 * @file
 */

#pragma once

/**
 * @brief Node of a doubly linked list.
 */
typedef struct list_node
{
  struct list_node * next;	///< Pointer to the next object.
  struct list_node * prev;	///< Pointer to the previous object.
}
list_node;

/**
 * @brief Doubly linked list.
 */
typedef struct list
{
  list_node * begin;	///< Pointer to the first element.
  list_node * rbegin;	///< Pointer to the last element.
  list_node * end;	///< Pointer to the dummy node after the last element.
  list_node * rend;	///< Pointer to the dummy node before the first element.
}
list;

/**
 * @brief Allocate
 */
list * list_alloc ();

/**
 * @brief Initialize.
 */
int list_init (list * l);

/**
 * @brief Allocate and initialize.
 */
list * list_new ();

/**
 * @brief Terminate.
 */ 
void list_end (list * l);

/**
 * @brief Terminate and free.
 */
void list_delete (list * l);

/**
 * @brief Clear the list.
 */
void list_clear (list * l);

/**
 * @brief Return true if empty.
 */
int list_empty (const list * l);

/**
 * @brief Push element at the front.
 */
void list_push_front (list * l, list_node * n);

/**
 * @brief Push element at the back.
 */
void list_push_back (list * l, list_node * n);

/**
 * @brief Pop element at the front.
 */
void list_pop_front (list * l);

/**
 * @brief Pop element at the back.
 */
void list_poph_back (list * l);

/**
 * @brief Remove element from list.
 */
void list_remove (list * l, list_node * n);

/** @} */
/** @} */
