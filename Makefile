OBJS = analysis.o
OBJS1= partialObserve.o
OBJS2= ambiguity.o
SRC = $(OBJS:.o=.cc)


# Compiler settings
CC              = g++
CFLAGS          = -Wall -O2
#INCLUDES        = -I$(SYSTEMC_INCLUDE) -I$(ACALIB_DIR)
#LIBS            = -lsystemc
#LIBDIR          = -L$(SYSTEMC_LIBDIR)


.PHONY: clean all

all: $(OBJS) $(OBJS1)

analysis: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

analysis.o: msg_def.h lpn.h analysis.cc 
	$(CC) $(CFLAGS) -c  analysis.cc


partialObserve: $(OBJS1)
	$(CC) $(CFLAGS) -o $@ $(OBJS1)
partialObserve.o: msg_def.h lpn.h partialObserve.cc
	$(CC) $(CFLAGS) -c partialObserve.cc

ambiguity: $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2)
ambiguity.o: msg_def.h lpn.h ambiguity.cc
	$(CC) $(CFLAGS) -c ambiguity.cc

clean:
	rm *.o