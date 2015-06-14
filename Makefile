# This Makefile is for smc server
# VIA Networking 2015
#

CC=gcc
CFLAGS=-O2 -Wall -W -pedantic -ansi -std=gnu99
LDFLAGS=-lpthread -levent
INCLUDES=-I.

SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))
TARGET=weplus_client


%.o : %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(LDFLAGS)


.PHONY:clean
clean:
	$(RM) $(OBJECT_FILES) $(TARGET)

