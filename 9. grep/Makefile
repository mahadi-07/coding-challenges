CC=gcc
CFLAGS=-g -Wall -Wextra -std=c11
TARGET=grep_

build:
	$(CC) $(CFLAGS) *.c -o $(TARGET)

step-1:
	./$(TARGET) "" data/x.txt | diff data/y.txt -

step-2:
	./$(TARGET) J data/rockbands.txt
	echo $$?

step-3:
	./$(TARGET) -r "Nirvana" ./data

clean:
	rm -rf grep_.dSYM
	rm grep_