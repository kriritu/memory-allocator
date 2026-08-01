#ifndef MM_H
#define MM_H

#include <stddef.h>

/* Basic size constants */
#define WSIZE 8            /* word size (bytes) — header/footer size */
#define DSIZE 16           /* double word size (bytes) — alignment unit */
#define CHUNKSIZE (1<<12)  /* extend heap by this many bytes (4KB) when more room is needed */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(size_t *)(p))
#define PUT(p, val)  (*(size_t *)(p) = (val))

/* Read the size and allocated fields from address p (a header or footer) */
#define GET_SIZE(p)  (GET(p) & ~0xF)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp (pointer to payload), compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

void *mm_init(void);
void *mm_malloc(size_t size);

#endif