/*
 * rbtree - Implements a red-black tree with parent pointers.
 *
 * Copyright (C) 2010 Franck Bui-Huu <fbuihuu@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 * For recall a red-black tree has the following properties:
 *
 *     1. All nodes are either BLACK or RED
 *     2. Leafs are BLACK
 *     3. A RED node has BLACK children only
 *     4. Path from a node to any leafs has the same number of BLACK nodes.
 *
 */

#ifndef RBTREE_H_
#define RBTREE_H_

#include <stdint.h>
#include <stddef.h>

#include "mask.h"
#include "param.h"

/*
 * The definition has been stolen from the Linux kernel.
 */
#ifdef __GNUC__
#  define rbtree_container_of(node, type, member) ({			\
	const struct rbtree_node *__mptr = (node);			\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#else
#  define rbtree_container_of(node, type, member)			\
	((type *)((char *)(node) - offsetof(type, member)))
#endif	/* __GNUC__ */

/*
 * Red-black tree
 */
enum rb_color {
	RB_BLACK,
	RB_RED,
};

struct rbtree_node {
	struct rbtree_node *left, *right;
	struct rbtree_node *parent;
	enum rb_color color;
};

typedef int (*rbtree_cmp_fn_t)(const struct rbtree_node *, const struct rbtree_node *);
typedef int (*rbtree_cmp_mask_fn_t)(const struct rbtree_node *, const struct rbtree_node *, const union mask_t * port);

union rbfun_t {
	rbtree_cmp_fn_t cmp_fn;
	rbtree_cmp_mask_fn_t cmp_mask_fn;
};

static inline
union rbfun_t
RBFUN(rbtree_cmp_fn_t fn) {
	union rbfun_t f;
	f.cmp_fn = fn;
	return f;
}
static inline
union rbfun_t RBFUN(rbtree_cmp_mask_fn_t fn) {
	union rbfun_t f;
	f.cmp_mask_fn = fn;
	return f;
}


struct rbtree {
	struct rbtree_node *root;
	union rbfun_t fun;
	struct rbtree_node *first, *last;
	uint64_t reserved[4];
};

inline void rbtree_init(struct rbtree * tree,
								union rbfun_t fn,
								Param::aggregation_t agg = Param::AGG_UNKNOWN) {
	tree->root = NULL;
	tree->fun = fn;
	tree->first = NULL;
	tree->last = NULL;
}

struct rbtree_node *rbtree_first(const struct rbtree *tree);
struct rbtree_node *rbtree_last(const struct rbtree *tree);
struct rbtree_node *rbtree_next(const struct rbtree_node *node);
struct rbtree_node *rbtree_prev(const struct rbtree_node *node);

struct rbtree_node *rbtree_lookup(const struct rbtree_node *key, const struct rbtree *tree);
struct rbtree_node *rbtree_insert(struct rbtree_node *node, struct rbtree *tree);
void rbtree_remove(struct rbtree_node *node, struct rbtree *tree);
void rbtree_replace(struct rbtree_node *old, struct rbtree_node *node, struct rbtree *tree);

#endif // RBTREE_H_

