CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11 -g -O0
SAN     := -fsanitize=address,undefined
SRC     := b_tree.c main.c
BIN     := btree

.PHONY: all run stress valgrind asan clean

all: $(BIN)

$(BIN): $(SRC) b_tree.h
	$(CC) $(CFLAGS) $(SRC) -o $@

run: $(BIN)
	./$(BIN)

# Reproducible stress run with a fixed seed.
stress: $(BIN)
	./$(BIN) 12345

# Memory check under valgrind.
valgrind: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(BIN)

# Build and run under AddressSanitizer + UndefinedBehaviorSanitizer.
asan: $(SRC) b_tree.h
	$(CC) $(CFLAGS) $(SAN) $(SRC) -o $(BIN)_asan
	./$(BIN)_asan

clean:
	rm -f $(BIN) $(BIN)_asan
