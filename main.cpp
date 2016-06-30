/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 03:49:12 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#include <iostream>

#include "param.h"
#include "file.h"
#include "flow.h"
#include "aggregation.h"

enum {
	RET_OK,
	RET_ERR_PARAM,
	RET_ERR_FILE,
	RET_ERR_AGG
};


/**
 * @brief  Main
 *
 * @param argc argument count
 * @param argv[] argument vector
 *
 * @return   RET_OK on success, otherwise error code indicating error
 */
int main(int argc, char * argv[]) {
	Param::getInstance().init(argc, argv);

	if (! Param::getInstance().is_valid())
		return RET_ERR_PARAM;

		if (! Filepool::getInstance().init(Param::getInstance().path()))
		return RET_ERR_FILE;

#ifdef USE_PORTMAP
	if (Param::getInstance().aggregation() == Param::AGG_SRCPORT
			|| Param::getInstance().aggregation() == Param::AGG_DSTPORT) {
		if (! Aggregation::run_port())
			return RET_ERR_AGG;
	} else
#endif // USE_PORTMAP
		if (! Aggregation::run())
			return RET_ERR_AGG;

	return RET_OK;
}

