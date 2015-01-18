# This Makefile is for KNX
#

CC=gcc
CFLAGS=-Wall
LDFLAGS=
INCLUDES=

TARGET=knx
SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

#%.o : %.c %.h
#	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(INCLUDES) $(LDFLAGS)

.PHONY : clean
clean:
	$(RM) $(TARGET) $(OBJECT_FILES)

