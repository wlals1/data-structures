CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g -O0
HEADERS = hash.h

.PHONY: all run-open run-chaining clean valgrind-open valgrind-chaining

all: hash_open hash_chaining

# 두 실행파일: main.o를 공유, 해시 구현만 갈아끼움
hash_open: main.o hash_open.o
	$(CC) $(CFLAGS) -o $@ $^

hash_chaining: main.o hash_chaining.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run-open: hash_open
	./hash_open

run-chaining: hash_chaining
	./hash_chaining

valgrind-open: hash_open
	valgrind --leak-check=full --error-exitcode=1 ./hash_open

valgrind-chaining: hash_chaining
	valgrind --leak-check=full --error-exitcode=1 ./hash_chaining

clean:
	rm -f *.o hash_open hash_chaining