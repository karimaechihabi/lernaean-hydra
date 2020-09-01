##############
# Make the SRS project
##############

PROD	:= DEBUG
OPT     := -O2 # -pg
VERSION := \"0.1_${PROD}\"
TARGETS := srs-ct cal_param gen_gt gen_hard_data #name of binary file
DEFINES := #-DREAL_PROF
SRCS    := stats.cpp srs.cpp ProjData.cpp ParamFile.cpp RandGen.cpp SRSCoverTree.cpp cal_param.cpp gen_gt.cpp gen_hard_data.cpp

CCFLAGS = -std=c++11 ${OPT} -Wno-deprecated  ${DEFINES} -I./ -I${HOME}/local/include/ -DVERSION=${VERSION}
LDFLAGS = ${OPT} 
LIBS    = 
CC	= g++ 
OBJS    := ${SRCS:.cpp=.o}


.PHONY: all clean distclean 
all:: ${TARGETS} 

srs-ct: srs.o SRSCoverTree.o ParamFile.o ProjData.o RandGen.o stats.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

cal_para: cal_param.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

gen_gt: gen_gt.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

gen_hard_data: gen_hard_data.o RandGen.o
	${CC} ${LDFLAGS} -o $@ $^ ${LIBS}

${OBJS}: %.o: %.cpp
	${CC} ${CCFLAGS} -o $@ -c $< 

clean:: 
	-rm -f *~ *.o ${TARGETS}

distclean:: clean
