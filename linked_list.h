/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/28/2014 10:23:15 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef LIST_H_
#define LIST_H_

#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

#include "common.h"

/*
 * The definition has been stolen from the Linux kernel.
 */
#ifdef __GNUC__
#  define list_container_of(node, type, member) ({			\
	const struct list_node *__mptr = (node);			\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
#  define list_container_of(node, type, member)			\
	((type *)((char *)(node) - offsetof(type, member)))
#endif	/* __GNUC__ */

/**
 * @brief  List node
 */
struct list_node {
	struct list_node * prev;
	struct list_node * next;
	uintptr_t parent;
};

/**
 * @brief  Init linked list node
 *
 * @param list node to init
 */
inline void list_init(struct list_node * list) {
	list->prev = NULL;
	list->next = NULL;
}

/**
 * @brief  Insert node to list
 *
 * @param where location where to insert new node
 * @param node node to be inserted
 */
inline void list_insert(struct list_node * where, struct list_node * node) {
	assert(node);
	assert(where);

	node->next = where->next;
	node->prev = where;

	if (where->next)
		where->next->prev = node;

	where->next = node;
}

/**
 * @brief  Erase list
 *
 * @param list list to be erased
 */
inline void list_erase(struct list_node * list) {
	UNUSED(list);
	assert(list);
	assert(0 && "list_erase not implemented!");
}

#endif // LIST_H_

