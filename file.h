/*
 ***********************************************************************
 *
 *        @version  1.0
 *        @date     02/25/2014 03:51:19 AM
 *        @author   Fridolin Pokorny <fridex.devel@gmail.com>
 *
 * COPYING:
 * Distributed under the terms of beer license. If you like this and
 * you want to thank me (or use these sources), you have to buy
 * me a beer.
 *
 ***********************************************************************
 */

#ifndef DIRUSE_H_
#define DIRUSE_H_

#include <list>
#include <fstream>
#include <ios>
#include <stdio.h>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <dirent.h>

#include "common.h"
#include "flow.h"
#include "file_list.h"

/**
 * @brief  Filepool structure
 */
class Filepool {
	public:
		struct linked_list list;

		/**
		 * @brief  Initialize filepool based on path
		 *
		 * @param path directory path to use
		 *
		 * @return   true on succes
		 */
		bool init(const std::string & path) {
			struct stat s;
			struct stat s2;
			bool ret = true;

			linked_list_init(&list);

			if (stat(path.c_str(), &s) == 0) {
				if(s.st_mode & S_IFDIR) {
					DIR *dir;
					struct dirent *ent;
					if ((dir = opendir(path.c_str())) != NULL) {
						while ((ent = readdir(dir)) != NULL) {
							if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
								if (stat((path + "/" + ent->d_name).c_str(), &s2) == 0) {

									if (s2.st_mode & S_IFREG) {
										if (! insert(path + "/" + ent->d_name)) {
											closedir(dir);
											return false;
										}
									} else {
										err() << "Not a regular file '"
												<< ent->d_name << "' in '"
												<< path << "'\n";
										closedir(dir);
										return false;
									}
								} else {
									std::cerr << "Cannot stat()\n";
									perror((path + "/" + ent->d_name).c_str());
									closedir(dir);
									return false;
								}
							}
						}
					closedir(dir);
					} else {
						/* could not open directory */
						std::cerr << "Cannot open directory!\n";
						perror (path.c_str());
						return EXIT_FAILURE;
					}
				} else if (s.st_mode & S_IFREG) {
					warn() << "Single file given, using only one file\n";
					if (! insert(path)) {
							return false;
					} else
						return true;
				} else {
					err() << "Unknown file type!\n";
					return false;
				}
			} else {
				err() << "Failed to open file pool!\n";
				perror(path.c_str());
				return false;
			}

			return ret;
		}

		/**
		 * @brief  Get singleton instance
		 *
		 * @return   singleton instance of Filepool
		 */
		static Filepool & getInstance() {
			static Filepool singleton;
			return singleton;
		}

	private:
		/**
		 * @brief  Constructor
		 */
		Filepool() {
		}

		/**
		 * @brief  Destructor
		 */
		~Filepool() {
			for (struct linked_list_node * i = linked_list_last(&list); i; /**/){
				struct linked_list_node * tmp = i->prev;
				fclose(i->f);
				delete i;
				i = tmp;
			}
		}

		/**
		 * @brief  Insert file fname to the pool
		 *
		 * @param fname name of a file to be inserted
		 *
		 * @return   true on success
		 */
		bool insert(const std::string & fname) {
			struct linked_list_node * node;

			node = new struct linked_list_node;

			if ((node->f = fopen(fname.c_str(), "rb")) == NULL) {
				perror(fname.c_str());
				err() << "Failed to add to pool!\n";
				return false;
			}

			linked_list_push(node, &list);

			return true;
		}
};

#endif // DIRUSE_H_

