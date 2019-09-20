# http://www.gnu.org/software/make/manual/make.html
# http://netbsd.gw.com/cgi-bin/man-cgi?make+1+NetBSD-current

CXX      = g++
CFLAGS   = -Wall -O2 # -ggdb # -O3 -ffast-math -msse2 -msse3 -ftree-vectorize -DNDEBUG # -ggdb # -O3 -ffast-math -msse2 -msse3 -ftree-vectorize  # -g -ggdb -pg
DEFINES  =  #  -DNDEBUG
INCLUDES = -I./boost_1_53_0 -I/sw/include
LFLAGS = -L/sw/lib -lgsl -lgslcblas -lm -lboost_filesystem 
APPNAMES = mtree

#####################################################################

SRC	= $(wildcard *.cpp) 

HDR	= $(wildcard *.h)
OBJ	= $(filter-out $(addsuffix .o, $(APPNAMES)),$(addsuffix .o, $(basename $(SRC)))) 
OBJ2 = $(addsuffix .o, $(APPNAMES))

#####################################################################

all: header $(APPNAMES) trailer


%.o: %.cpp
	@echo Compiling: "$@ ( $< )"
	$(CXX) $(INCLUDES) $(CFLAGS) $(DEFINES) -c -o $@ $<


$(APPNAMES): $(LIBS) $(OBJ) $(OBJ2)
	@echo Compiling: "$@ ( $^ )"
	$(CXX)  $(CFLAGS) $(LFLAGS) $(OBJ) $@.o -o $@ $(LIBS)


header:
	@echo "%"
	@echo "% Compiling apps {$(APPNAMES)}"
	@echo "%"
	@echo "%  CCX..................: $(CXX)"
	@echo "%  CFLAGS...............: $(CFLAGS)"
	@echo "%  DEFINES..............: $(DEFINES)"
	@echo "%  INCLUDES.............: $(INCLUDES)"
	@echo "%  LIBS.................: $(LIBS)"
	@echo "%  APPNAMES.............: $(APPNAMES)"
	@echo "%"

trailer:
	@echo "%"
	@echo "% Apps {$(APPNAMES)} ready."
	@echo "%"


.PHONY: all clean header dist


clean:
	@rm -rf *.o *.dSYM $(APPNAMES)


# eof
