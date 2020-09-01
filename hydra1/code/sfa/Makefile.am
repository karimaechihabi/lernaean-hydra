AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}
AM_CFLAGS="-g -O0"

lib_LIBRARIES= lib/libsfa.a 

lib_libsfa_a_SOURCES = src/pqueue.c src/sfa_file_loaders.c  src/sfa_trie.c src/sfa_node.c   src/ts.c src/calc_utils.c src/sfa_query_engine.c  src/sfa_file_buffer.c src/sfa_file_buffer_manager.c src/dft.c

lib_libsfa_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/sfa

bin_sfa_SOURCES=src/sfa.c
bin_sfa_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_sfa_LDADD =  -lreadline  -lsfa  -lfftw3f -ljemalloc  -lm    
bin_sfa_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib
