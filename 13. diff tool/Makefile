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

diff_tool: diff_tool.c ../lib/algorithm.c
	$(CC) $(CFLAGS) $^ -o $@

run: diff_tool
	./diff_tool
	$(MAKE) clean

clean:
	rm -f diff_tool server
	rm -rf *.dSYM