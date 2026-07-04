CC      := gcc
CFLAGS  := -g -Wall -Wextra -std=c11

TARGET  := ccsh
LS      := custom_cmd/ls
CD      := custom_cmd/cd
PWD      := custom_cmd/pwd

.PHONY: all run ls clean

all: $(TARGET)

$(TARGET): *.c $(CD).c
	$(CC) $(CFLAGS) $^ -o $@

$(LS): custom_cmd/ls.c
	$(CC) $(CFLAGS) $< -o $@

$(CD): custom_cmd/cd.c
	$(CC) $(CFLAGS) $< -o $@

$(PWD): custom_cmd/pwd.c
	$(CC) $(CFLAGS) $< -o $@

run: $(TARGET) $(LS) $(PWD)
	./$(TARGET)

ls: $(LS)
	./$(LS)

cd: $(CD)
	./$(CD)

pwd: $(PWD)
	./$(PWD)

clean:
	rm -f $(TARGET) $(LS)
	rm -f $(TARGET) $(CD)
	rm -f $(TARGET) $(PWD)
	rm -rf *.dSYM custom_cmd/*.dSYM