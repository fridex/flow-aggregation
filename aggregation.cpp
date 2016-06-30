/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 10:29:45 PM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#include "aggregation.h"

#include "param.h"
#include "flow.h"
#include "file.h"

#include <stdint.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <iostream>
#include <pthread.h>

#include "rbtree.h"
#include "common.h"
#include "flow.h"
#include "file_list.h"
#include "bstree.h"

#ifndef THREAD_COUNT
# define THREAD_COUNT		1		// probably best value based on results on my PC
#endif

const unsigned Aggregation::PORT_COUNT = 65536;

/**
 * @brief  Compare number of packets in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
int cmp_packets(const struct bstree_node *a, const struct bstree_node *b) {
	Flow *p = bstree_container_of(a, Flow, node_sort);
	Flow *q = bstree_container_of(b, Flow, node_sort);

	if (p->data.packets == q->data.packets)
		return 0;
	else if (p->data.packets < q->data.packets)
		return 1;
	else
		return -1;
}

/**
 * @brief  Compare bytes of packets in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
int cmp_bytes(const struct bstree_node *a, const struct bstree_node *b) {
	Flow *p = bstree_container_of(a, Flow, node_sort);
	Flow *q = bstree_container_of(b, Flow, node_sort);

	if (p->data.bytes == q->data.bytes) {
		return 0;
	} else if (p->data.bytes < q->data.bytes)
		return 1;
	else
		return -1;
}

/**
 * @brief  Compare source IPv4 in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_srcip4_mask(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return  (int)(GETIPV4VAL(p->data.src_addr)) - (int)(GETIPV4VAL(q->data.src_addr));
}

/**
 * @brief  Compare source IPv6 in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_srcip6_mask(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	struct in6_addr * a1 = &p->data.src_addr;
	struct in6_addr * a2 = &q->data.src_addr;

	return memcmp(a1, a2, sizeof(struct in6_addr));
}

/**
 * @brief  Compare destination IPv4 in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_dstip4_mask(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return  (int)(GETIPV4VAL(p->data.dst_addr)) - (int)(GETIPV4VAL(q->data.dst_addr));
}

/**
 * @brief  Compare destination IPv6 in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_dstip6_mask(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	struct in6_addr * a1 = &p->data.dst_addr;
	struct in6_addr * a2 = &q->data.dst_addr;

	return memcmp(a1, a2, sizeof(struct in6_addr));
}

/**
 * @brief  Compare source IP in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_srcip(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return memcmp(&p->data.src_addr, &q->data.src_addr, sizeof(struct in6_addr));
}

/**
 * @brief  Compare destination IP in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_dstip(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return memcmp(&p->data.dst_addr, &q->data.dst_addr, sizeof(struct in6_addr));
}

/**
 * @brief  Compare source port in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_srcport(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return p->data.src_port - q->data.src_port;
}

/**
 * @brief  Compare destination port in flow
 *
 * @param a node to compare
 * @param b node to compare
 *
 * @return   return 0 if equal, 1 or -1 to point out the difference
 */
static
int cmp_dstport(const struct rbtree_node *a, const struct rbtree_node *b) {
	Flow *p = rbtree_container_of(a, Flow, node_agg);
	Flow *q = rbtree_container_of(b, Flow, node_agg);

	return q->data.dst_port - p->data.dst_port;
}

/**
 * @brief  Get leftmost node in binary search (sub)tree
 *
 * @param node node to start with
 *
 * @return   leftmost node
 */
static inline
struct bstree_node * get_leftmost(struct bstree_node * node) {
	while (node->left && ! node->left_is_thread)
		node = node->left;

	return node;
}

/**
 * @brief  Traverse tree inorder, print visited node and delete/free it
 *
 * @param tree tree to traverse
 * @param fun function used for printing
 */
static inline
void tree_inorder(struct bstree * tree, void (*fun)(const Flow *)) {
	struct bstree_node * leftmost;
	struct bstree_node * prev = NULL;
	struct bstree_node * tmp;
	struct bstree_node * tmp2;
	Flow * f;

	if (! tree->root)
		return;

	leftmost = tree->first;

	while (leftmost) {
		// print synonyms
		f = bstree_container_of(leftmost, Flow, node_sort);
		fun(f);

		// print synonyms, but  DO NOT delete leftmost
		for (tmp = leftmost->list_next; tmp; /**/) {
			tmp2 = tmp;
			tmp = tmp->list_next;
			f = bstree_container_of(tmp2, Flow, node_sort);
			fun(f);
		}

		prev = leftmost;
		leftmost = leftmost->right;

		if (! prev->right_is_thread) {
			if (leftmost) {
				leftmost = get_leftmost(leftmost);
			}
		}
	}
}

/**
 * @brief  Traverse tree inorder, print visited node and delete/free it
 *
 * @param tree tree to traverse
 * @param fun function used for printing
 */
static inline
void tree_inorder_free(struct bstree * tree, void (*fun)(const Flow *)) {
	struct bstree_node * leftmost;
	struct bstree_node * prev = NULL;
	struct bstree_node * tmp;
	struct bstree_node * tmp2;
	Flow * f;

	if (! tree->root)
		return;

	leftmost = tree->first;

	while (leftmost) {
		// print synonyms
		f = bstree_container_of(leftmost, Flow, node_sort);
		fun(f);

		// print synonyms, but  DO NOT delete leftmost
		for (tmp = leftmost->list_next; tmp; /**/) {
			tmp2 = tmp;
			tmp = tmp->list_next;
			f = bstree_container_of(tmp2, Flow, node_sort);
			fun(f);
			delete f;
		}

		if (prev) {
			delete bstree_container_of(prev, Flow, node_sort);
			prev = NULL;
		}
		prev = leftmost;
		leftmost = leftmost->right;

		if (! prev->right_is_thread) {
			if (leftmost) {
				leftmost = get_leftmost(leftmost);
			}
		}
	}

	if (prev)
		delete bstree_container_of(prev, Flow, node_sort);
}


/**
 * @brief  Lookup a flow, if found update data, otherwise insert new node
 *
 * @param flow flow to lookup
 * @param tree tree to use
 *
 * @return   true if new node was inserted, if updated return false
 */
static inline
bool rbtree_lookup_or_insert(Flow * flow, struct rbtree * tree) {
	assert(flow);
	assert(tree);

		struct rbtree_node * node;
		if ((node = rbtree_lookup(&flow->node_agg, tree)) != NULL) {
			Flow * record = rbtree_container_of(node, Flow, node_agg);
			record->data.packets += flow->data.packets;
			record->data.bytes += flow->data.bytes;
			return false;
		} else {
			rbtree_insert(&flow->node_agg, tree);
			return true;
		}
}

/**
 * @brief  Aggregate flow in single thread
 *
 * @param param aggregation parameters
 *
 * @return   NULL
 */
void * Aggregation::aggregate(struct thread_param * param) {
	Flow * flow = new Flow;

	while (Flow::getFlow(flow, param->node)) {
		if (rbtree_lookup_or_insert(flow, param->tree))
			flow = new Flow;
	}

	delete flow;

	return NULL;
}

/**
 * @brief  Aggregate flow in single thread based on dst IPv4
 *
 * @param param aggregation parameters
 *
 * @return   NULL
 */
void * Aggregation::aggregate_dstip4(struct thread_param * param) {
	Flow * flow = new Flow;
	union mask_t mask;

	get_ipv4_mask(mask, Param::getInstance().mask());

	while (Flow::getFlow(flow, param->node)) {
		if (! Flow::is_ipv4_dst(flow))
			continue;

		Flow::mask_dstip4(flow, mask);

		if (rbtree_lookup_or_insert(flow, param->tree))
			flow = new Flow;
	}

	delete flow;

	return NULL;
}

/**
 * @brief  Aggregate flow in single thread based on dst IPv6
 *
 * @param param aggregation parameters
 *
 * @return   NULL
 */
void * Aggregation::aggregate_dstip6(struct thread_param * param) {
	Flow * flow = new Flow;
	union mask_t mask;

	get_ipv6_mask(mask, Param::getInstance().mask());

	while (Flow::getFlow(flow, param->node)) {
		if (! Flow::is_ipv6_dst(flow))
			continue;

		Flow::mask_dstip6(flow, mask);

		if (rbtree_lookup_or_insert(flow, param->tree))
			flow = new Flow;
	}

	delete flow;

	return NULL;
}

/**
 * @brief  Aggregate flow in single thread based on src IPv4
 *
 * @param param aggregation parameters
 *
 * @return   NULL
 */
void * Aggregation::aggregate_srcip4(struct thread_param * param) {
	Flow * flow = new Flow;
	union mask_t mask;

	get_ipv4_mask(mask, Param::getInstance().mask());

	while (Flow::getFlow(flow, param->node)) {
		if (! Flow::is_ipv4_src(flow))
			continue;

		Flow::mask_srcip4(flow, mask);

		if (rbtree_lookup_or_insert(flow, param->tree))
			flow = new Flow;
	}

	delete flow;

	return NULL;
}

/**
 * @brief  Aggregate flow in single thread based on src IPv6
 *
 * @param param aggregation parameters
 *
 * @return   NULL
 */
void * Aggregation::aggregate_srcip6(struct thread_param * param) {
	Flow * flow = new Flow;
	union mask_t mask;

	get_ipv6_mask(mask, Param::getInstance().mask());

	while (Flow::getFlow(flow, param->node)) {
		if (! Flow::is_ipv6_src(flow))
			continue;

		Flow::mask_srcip6(flow, mask);

		if (rbtree_lookup_or_insert(flow, param->tree))
			flow = new Flow;
	}

	delete flow;

	return NULL;
}

/**
 * @brief  Aggregation entry point
 *
 * @return   false if aggregation failed (e.g. thread create failed)
 */
bool Aggregation::run() {
	struct bstree sort_tree;							// tree used for sorting
	struct rbtree agg_all;								// tree used for temporary/continous result
	pthread_t thread[THREAD_COUNT];					// threads
	struct rbtree agg_tree[2*THREAD_COUNT];		// aggregation threes for every thread
	struct thread_param param[2*THREAD_COUNT];	// thread parameters
	struct rbtree tree_init;							// tree used for initialization

	void (* print_fun)(const Flow *) = NULL;				// function used for printing flow
	void (* print_fun_header)() = NULL;						// output header
	void * (* agg_fun)(struct thread_param *) = NULL;	// thread aggregation routine
	union rbfun_t cmp_fn;							// compare function used for comparing nodes in rbtree

	/*
	 * Initialize all variables. The decision based on AGG/SORT is traversed only
	 * once, using function pointers to boost the speed.
	 */
	switch (Param::aggregation()) {
#ifndef USE_PORTMAP
		case Param::AGG_SRCPORT:
				cmp_fn = RBFUN(cmp_srcport);
				print_fun = Flow::print_srcport;
				print_fun_header = Flow::print_srcport_header;
				agg_fun = aggregate;
				break;
		case Param::AGG_DSTPORT:
				cmp_fn = RBFUN(cmp_dstport);
				print_fun = Flow::print_dstport;
				print_fun_header = Flow::print_dstport_header;
				agg_fun = aggregate;
				break;
#endif
		case Param::AGG_SRCIP:
				cmp_fn = RBFUN(cmp_srcip);
				print_fun = Flow::print_srcip;
				print_fun_header = Flow::print_srcip_header;
				agg_fun = aggregate;
				break;
		case Param::AGG_SRCIP4:
				cmp_fn = RBFUN(cmp_srcip4_mask);
				print_fun = Flow::print_srcip;
				print_fun_header = Flow::print_srcip_header;
				agg_fun = aggregate_srcip4;
				break;
		case Param::AGG_SRCIP6:
				cmp_fn = RBFUN(cmp_srcip6_mask);
				print_fun = Flow::print_srcip;
				print_fun_header = Flow::print_srcip_header;
				agg_fun = aggregate_srcip6;
				break;
		case Param::AGG_DSTIP:
				cmp_fn = RBFUN(cmp_dstip);
				print_fun = Flow::print_dstip;
				print_fun_header = Flow::print_dstip_header;
				agg_fun = aggregate;
				break;
		case Param::AGG_DSTIP4:
				cmp_fn = RBFUN(cmp_dstip4_mask);
				print_fun = Flow::print_dstip;
				print_fun_header = Flow::print_dstip_header;
				agg_fun = aggregate_dstip4;
				break;
		case Param::AGG_DSTIP6:
				cmp_fn = RBFUN(cmp_dstip6_mask);
				print_fun = Flow::print_dstip;
				print_fun_header = Flow::print_dstip_header;
				agg_fun = aggregate_dstip6;
				break;
		default:
#ifdef USE_PORTMAP
				UNUSED(cmp_dstport);
				UNUSED(cmp_srcport);
#endif
				assert(! "Unknown aggregation type!\n");
				break;
	}

	switch (Param::sort()) {
		case Param::SORT_BYTES:
			bstree_init(&sort_tree, cmp_bytes);
			break;
		case Param::SORT_PACKETS:
			bstree_init(&sort_tree, cmp_packets);
			break;
		default:
			assert(! "Unknown sort type!\n");
			break;
	}

	rbtree_init(&tree_init, cmp_fn,
					Param::getInstance().aggregation());

	memcpy(&agg_all, &tree_init, sizeof(struct rbtree));

	for (int i = 0; i < 2*THREAD_COUNT; ++i) {
		// Hey, Mr. Compiler! Are you reading this? Unroll the loop please! Do it
		// for me, I swear I will be a good boy. I am not lying this time!
		param[i].tree   = &agg_tree[i];
		memcpy(&agg_tree[i], &tree_init, sizeof(struct rbtree));
	}

#ifdef LINEAR
	// linear...
	for (auto l = Filepool::getInstance().list.end; l; l = l->prev) {
		struct thread_param param { l, &agg_all};
		agg_fun(&param);
	}
#else

	int count1 = 0;
	int count2 = 0;

	for (auto l = Filepool::getInstance().list.end; l; /*l = 2*THREAD_COUNT times l->prev*/) {
		for (count1 = 0; count1 < THREAD_COUNT && l; l = l->prev, count1++) {
			param[count1].node = l;

			if(pthread_create(&thread[count1], NULL, (void * (*)(void *))agg_fun, &param[count1])) {
				err() << "Unable to create thread!\n"; perror("pthread");
				return false;
			}
		}

		for (int i = 0; i < count2; ++i) {
			while (agg_tree[THREAD_COUNT + i].root) {
				Flow * record = rbtree_container_of(agg_tree[THREAD_COUNT + i].root, Flow, node_agg);
				rbtree_remove(agg_tree[THREAD_COUNT + i].root, &agg_tree[THREAD_COUNT + i]);
				if (! rbtree_lookup_or_insert(record, &agg_all))
					delete record;
			}
			memcpy(&agg_tree[THREAD_COUNT + i], &tree_init, sizeof(struct rbtree));
		}

		for (int i = 0; i < count1; ++i)
			pthread_join(thread[i], NULL);

		for (count2 = 0; count2 < THREAD_COUNT && l; l = l->prev, count2++) {
			param[THREAD_COUNT + count2].node = l;

			if(pthread_create(&thread[count2], NULL, (void * (*)(void *))agg_fun, &param[THREAD_COUNT + count2])) {
				err() << "Unable to create thread!\n"; perror("pthread");
				return false;
			}
		}

		for (int i = 0; i < count1; ++i) {
			while (agg_tree[i].root) {
				Flow * record = rbtree_container_of(agg_tree[i].root, Flow, node_agg);
				rbtree_remove(agg_tree[i].root, &agg_tree[i]);
				if (! rbtree_lookup_or_insert(record, &agg_all))
					delete record;
			}
			memcpy(&agg_tree[i], &tree_init, sizeof(struct rbtree));
		}

		for (int i = 0; i < count2; ++i)
			pthread_join(thread[i], NULL);
	}

	// aggregation from last iteration
	for (int i = 0; i < count2; ++i) {
		while (agg_tree[THREAD_COUNT + i].root) {
			Flow * record = rbtree_container_of(agg_tree[THREAD_COUNT + i].root, Flow, node_agg);
			rbtree_remove(agg_tree[THREAD_COUNT + i].root, &agg_tree[THREAD_COUNT + i]);
			if (! rbtree_lookup_or_insert(record, &agg_all))
				delete record;
		}
	}
#endif

	print_fun_header();

	// Construct binary tree
	while (agg_all.root) {
		Flow * record = rbtree_container_of(agg_all.root, Flow, node_agg);
		rbtree_remove(agg_all.root, &agg_all);
		bstree_insert(&record->node_sort, &sort_tree);
	}

	// traversing inorder sorted binary tree gives sorted sequence.
	// nodes are freed within function, only one traversal needed
	tree_inorder_free(&sort_tree, print_fun);

	return true;
}

/*****************************************************************************/
/*****************************************************************************/

static inline
void port_map_update_dst(const Flow * flow, struct Aggregation::port_map_t * map) {
	const unsigned idx = flow->data.dst_port;

	pthread_mutex_lock(&map[idx].mutex);
	if (map[idx].valid) {
		map[idx].flow.data.packets += flow->data.packets;
		map[idx].flow.data.bytes += flow->data.bytes;
	} else {
		map[idx].valid = true;
		memcpy(&map[idx].flow.data, &flow->data, sizeof(Flow::data));
	}
	pthread_mutex_unlock(&map[idx].mutex);
}


static inline
void port_map_update_src(const Flow * flow, struct Aggregation::port_map_t * map) {
	const unsigned idx = flow->data.src_port;

	pthread_mutex_lock(&map[idx].mutex);
	if (map[idx].valid) {
		map[idx].flow.data.packets += flow->data.packets;
		map[idx].flow.data.bytes += flow->data.bytes;
	} else {
		map[idx].valid = true;
		memcpy(&map[idx].flow.data, &flow->data, sizeof(struct Flow::data));
	}
	pthread_mutex_unlock(&map[idx].mutex);
}

static
void * aggregate_srcport(struct Aggregation::thread_param_port * param) {
	Flow flow;

	while (Flow::getFlow(&flow, param->node)) {
		port_map_update_src(&flow, param->map);
	}

	return NULL;
};

static
void * aggregate_dstport(struct Aggregation::thread_param_port * param) {
	Flow flow;

	while (Flow::getFlow(&flow, param->node)) {
		port_map_update_dst(&flow, param->map);
	}

	return NULL;
};

/**
 * @brief  Aggregation entry point
 *
 * @return   false if aggregation failed (e.g. thread create failed)
 */
bool Aggregation::run_port() {
	struct bstree sort_tree;									// tree used for sorting
	pthread_t thread[THREAD_COUNT];							// threads

	void (* print_fun)(const Flow *) = NULL;				// function used for printing flow
	void (* print_fun_header)() = NULL;						// output header
	void * (* agg_fun)(struct thread_param_port *) = NULL;		// thread aggregation routine
	struct port_map_t * port_map = NULL;
	struct thread_param_port param[THREAD_COUNT];

	port_map = new struct port_map_t[PORT_COUNT];

	/*
	 * Initialize all variables. The decision based on AGG/SORT is traversed only
	 * once, using function pointers to boost the speed.
	 */
	switch (Param::aggregation()) {
		case Param::AGG_SRCPORT:
				print_fun = Flow::print_srcport;
				print_fun_header = Flow::print_srcport_header;
				agg_fun = aggregate_srcport;
				break;
		case Param::AGG_DSTPORT:
				print_fun = Flow::print_dstport;
				print_fun_header = Flow::print_dstport_header;
				agg_fun = aggregate_dstport;
				break;
		default:
				assert(! "Unknown aggregation type!\n");
				break;
	}

	switch (Param::sort()) {
		case Param::SORT_BYTES:
			bstree_init(&sort_tree, cmp_bytes);
			break;
		case Param::SORT_PACKETS:
			bstree_init(&sort_tree, cmp_packets);
			break;
		default:
			assert(! "Unknown sort type!\n");
			break;
	}

	for (auto i = 0u; i < PORT_COUNT; /**/) {
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
		port_map[i++].mutex = PTHREAD_MUTEX_INITIALIZER;
	}

	for(auto i = 0u; i < THREAD_COUNT; ++i)
		param[i].map = port_map;

	unsigned count1 = 0;
	unsigned count2 = 0;
	for (auto l = Filepool::getInstance().list.end; l; /*l = 2*THREAD_COUNT times l->prev*/) {
		for (count1 = 0; count1 < THREAD_COUNT && l; l = l->prev, count1++) {
			param[count1].node = l;
			if(pthread_create(&thread[count1], NULL, (void * (*)(void *))agg_fun, &param[count1])) {
				err() << "Unable to create thread!\n";
				perror("pthread");
				return false;
			}
		}

		for (unsigned i = 0; i < count1; ++i)
			pthread_join(thread[i], NULL);

		for (count2 = 0; count2 < THREAD_COUNT && l; l = l->prev, count2++) {
			param[count2].node = l;
			if(pthread_create(&thread[count2], NULL, (void * (*)(void *))agg_fun, &param[count2])) {
				err() << "Unable to create thread!\n";
				perror("pthread");
				return false;
			}
		}

		for (unsigned i = 0; i < count2; ++i)
			pthread_join(thread[i], NULL);
	}

	print_fun_header();

	// Construct binary tree
	for (auto i = 0u; i < PORT_COUNT; ++i) {
		if (port_map[i].valid)
			bstree_insert(&port_map[i].flow.node_sort, &sort_tree);
	}

	// traversing inorder sorted binary tree gives sorted sequence.
	// nodes are freed within function, only one traversal needed
	tree_inorder(&sort_tree, print_fun);

	delete [] port_map;

	return true;
}

