/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 10:31:11 PM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef AGGREGATION_H_
#define AGGREGATION_H_

#include <semaphore.h>

#include "rbtree.h"
#include "file.h"
#include "param.h"
#include "mask.h"

#include "file_list.h"
#include "bstree.h"

/**
 * @brief  Aggregation routines
 */
class Aggregation {
	public:
		typedef Param::aggregation_t aggregation_t;
		typedef Param::sort_t sort_t;

		struct thread_param {
			struct linked_list_node * node;
			struct rbtree * tree;
		};

		struct port_map_t {
			bool valid;
			class Flow flow;
			pthread_mutex_t mutex;
		};

		struct thread_param_port {
			struct linked_list_node * node;
			struct port_map_t * map;
		};

		static const unsigned PORT_COUNT;

		static bool run();
		static bool run_port();
		static void * aggregate(struct thread_param * param);
		static void * aggregate_srcip4(struct thread_param * param);
		static void * aggregate_srcip6(struct thread_param * param);
		static void * aggregate_dstip4(struct thread_param * param);
		static void * aggregate_dstip6(struct thread_param * param);

	private:
		Aggregation() { }
		~Aggregation() { }
};


int cmp_bytes(const struct bstree_node *a, const struct bstree_node *b);
int cmp_packets(const struct bstree_node *a, const struct bstree_node *b);

#endif // AGGREGATION_H_

