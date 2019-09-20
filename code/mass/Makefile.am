AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}

lib_LIBRARIES= lib/libmass.a 

lib_libmass_a_SOURCES = src/mass_instance.c src/mass_file_loaders.c src/ts.c src/calc_utils.c src/mass_query_engine.c  src/mass_buffer_manager.c 

lib_libmass_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/mass

bin_mass_SOURCES=src/mass.c
bin_mass_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_mass_LDADD = -lreadline -L$/lib  -ljemalloc -lmass -lfftw3f -lm 
bin_mass_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib
