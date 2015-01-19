# This Makefile is for KNX
# VIA Networking <2014.12>

CC=gcc
CFLAGS=-Wall -W -pedantic -std=c99 -DUSE_POSIX=1
LDFLAGS=
INCLUDES=-L.

TARGET=knx
SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))

#%.o : %.c
#	$(CC) -c $(CFLAGS) $< -o $@

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(INCLUDES) $(LDFLAGS)

.PHONY : clean
clean:
	$(RM) $(TARGET) $(OBJECT_FILES)
