CC	= gcc
CFLAGS	= -Wall -g
LDFLAGS	= -lpthread -ljpeg -lm #-lcurses

TARGETS	= main
.PHONY: all clean

all: $(TARGETS)
	
clean:
	rm -f *~ a.out *.o $(TARGETS) core.*  .*.*.swp
