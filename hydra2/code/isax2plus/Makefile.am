AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}
AM_CFLAGS="-g -O0"

lib_LIBRARIES= lib/libisax2plus.a 

lib_libisax2plus_a_SOURCES = src/isax_file_loaders.c src/isax_first_buffer_layer.c src/isax_index.c src/isax_node.c src/isax_node_buffer.c src/isax_node_record.c src/isax_node_split.c src/isax_query_engine.c src/isax_visualize_index.c  src/pqueue.c src/sax/sax.c src/sax/ts.c

lib_libisax2plus_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/isax2plus

bin_isax2plus_SOURCES=src/main.c
bin_isax2plus_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_isax2plus_LDADD = -lreadline -L$/lib -ljemalloc -lisax2plus -lm
bin_isax2plus_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib


