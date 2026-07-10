CC := gcc
CFLAGS := -g -Wall -Wextra -std=c11

REDIS_DIR := ../8.\ redis\ server/redis_clone/src

SRCS := \
	$(REDIS_DIR)/resp.c \
	$(REDIS_DIR)/server.c \
	$(REDIS_DIR)/utils.c \
	$(REDIS_DIR)/server_main.c \
	$(REDIS_DIR)/db.c

server:
	$(CC) $(CFLAGS) $(SRCS) -o $@

diff_tool: diff_tool.c
	$(CC) $(CFLAGS) $^ -o $@

run: diff_tool
	./diff_tool
	$(MAKE) clean

test:
	$(CC) $(CFLAGS) \
	diff_tool.c \
	tests/test_lcs_word.c \
	tests/test_lcs_lines.c \
	tests/test_runner.c \
	-o lcs_word \

	./lcs_word

clean:
	rm -f diff_tool server
	rm -rf *.dSYM
	rm -f lcs_word