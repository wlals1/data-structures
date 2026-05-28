CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=c11 -g -O0
TARGET  = test
OBJS    = main.o rb_tree.o
HEADERS = rb_tree.h

.PHONY: all run clean valgrind

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f *.o $(TARGET)
