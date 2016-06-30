/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 01:23:12 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef PARAM_H_
#define PARAM_H_

#include <iostream>

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "common.h"

/**
 * @brief  Parameter singleton class
 */
class Param {
	public:
		/**
		 * @brief  Sort type
		 */
		enum sort_t {
			SORT_UNKNOWN,
			SORT_BYTES,
			SORT_PACKETS
		};

		/**
		 * @brief  Aggregation tyoe
		 */
		enum aggregation_t {
			AGG_UNKNOWN,
			AGG_SRCPORT,
			AGG_DSTPORT,
			AGG_SRCIP,
			AGG_DSTIP,
			AGG_SRCIP4,
			AGG_DSTIP4,
			AGG_SRCIP6,
			AGG_DSTIP6
		};

		static aggregation_t aggregation() {
			return getInstance().m_aggregation;
		}

		/**
		 * @brief  Get sort type
		 *
		 * @return  defined sort type
		 */
		static sort_t sort() {
			return getInstance().m_sort;
		}

		/**
		 * @brief  Are program arguments valid?
		 *
		 * @return   true if atguments are valid
		 */
		bool is_valid() {
			return this->m_valid;
		}

		/**
		 * @brief  Get Param singleton instacne
		 *
		 * @return   Param singleton
		 */
		static Param & getInstance() {
			static class Param singleton;
			return singleton;
		}

		/**
		 * @brief  Init Param and parse arguments
		 *
		 * @param argc arguments count
		 * @param argv[] arguments vector
		 *
		 * @return   true if initialization is valid
		 */
		bool init(int argc, char * argv[]) {
			assert((! m_valid) && "Multiple Param init!");

			const char * srcip4 = "srcip4/";
			const char * dstip4 = "dstip4/";
			const char * srcip6 = "srcip6/";
			const char * dstip6 = "dstip6/";

			argc > 1 ? m_valid = true : m_valid = false;

			for (int i = 1; i < argc; i += 2) {
				if (! strcmp(argv[i], "-f")) {
					if (i + 1 == argc) {
						err() << "Option '-f' requires a parameter!\n";
						m_valid = false;
						break;
					} else {
						m_dirname = argv[i + 1];
					}
				} else if (! strcmp(argv[i], "-a")) {
					if (i + 1 == argc) {
						err() << "Option '-a' requires a parameter!\n";
						m_valid = false;
						break;
					} else if (! strncmp(argv[i + 1], srcip4, strlen(srcip4))) {
						m_aggregation = AGG_SRCIP4;
						if (! get_mask(argv[i + 1], srcip4)) {
							m_valid = false;
							break;
						}
					} else if (! strncmp(argv[i + 1], dstip4, strlen(dstip4))) {
						m_aggregation = AGG_DSTIP4;
						if (! get_mask(argv[i + 1], dstip4)) {
							m_valid = false;
							break;
						}
					} else if (! strncmp(argv[i + 1], srcip6, strlen(srcip6))) {
						m_aggregation = AGG_SRCIP6;
						if (! get_mask(argv[i + 1], srcip6)) {
							m_valid = false;
							break;
						}
					} else if (! strncmp(argv[i + 1], dstip6, strlen(dstip6))) {
						m_aggregation = AGG_DSTIP6;
						if (! get_mask(argv[i + 1], dstip6)) {
							m_valid = false;
							break;
						}
					} else if (! strcmp(argv[i + 1], "srcip")) {
						m_aggregation = AGG_SRCIP;
					} else if (! strcmp(argv[i + 1], "dstip")) {
						m_aggregation = AGG_DSTIP;
					} else if (! strcmp(argv[i + 1], "srcport")) {
						m_aggregation = AGG_SRCPORT;
					} else if (! strcmp(argv[i + 1], "dstport")) {
						m_aggregation = AGG_DSTPORT;
					} else {
						err() << "Unknown aggregation type '" << argv[i + 1] << "'!\n";
						m_valid = false;
						break;
					}
				} else if (! strcmp(argv[i], "-s")) {
					if (i + 1 == argc) {
						err() << "Option '-s' requires a parameter!\n";
						m_valid = false;
						break;
					} else {
						if (! strcmp(argv[i + 1], "packets")) {
							m_sort = SORT_PACKETS;
						} else if (! strcmp(argv[i + 1], "bytes")) {
							m_sort = SORT_BYTES;
						} else {
							err() << "Unknown sort type '" << argv[i + 1] << "'!\n";
							m_valid = false;
							break;
						}
					}
				} else {
					err() << "Unknown option '" << argv[i] << "'!\n";
				}
			}

			if (m_valid && m_sort == SORT_UNKNOWN) {
				err() << "Sort type not entered!\n";
				m_valid = false;
			}

			if (m_valid && m_dirname == NULL) {
				err() << "Directory or file name not entered!\n";
				m_valid = false;
			}

			if (m_valid && m_aggregation == AGG_UNKNOWN) {
				err() << "Aggregation type not entered!\n";
				m_valid = false;
			}

			if (m_aggregation == AGG_SRCIP4 || m_aggregation == AGG_DSTIP4) {
				if (m_mask > 32) {
					err() << "Given mask too big for IPv4!\n";
					m_valid = false;
				}
			}

			if (m_aggregation == AGG_SRCIP6 || m_aggregation == AGG_DSTIP6) {
				if (m_mask > 128) {
					err() << "Given mask too big for IPv6!\n";
					m_valid = false;
				}
			}

			if (m_aggregation == AGG_SRCIP4 || m_aggregation == AGG_DSTIP4
				|| m_aggregation == AGG_SRCIP6 || m_aggregation == AGG_DSTIP6) {
				if (m_mask == 0) {
					err() << "Mask has to be non-zero!\n";
					m_valid = false;
				}
			}

			if (! m_valid)
				print_help(argv[0]);
			return m_valid;
		}

		/**
		 * @brief  Get mask
		 *
		 * @return   mask value
		 */
		unsigned mask() {
			return m_mask;
		}

		/**
		 * @brief  Get directory path
		 *
		 * @return directory path
		 */
		const char * path() {
			return m_dirname;
		}

	private:
		/**
		 * @brief  Constructor
		 */
		Param() {
			m_valid = false;
			m_dirname = NULL;
			m_aggregation = AGG_UNKNOWN;
			m_sort = SORT_UNKNOWN;
			m_mask = 0;
		}

		/**
		 * @brief  Get mask from argument argv using string mask_str
		 *
		 * @param argv argument to parse
		 * @param mask_str mask type
		 *
		 * @return   true on success
		 */
		bool get_mask(const char * argv, const char * mask_str) {
			char * endptr = NULL;
			m_mask = strtoul(&argv[strlen(mask_str)], &endptr, 10);

			// check enptr
			if (endptr != &argv[strlen(argv)]
					|| strlen(argv) == strlen(mask_str)) {
				err() << "Bad mask '" << &argv[strlen(mask_str)] << "'!\n";
				return false;
			}

			return true;
		}

		/**
		 * @brief  Print a simple help
		 *
		 * @param pname program name
		 */
		void print_help(char * pname) {
			using namespace std;

			cerr << "Usage: " << pname << " -a [AGREGATION] -f [FILE] -s [SORT]\n"
							<< "\t-f\t\t- file or directory name with data\n"
							<< "\t-a\t\t- aggregation type\n"
							<< "\t-s\t\t- sort type\n\n";

			cerr << "Aggregation types:\n"
							<< "\tsrcip\t\t- aggregation using source IP\n"
							<< "\tdstip\t\t- aggregation using destination IP\n"
							<< "\tsrcip4/MASK\t- aggregation using source IPv4 with mask MASK\n"
							<< "\tdstip4/MASK\t- aggregation using destination IPv4 with mask MASK\n"
							<< "\tsrcip6/MASK\t- aggregation using source IPv6 with mask MASK\n"
							<< "\tdstip6/MASK\t- aggregation using destination IPv6 with mask MASK\n"
							<< "\tsrcport\t\t- source port aggregation\n"
							<< "\tdstport\t\t- destination port aggregation\n\n";

			cerr << "Sort types:\n"
							<< "\tpackets\t\t- sort by packets\n"
							<< "\tbytes\t\t- sort by bytes\n\n";

			cerr << "Developed by Fridolin Pokorny <fridex.devel@gmail.com> 2014\n";
		}

		/**
		 * @brief  Destructor
		 */
		~Param() {
		}

		bool m_valid;						///< Is holding Param singleton valid data?
		const char		* m_dirname;	///< File or directory name
		aggregation_t	m_aggregation;	///< Aggregation used
		sort_t			m_sort;			///< Sort used
		unsigned			m_mask;			///< Mask decimal value
};

#endif // PARAM_H_

