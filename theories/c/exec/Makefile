CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

sample_program: sample_program.c
	$(CC) $(CFLAGS) $< -o $@

execvp: execvp.c
	$(CC) $(CFLAGS) $< -o $@

run-execvp: sample_program execvp
	./execvp

.PHONY: clean
clean:
	rm -f sample_program
	rm -f execvp