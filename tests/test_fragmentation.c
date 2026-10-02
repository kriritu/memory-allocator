#define _DEFAULT_SOURCE
#include "mm.h"
#include <stdio.h>

static void print_block(const char *label, void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    size_t alloc = GET_ALLOC(HDRP(bp));
    printf("%-12s bp=%p size=%zu alloc=%zu\n", label, bp, size, alloc);
}

static void walk_heap(void *heap_start) {
    void *bp;
    printf("\n--- Full heap walk ---\n");
    for (bp = heap_start; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        print_block("blk", bp);
    }
    print_block("epilogue", bp);
}

int main(void) {
    void *heap_start = mm_init();

    void *A = mm_malloc(200);
    void *B = mm_malloc(32);
    void *C = mm_malloc(400);
    void *D = mm_malloc(32);
    void *E = mm_malloc(100);
    void *F = mm_malloc(32);
    (void)B; (void)D; (void)F;

    printf("--- Before freeing ---\n");
    print_block("A", A);
    print_block("C", C);
    print_block("E", E);

    mm_free(A);
    mm_free(C);
    mm_free(E);

    printf("\n--- Holes available ---\n");
    print_block("A-hole", A);
    print_block("C-hole", C);
    print_block("E-hole", E);

    walk_heap(heap_start);  /* <-- NEW: see exact state right before the risky call */

    printf("\n--- Requesting a block that fits in all 3 holes ---\n");
    void *X = mm_malloc(80);
    if (X == NULL) {
        printf("mm_malloc(80) returned NULL!\n");
        return 1;
    }
    print_block("X placed at", X);

    if (X == A) printf("-> A-hole (first-fit)\n");
    else if (X == C) printf("-> C-hole\n");
    else if (X == E) printf("-> E-hole (best-fit)\n");
    else printf("-> somewhere else\n");

    return 0;
}