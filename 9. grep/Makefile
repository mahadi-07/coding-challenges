CC=gcc
CFLAGS=-g -Wall -Wextra -std=c11
TARGET=grep_

build:
	$(CC) $(CFLAGS) *.c -o $(TARGET)

step-1:
	$(build)
	./$(TARGET) "" data/x.txt | diff data/y.txt -

step-2:
	$(build)
	./$(TARGET) J data/rockbands.txt
	echo $$?

step-3:
	$(build)
	./$(TARGET) -r "Nirvana" ./data

step-4:
	$(build)
	./$(TARGET) -r "Nirvana" ./data | ./$(TARGET) -v "Madonna"


clean:
	rm -rf grep_.dSYM
	rm grep_