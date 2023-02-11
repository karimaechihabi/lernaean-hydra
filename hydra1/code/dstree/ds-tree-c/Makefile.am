AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}
AM_CFLAGS="-g -O0"

lib_LIBRARIES= lib/libdstree.a 

lib_libdstree_a_SOURCES = src/pqueue.c src/dstree_file_loaders.c  src/dstree_index.c src/dstree_node.c src/dstree_node_split.c  src/ts.c src/calc_utils.c src/dstree_query_engine.c  src/dstree_file_buffer.c src/dstree_file_buffer_manager.c  

lib_libdstree_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/dstree

bin_dstree_SOURCES=src/dstree.c
bin_dstree_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_dstree_LDADD = -lreadline -L$/lib  -ldstree -lm
bin_dstree_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib
