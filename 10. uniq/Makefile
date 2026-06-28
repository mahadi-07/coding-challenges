CC=gcc
CFLAGS=-g -Wall -Wextra -std=c11
TARGET=grep_

build:
	$(CC) $(CFLAGS) *.c -o $(TARGET)

step-1: build
	$(MAKE) step-1.1
	$(MAKE) step-1.2
	
step-1.1: build
	./$(TARGET) ./data/test.txt

step-1.2: build
	./$(TARGET) ./data/countries.txt | ../1.\ wc\ tool/ccwc.sh -l