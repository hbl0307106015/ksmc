# This Makefile is for smc server
# Networking 2015
#

CC=gcc
CFLAGS=-O -Wall -W
LDFLAGS=-lpthread
INCLUDES=-I.

SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
TARGET=smc


%.o : %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)


.PHONY:clean
clean:
	$(RM) $(OBJECT_FILES) $(TARGET)

