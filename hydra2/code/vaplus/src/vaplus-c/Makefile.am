AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}

lib_LIBRARIES= lib/libvaplus.a 

lib_libvaplus_a_SOURCES = src/pqueue.c src/stats.c src/vaplus_file_loaders.c  src/vaplus_index.c src/vaplus_node.c   src/ts.c src/calc_utils.c src/vaplus_query_engine.c  src/vaplus_file_buffer.c src/vaplus_file_buffer_manager.c src/dft.c

lib_libvaplus_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/vaplus

bin_vaplus_SOURCES=src/vaplus.c
bin_vaplus_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_vaplus_LDADD =  -lreadline  -lvaplus -lfftw3 -lfftw3f -ljemalloc  -lm    
bin_vaplus_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib
