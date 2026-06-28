CC=gcc
CFLAGS=-g -Wall -Wextra -std=c11
TARGET=uniq

build:
	$(CC) $(CFLAGS) *.c -o $(TARGET)

step-1: build
	$(MAKE) step-1.1
	$(MAKE) step-1.2
	$(MAKE) clean
	
step-1.1: build
	./$(TARGET) ./data/test.txt
	$(MAKE) clean

step-1.2: build
	./$(TARGET) ./data/countries.txt | ../1.\ wc\ tool/ccwc.sh -l
	$(MAKE) clean

step-2.1: build
	cat ./data/test.txt | ./$(TARGET) -
	$(MAKE) clean

step-2.2: build
	cat ./data/test.txt | ./$(TARGET) - out.txt
	$(MAKE) clean
	echo
	cat out.txt

step-3: build
	./$(TARGET) -c ./data/test.txt
	$(MAKE) clean

step-4: build
	./$(TARGET) -d ./data/test.txt
	$(MAKE) clean

step-5: build
	./$(TARGET) -u ./data/test.txt
	$(MAKE) clean

clean:
	rm -f $(TARGET)
	rm -rf uniq.dSYM

clean-all:
	rm out.txt
	$(MAKE) clean