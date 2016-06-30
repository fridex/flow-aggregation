/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     03/04/2014 08:47:21 PM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef MASK_H_
#define MASK_H_

#include <inttypes.h>
#include <cassert>
#include <iostream>

#include "common.h"

/**
 * @brief  Binary mask type
 */
union mask_t {
	struct ipv4_mask_t {
		uint32_t mask;
	} ipv4;

	struct ipv6_mask_t {
		uint64_t part64[2];
	} ipv6;

	uint32_t part32[4];
};

extern const char * IPV4_MASKS_VAL;
extern const char * IPV6_MASKS_VAL;

/**
 * @brief  Get binary mask for IPv4 address
 *
 * @param m mask to store
 * @param val mask decimal value
 */
inline void get_ipv4_mask(union mask_t & m, unsigned val) {
	assert(val != 0 && val <= 32);
	m.ipv4.mask = ((uint32_t *)IPV4_MASKS_VAL)[val];
}

inline unsigned int little_big(unsigned int little)
{
    return((little&0xff)<<24)+((little&0xff00)<<8)+((little&0xff0000)>>8)+((little>>24)&0xff);
}

/**
 * @brief  Get binary mask for IPv6 address
 *
 * @param m mask to store
 * @param val mask decimal value
 */
inline void get_ipv6_mask(union mask_t & m, unsigned val) {
	assert(val != 0 && val <= 128);

	if (val > 64) {
		m.ipv6.part64[1]   = ((uint64_t *)IPV6_MASKS_VAL)[64];
		m.ipv6.part64[0]   = ((uint64_t *)IPV6_MASKS_VAL)[val - 64];
	} else {
		m.ipv6.part64[1]   = ((uint64_t *)IPV6_MASKS_VAL)[val];
		m.ipv6.part64[0]   = ((uint64_t *)IPV6_MASKS_VAL)[0];
	}

		//fprintf(stderr, "%04X:", (int32_t)little_big(m.part32[2]));
		//fprintf(stderr, "%04X:", (int32_t)little_big(m.part32[3]));
		//fprintf(stderr, "%04X:", (int32_t)little_big(m.part32[0]));
		//fprintf(stderr, "%04X:", (int32_t)little_big(m.part32[1]));
		//fputc('\n', stderr);
}

#endif // MASK_H_

