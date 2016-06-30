#############################################################
#
#        Makefile for PDS project at BUT FIT 2014
#                   Fridolin Pokorny
#                fridex.devel@gmail.com
#
#############################################################

CXX=g++
LDFLAGS=-lm -lc -pthread
#CXXFLAGS=-std=gnu++0x -O3 -finline-limit=200000 -fomit-frame-pointer -Wall -DNDEBUG -DTHREAD_COUNT=4
CXXFLAGS=-std=gnu++0x -O3 -finline-limit=200000 -fomit-frame-pointer -Wall -DNDEBUG -DTHREAD_COUNT=4 -DUSE_PORTMAP
#CXXFLAGS=-std=c++11 -ggdb

SRCS=main.cpp aggregation.cpp rbtree.cpp bstree.cpp rbtree.cpp mask.cpp
HDRS=param.h flow.h diruse.h common.h aggregation.h rbtree.h linked_list.h bstree.h file.h file_list.h mask.h
AUX=Makefile

PACKNAME=project.zip

all: clean flow

.PHONY: clean pack

flow: ${SRCS}
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

pack:
	#make -C DOC/
	#mv DOC/Documentation.pdf .
	zip -R $(PACKNAME) $(SRCS) $(HDRS) ./$(AUX) Documentation.pdf

clean:
	@rm -f *.o flow $(PACKNAME) Documentation.pdf

