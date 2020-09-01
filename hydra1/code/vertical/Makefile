CXXFLAGS=-g -Wall -O3 -DBIT_SIGNS_MATRIX
LDLIBS=-lrt
CC=g++
OBJS=wavelets_karras.o search.o nodes.o heap.o ./utils/utils.o ./utils/computeDHWT.o ./utils/bitArray.o

wavelets_karras : $(OBJS)

clean : 
	rm -f *.d *.o ./utils/*.o ./utils/*.d wavelets_karras

%.d : %.cpp %.hpp
	$(CC) -MM -MF $@ $< 

-include $(OBJS : .o=.d )


gprof: 
	g++ -g -Wall -pg hscandb/*.cpp hscandb/bounds/*.cpp utils/*.cpp afft.cpp  -lfftw3 -o wavelets_karras
