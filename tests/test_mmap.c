#define _DEFAULT_SOURCE
#include "mm.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    void *heap_start = mm_init();
    if (heap_start == NULL) {
        printf("mm_init failed\n");
        return 1;
    }

    printf("--- Testing mm_malloc with large request (should use mmap) ---\n");
    size_t big_size = 200000; /* 200KB, above MMAP_THRESHOLD of 128KB */
    void *big = mm_malloc(big_size);
    if (big == NULL) {
        printf("mm_malloc(200000) failed\n");
        return 1;
    }
    printf("big allocation succeeded, bp=%p\n", big);

    /* write and read back a pattern across the whole region to confirm it's really usable memory */
    memset(big, 'Z', big_size);
    int content_ok = 1;
    for (size_t i = 0; i < big_size; i++) {
        if (((char *)big)[i] != 'Z') {
            content_ok = 0;
            break;
        }
    }
    printf("Full region writable/readable? %s\n", content_ok ? "yes" : "no");

    printf("\n--- Freeing the mmap'd block ---\n");
    mm_free(big);
    printf("mm_free on mmap block completed without crash\n");

    printf("\n--- Confirming small allocations still work normally after mmap use ---\n");
    void *small = mm_malloc(64);
    if (small == NULL) {
        printf("mm_malloc(64) failed\n");
        return 1;
    }
    printf("small allocation bp=%p size=%zu alloc=%zu\n",
           small, GET_SIZE(HDRP(small)), GET_ALLOC(HDRP(small)));
    mm_free(small);

    printf("\nAll mmap tests completed.\n");
    return 0;
}
