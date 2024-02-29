TARGET = dumpomf
OBJS =	common.o main.o mem.o omf51.o omf85.o omf86.o omf96.o readobj.o typedef86.o

include ../common.mk

main.o: showVersion.h
$(OBJS): omf.h
omf86.o typedef86.o: omf86.h
