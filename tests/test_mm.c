#define _DEFAULT_SOURCE
#include "mm.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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

    printf("\n--- Testing mm_free ---\n");
    printf("Freeing p2...\n");
    mm_free(p2);
    //print_block("after free p2", p2);

    printf("Freeing p1...\n");
    mm_free(p1);
    //print_block("after free p1", p1);

    printf("Freeing p3...\n");
    mm_free(p3);
    //print_block("after free p3", p3);

    /*printf("\n--- Full heap walk BEFORE any free ---\n");
    void *walk;
    for (walk = heap_start; GET_SIZE(HDRP(walk)) > 0; walk = NEXT_BLKP(walk)) {
        print_block("blk", walk);
    }
    print_block("epilogue", walk);*/

    printf("\n--- Full heap walk ---\n");
    void *bp;
    for (bp = heap_start; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        print_block("blk", bp);
    }
    print_block("epilogue", bp);

    printf("\n--- Testing mm_realloc(NULL, 50) ---\n");
    void *r1 = mm_realloc(NULL, 50);
    if (r1 == NULL) {
        printf("realloc(NULL, 50) failed\n");
        return 1;
    }
    print_block("r1", r1);

    printf("\n--- Testing mm_realloc shrink + content check ---\n");
    memset(r1, 'A', 50);  /* fill the payload with a known pattern */
    void *r2 = mm_realloc(r1, 20);  /* shrink */
    if (r2 == NULL) {
        printf("realloc shrink failed\n");
        return 1;
    }
    print_block("r2 (shrunk)", r2);
    printf("Same address as r1? %s\n", (r2 == r1) ? "yes" : "no");

    int content_ok = 1;
    for (int i = 0; i < 20; i++) {
        if (((char *)r2)[i] != 'A') {
            content_ok = 0;
            break;
        }
    }
    printf("Content preserved for first 20 bytes? %s\n", content_ok ? "yes" : "no");

    printf("\n--- Testing mm_realloc in-place grow ---\n");
    void *a = mm_malloc(32);
    void *b = mm_malloc(32);
    print_block("a", a);
    print_block("b", b);

    mm_free(b);  /* free b so a's right neighbor becomes free */
    print_block("after freeing b", NEXT_BLKP(a));

    memset(a, 'B', 32);
    void *a_grown = mm_realloc(a, 60);  /* grow — should fit using a + freed b's space */
    print_block("a_grown", a_grown);
    printf("Same address as a? %s\n", (a_grown == a) ? "yes" : "no");

    int a_content_ok = 1;
    for (int i = 0; i < 32; i++) {
        if (((char *)a_grown)[i] != 'B') {
            a_content_ok = 0;
            break;
        }
    }
    printf("Original content preserved? %s\n", a_content_ok ? "yes" : "no");

    printf("\n--- Testing mm_realloc fallback (copy) path ---\n");
    void *c = mm_malloc(32);
    void *d = mm_malloc(32);  /* d stays allocated — blocks in-place growth of c */
    print_block("c", c);
    print_block("d", d);

    memset(c, 'C', 32);
    void *c_grown = mm_realloc(c, 500);  /* too big to fit in c + (allocated) d */
    print_block("c_grown", c_grown);
    printf("Same address as c? %s (expect: no)\n", (c_grown == c) ? "yes" : "no");

    int c_content_ok = 1;
    for (int i = 0; i < 32; i++) {
        if (((char *)c_grown)[i] != 'C') {
            c_content_ok = 0;
            break;
        }
    }
    printf("Original content preserved after move? %s\n", c_content_ok ? "yes" : "no");

    return 0;
}