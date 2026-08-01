CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = src/mm.c
TEST = tests/test_mm.c

test: $(SRC) $(TEST)
	$(CC) $(CFLAGS) -Isrc $(SRC) $(TEST) -o test_mm
	./test_mm

clean:
	rm -f test_mm *.o