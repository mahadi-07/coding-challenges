CC=gcc
CFLAGS=-g -Wall -Wextra -std=c11
TARGET=grep_

build:
	$(CC) $(CFLAGS) *.c -o $(TARGET)

step-1: build
	./$(TARGET) "" data/x.txt | diff data/y.txt -
	$(MAKE) clean

step-2: build
	./$(TARGET) J data/rockbands.txt
	echo $$?
	$(MAKE) clean

step-3: build
	./$(TARGET) -r "Nirvana" ./data
	$(MAKE) clean

step-4: build
	./$(TARGET) -r "Nirvana" ./data | ./$(TARGET) -v "Madonna"
	$(MAKE) clean

step-5.1: build
	./$(TARGET) "\d" ./data/test-subdir/BFS1985.txt
	$(MAKE) clean

step-5.2: build
	./$(TARGET) "\w" ./data/symbols.txt
	$(MAKE) clean

clean:
	rm -rf $(TARGET).dSYM
	rm $(TARGET)