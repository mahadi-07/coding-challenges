CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

main.out: hello.o main.o
	$(CC) $(CFLAGS) $^ -o main.out

hello.o: hello.c
	$(CC) $(CFLAGS) -c $^ -o hello.o

main.o: main.c
	$(CC) $(CFLAGS) -c $^ -o main.o


# CMD
.PHONY: run
run:
	./main.out

.PHONY: clean
clean:
	rm -f *.o *.out