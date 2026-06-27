CC      := gcc
CFLAGS  := -g -Wall -Wextra -std=c11

LIB_SRC := src/resp.c

# Two programs, each with its own main():
#   test_resp – runs the parser + command tests and EXITS (used for run/asan/leaks/check)
#   server    – the TCP server (blocks forever in its accept loop)
test_resp_SRCS := $(LIB_SRC) src/server.c tests/test_resp.c tests/test_server.c tests/test_run.c
server_SRCS    := $(LIB_SRC) src/server.c src/server_main.c src/db.c src/utils.c

# Which program run/asan/leaks/check act on. Override with:  make run BIN=server
BIN ?= test_resp

.PHONY: all
all: test_resp server

test_resp: $(test_resp_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

server: $(server_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

# Build + run the selected program.
.PHONY: run test
run test: $(BIN)
	./$(BIN)

# Run the tests, then (only if they pass) start the server. Blocks on :6379 (Ctrl+C).
.PHONY: both
both: test_resp server
	./test_resp && ./server

# AddressSanitizer (UAF/overflow/double-free). Don't point leaks at this binary.
.PHONY: asan
asan:
	$(CC) $(CFLAGS) -fsanitize=address $($(BIN)_SRCS) -o $(BIN)_asan && ./$(BIN)_asan

# macOS leak check at exit (run on a plain binary, never an ASan one).
.PHONY: leaks
leaks: $(BIN)
	leaks --atExit -- ./$(BIN)

# Full pass: ASan + leaks. Default BIN=test_resp (don't run this on the server —
# it blocks, so the tools would hang).
.PHONY: check
check: asan leaks
	@echo "All checks passed for $(BIN): ASan clean, 0 leaks."

.PHONY: clean
clean:
	rm -f test_resp server test_resp_asan server_asan app app_asan a a_asan
