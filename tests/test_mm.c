#define _DEFAULT_SOURCE
#include "mm.h"
#include <stdio.h>
#include <unistd.h>

static void print_block(const char *label, void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    size_t alloc = GET_ALLOC(HDRP(bp));
    printf("%-10s bp=%p size=%zu alloc=%zu\n", label, bp, size, alloc);
}

int main(void) {
    printf("Heap break before init: %p\n", sbrk(0));

    void *heap_start = mm_init();
    if (heap_start == NULL) {
        printf("mm_init failed\n");
        return 1;
    }

    printf("Heap break after init:  %p\n", sbrk(0));
    printf("heap_listp:             %p\n\n", heap_start);

    /* heap_listp points at the prologue's "payload" position (between header/footer) */
    print_block("prologue", heap_start);

    void *first_free = NEXT_BLKP(heap_start);
    print_block("free blk", first_free);

    void *epilogue = NEXT_BLKP(first_free);
    print_block("epilogue", epilogue);

    printf("\n--- Testing mm_malloc(24) ---\n");
    void *p1 = mm_malloc(24);
    if (p1 == NULL) {
        printf("mm_malloc(24) failed\n");
        return 1;
    }
    print_block("p1", p1);

    void *new_free = NEXT_BLKP(p1);
    print_block("free blk", new_free);

    printf("\n--- Testing mm_malloc(100) ---\n");
    void *p2 = mm_malloc(100);
    if (p2 == NULL) {
        printf("mm_malloc(100) failed\n");
        return 1;
    }
    print_block("p2", p2);

    void *free_blk2 = NEXT_BLKP(p2);
    print_block("free blk", free_blk2);

    printf("\n--- Testing mm_malloc(4000) ---\n");
    void *heap_break_before = sbrk(0);
    void *p3 = mm_malloc(4000);
    if (p3 == NULL) {
        printf("mm_malloc(4000) failed\n");
        return 1;
    }
    void *heap_break_after = sbrk(0);
    print_block("p3", p3);
    printf("heap break before: %p, after: %p (grew: %s)\n",
        heap_break_before, heap_break_after,
        heap_break_after > heap_break_before ? "yes" : "no");

    return 0;
}