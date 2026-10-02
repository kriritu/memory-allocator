CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = src/mm.c
TEST = tests/test_mm.c

test: $(SRC) $(TEST)
	$(CC) $(CFLAGS) -Isrc $(SRC) $(TEST) -o test_mm
	./test_mm

test-bestfit: $(SRC) $(TEST)
	$(CC) $(CFLAGS) -DPLACEMENT_POLICY=1 -Isrc $(SRC) $(TEST) -o test_mm_bestfit
	./test_mm_bestfit

test-frag: $(SRC) tests/test_fragmentation.c
	$(CC) $(CFLAGS) -Isrc $(SRC) tests/test_fragmentation.c -o test_frag
	./test_frag

test-frag-bestfit: $(SRC) tests/test_fragmentation.c
	$(CC) $(CFLAGS) -DPLACEMENT_POLICY=1 -Isrc $(SRC) tests/test_fragmentation.c -o test_frag_bestfit
	./test_frag_bestfit

valgrind-test: $(SRC) $(TEST)
	$(CC) $(CFLAGS) -Isrc $(SRC) $(TEST) -o test_mm
	valgrind --leak-check=full --show-leak-kinds=all ./test_mm

asan-test: $(SRC) $(TEST)
	$(CC) $(CFLAGS) -fsanitize=address -Isrc $(SRC) $(TEST) -o test_mm_asan
	./test_mm_asan

clean:
	rm -f test_mm test_mm_bestfit *.o
