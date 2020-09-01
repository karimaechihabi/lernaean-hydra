AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS =  ${ACLOCAL_FLAGS}
AM_CFLAGS=""

lib_LIBRARIES= lib/libserial.a 

lib_libserial_a_SOURCES =  src/ts.c src/serial_scan_utils.c 

lib_libserial_a_CFLAGS=-I/opt/local/include -Iinclude 

bin_PROGRAMS = bin/serial

bin_serial_SOURCES=src/serial_scan.c
bin_serial_CFLAGS = -I/opt/local/include -Iinclude -I/usr/local/include/ 
bin_serial_LDADD = -lreadline -L$/lib -ljemalloc -lserial -lm
bin_serial_LDFLAGS = -L/opt/local/lib -Llib/ -Wl,-rpath,-r/lib
