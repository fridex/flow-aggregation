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

#ifndef FILE_LIST_H_
#define FILE_LIST_H_

#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

#include "flow.h"

/**
 * @brief  Node used to store a file
 */
struct linked_list_node {
	struct linked_list_node * prev;
	FILE * f;
};

/**
 * @brief  Linked list node
 */
struct linked_list {
	struct linked_list_node * end;
};

#define linked_list_last(X)		((X)->end)
#define linked_list_empty(X)		((X)->end == NULL)

/**
 * @brief  Init linked list
 *
 * @param list list to init
 */
inline void linked_list_init(struct linked_list * list) {
	assert(list);
	list->end = NULL;
}

/**
 * @brief  Push node into linked list
 *
 * @param node node to push
 * @param list linked_list to be used
 */
inline void linked_list_push(struct linked_list_node * node, struct linked_list * list) {
	assert(node);
	assert(list);

	node->prev = list->end;
	list->end = node;
}

/**
 * @brief  Pop node from linked list
 *
 * @param list list to be used
 *
 * @return   popped linked_list_node
 */
inline struct linked_list_node * linked_list_pop(struct linked_list * list) {
	assert(list);
	struct linked_list_node * ret;

	ret = list->end;
	list->end = list->end->prev;
	return ret;
}

#endif // FILE_LIST_H_

