TARGET = abstool
OBJS = abstool.o loadfile.o patch.o savefile.o

include ../common.mk
abstool.o : showVersion.h
abstool.o patch.o savefile.o: abstool.h
abstool.o loadfile.o savefile.o: image.h
