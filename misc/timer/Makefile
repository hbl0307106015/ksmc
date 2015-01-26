# This Makefile is for timer
# VIA Networking <2015.1>

CC=gcc
CFLAGS=-Wall -W -O -g
LDFLAGS=
INCLUDES=-I.

TARGET=timer
SOURCE_FILES=$(wildcard *.c)
OBJECT_FILES=$(patsubst %.c,%.o,$(SOURCE_FILES))

%.o : %.c $.h
	$(CC) $(CLFAGS) $(INCLUDES) -c $< -o $@

$(TARGET) : $(OBJECT_FILES)
	$(CC) $^ -o $@ $(LDFALGS)

.PHONY:clean
clean:
	$(RM) $(OBJECT_FILES) $(TARGET)
