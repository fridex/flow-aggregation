/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 03:25:52 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <cstdio>
#include <unistd.h>

#define UNUSED(V)				((void) V)
#define GETIPV4VAL(X)		(*((int32_t *)(((char *)(&X)) + 12)))

/**
 * @brief  Print error
 *
 * @return   std::cerr
 */
inline std::ostream & err() {
#ifdef __linux__
	if (isatty(fileno(stderr)))
		std::cerr << "\033[1;31mERROR:\e[0m ";
	else
		std::cerr << "ERROR: ";
	return std::cerr;
#else
	return std::cerr << "ERROR: ";
#endif
}

/**
 * @brief  Print warning
 *
 * @return   std::cerr
 */
inline std::ostream & warn() {
#ifdef __linux__
	if (isatty(fileno(stderr)))
		std::cerr << "\e[0;33mWARNING:\e[0m ";
	else
		std::cerr << "WARNING: ";
	return std::cerr;
#else
	return std::cerr << "WARNING: ";
#endif
}

/**
 * @brief  Print debug
 *
 * @return   std::cerr
 */
#ifndef NDEBUG
#	if defined(__linux__)
#		define dbg() ((isatty(fileno(stderr))) ? \
							std::cerr << "\e[1;35mDBG:\e[0m " : \
							std::cerr << "DBG: ")
#	else
#		define dbg() std::cerr << "DBG: "
#	endif
#else
#	define dbg()		while(0) std::cerr
#endif // NDEBUG

#endif // COMMON_H_

