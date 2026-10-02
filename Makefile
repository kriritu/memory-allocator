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

clean:
	rm -f test_mm test_mm_bestfit *.o