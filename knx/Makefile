# This Makefile is for KNX
# Networking <2014.12>

CC=gcc
CFLAGS=-Wall -W -O -DUSE_POSIX=1
LDFLAGS=-lpthread
INCLUDES=-I.

TARGET=knx
SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))

#%.o : %.c
#	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)

.PHONY : clean
clean:
	$(RM) $(TARGET) $(OBJECT_FILES) *~
