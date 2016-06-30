/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 04:00:00 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef FLOW_H_
#define FLOW_H_

#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

#include "rbtree.h"
#include "bstree.h"

#include "file_list.h"
#include "linked_list.h"


/**
 * @brief  Flow record structure and routines
 */
class Flow {
	public:
		struct data {
			uint32_t				sa_family;
			struct in6_addr	src_addr;
			struct in6_addr	dst_addr;
			uint16_t				src_port;
			uint16_t				dst_port;
			uint64_t				packets;
			uint64_t				bytes;
		} data;

		struct rbtree_node			node_agg;		// node for aggregation tree
		struct bstree_node			node_sort;		// node for sort tree

		/**
		 * @brief  Is source an IPv4?
		 *
		 * @param f Flow to check
		 *
		 * @return   true if is an IPv4
		 */
		static bool is_ipv4_src(const Flow * f) {
			return IN6_IS_ADDR_V4COMPAT(&f->data.src_addr);
		}
		/**
		 * @brief  Is source an IPv6?
		 *
		 * @param f Flow to check
		 *
		 * @return   true if is an IPv6
		 */
		static bool is_ipv6_src(const Flow * f) {
			//return f->data.sa_family == AF_INET6;
			return ! is_ipv4_src(f);
		}
		/**
		 * @brief  Is destination an IPv4?
		 *
		 * @param f Flow to check
		 *
		 * @return   true if is an IPv4
		 */
		static bool is_ipv4_dst(const Flow * f) {
			return IN6_IS_ADDR_V4COMPAT(&f->data.dst_addr);
		}
		/**
		 * @brief  Is destination an IPv6?
		 *
		 * @param f Flow to check
		 *
		 * @return   true if is an IPv6
		 */
		static bool is_ipv6_dst(const Flow * f) {
			//return f->data.sa_family == AF_INET6;
			return ! is_ipv4_dst(f);
		}

		/**
		 * @brief  Print source IP header
		 */
		static void print_srcip_header() {
			std::cout << "#srcip,packets,bytes\n";
		}

		/**
		 * @brief  Print source port IP header
		 */
		static void print_srcport_header() {
			std::cout << "#srcport,packets,bytes\n";
		}

		/**
		 * @brief  Print destination IP header
		 */
		static void print_dstip_header() {
			std::cout << "#dstip,packets,bytes\n";
		}

		/**
		 * @brief  Print destination port header
		 */
		static void print_dstport_header() {
			std::cout << "#dstport,packets,bytes\n";
		}

		/**
		 * @brief  Print flow based on source IP
		 *
		 * @param flow Flow to print
		 */
		static void print_srcip(const Flow * flow) {
			char srcip[INET6_ADDRSTRLEN];

			if (is_ipv4_src(flow))
				inet_ntop(AF_INET, (((char *)&flow->data.src_addr) + 12), srcip, INET6_ADDRSTRLEN);
			else
				inet_ntop(AF_INET6, &flow->data.src_addr, srcip, INET6_ADDRSTRLEN);

			std::cout << srcip
				<< "," << flow->data.packets
				<< "," << flow->data.bytes << std::endl;

			return;
		}

		/**
		 * @brief  Print flow based on destination IP
		 *
		 * @param flow Flow to print
		 */
		static void print_dstip(const Flow * flow) {
			char dstip[INET6_ADDRSTRLEN];

			if (is_ipv4_src(flow))
				inet_ntop(AF_INET, (((char *)&flow->data.dst_addr) + 12), dstip, INET6_ADDRSTRLEN);
			else
				inet_ntop(AF_INET6, &flow->data.dst_addr, dstip, INET6_ADDRSTRLEN);

			std::cout << dstip
				<< "," << flow->data.packets
				<< "," << flow->data.bytes << std::endl;

			return;
		}

		/**
		 * @brief  Print flow based on source port
		 *
		 * @param flow Flow to print
		 */
		static void print_srcport(const Flow * flow) {
			std::cout << ntohs(flow->data.src_port)
				<< "," << flow->data.packets
				<< "," << flow->data.bytes << std::endl;

			return;
		}

		/**
		 * @brief  Print flow based on destination port
		 *
		 * @param flow Flow to print
		 */
		static void print_dstport(const Flow * flow) {
			std::cout << ntohs(flow->data.dst_port)
				<< "," << flow->data.packets
				<< "," << flow->data.bytes << std::endl;

			return;
		}

		/**
		 * @brief  Debug procedure to print flow in user-friendly manner
		 *
		 * @param flow Flow to be printed
		 * @param f output stream
		 */
		static void print_flow(const Flow * flow, std::ostream & f) {
			static struct in6_addr zero_addr; // zero-ed

			char srcip[INET6_ADDRSTRLEN];
			char dstip[INET6_ADDRSTRLEN];

			if (memcmp((void *)&flow->data.src_addr,
							(void *)&zero_addr,
							sizeof(struct in6_addr))) {
				inet_ntop(AF_INET6, &(flow->data.src_addr), srcip, INET6_ADDRSTRLEN);
				f << srcip;
			} else
				f << "multicast";

			f << " port " << ntohs(flow->data.src_port)
				<<  " -> ";

			if (memcmp((void *)&flow->data.dst_addr,
							(void *)&zero_addr,
							sizeof(struct in6_addr))) {
				inet_ntop(AF_INET6, &(flow->data.dst_addr), dstip, INET6_ADDRSTRLEN);
				f << dstip;
			} else
				f << "multicast";

			f << " port " << ntohs(flow->data.dst_port)
				<< ", packets: " << flow->data.packets
				<< ", bytes: " << flow->data.bytes << std::endl;
		}

		static void mask_dstip4(Flow * flow, union mask_t & mask) {
			UNUSED(flow);
			UNUSED(mask);

			flow->data.dst_addr.__in6_u.__u6_addr32[0] = 0;
			flow->data.dst_addr.__in6_u.__u6_addr32[1] = 0;
			flow->data.dst_addr.__in6_u.__u6_addr32[2] = 0;
			flow->data.dst_addr.__in6_u.__u6_addr32[3] &= mask.ipv4.mask;
		}

		static void mask_dstip6(Flow * flow, union mask_t & mask) {
			// note order :(
			flow->data.dst_addr.__in6_u.__u6_addr32[0] &= mask.part32[2];
			flow->data.dst_addr.__in6_u.__u6_addr32[1] &= mask.part32[3];
			flow->data.dst_addr.__in6_u.__u6_addr32[2] &= mask.part32[0];
			flow->data.dst_addr.__in6_u.__u6_addr32[3] &= mask.part32[1];
		}

		static void mask_srcip4(Flow * flow, union mask_t & mask) {
			flow->data.src_addr.__in6_u.__u6_addr32[0] = 0;
			flow->data.src_addr.__in6_u.__u6_addr32[1] = 0;
			flow->data.src_addr.__in6_u.__u6_addr32[2] = 0;
			flow->data.src_addr.__in6_u.__u6_addr32[3] &= mask.ipv4.mask;
		}

		static void mask_srcip6(Flow * flow, union mask_t & mask) {
			// note order :(
			flow->data.src_addr.__in6_u.__u6_addr32[0] &= mask.part32[2];
			flow->data.src_addr.__in6_u.__u6_addr32[1] &= mask.part32[3];
			flow->data.src_addr.__in6_u.__u6_addr32[2] &= mask.part32[0];
			flow->data.src_addr.__in6_u.__u6_addr32[3] &= mask.part32[1];
		}

		/**
		 * @brief  Get flow from a file
		 *
		 * @param flow read Flow
		 * @param node node describing file
		 *
		 * @return   true on success
		 */
		static bool getFlow(Flow * flow, struct linked_list_node * node) {
			int c = fread(&flow->data, sizeof(struct Flow::data), 1, node->f);

			if (! feof(node->f) && c > 0) {
				flow->data.packets = __builtin_bswap64(flow->data.packets);
				flow->data.bytes = __builtin_bswap64(flow->data.bytes);
				return true;
			} else
				return false;
		}
};

#endif // FLOW_H_

