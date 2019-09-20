AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}

lib_LIBRARIES	= lib/libads.a

lib_libads_a_SOURCES = src/ads/isax_file_loaders.c src/ads/isax_first_buffer_layer.c src/ads/isax_index.c src/ads/isax_node.c src/ads/isax_node_buffer.c src/ads/isax_node_record.c src/ads/isax_node_split.c src/ads/isax_query_engine.c src/ads/isax_visualize_index.c src/ads/pqueue.c src/ads/sax/sax.c src/ads/sax/ts.c
lib_libads_a_CFLAGS =-I/opt/local/include -Iinclude/

bin_PROGRAMS	= bin/ads 


bin_ads_SOURCES = src/utils/ads.c
bin_ads_CFLAGS = -I/opt/local/include -Iinclude/ -I/usr/local/include/
bin_ads_LDADD = -lreadline -ljemalloc -lads -lm
bin_ads_LDFLAGS = -L/opt/local/lib -Llib/


