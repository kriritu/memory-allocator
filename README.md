# Custom Memory Allocator in C

A from-scratch implementation of `malloc()`, `free()`, and `realloc()` in C,
using `sbrk()` and `mmap()` directly — no standard library heap functions.

## Status: In development — heap init and mm_malloc implemented

## Features
- [x] Heap initialization with prologue/epilogue sentinel blocks
- [x] Implicit free list with header/footer boundary tags
- [x] `mm_malloc()` — first-fit search with block splitting
- [x] `mm_free()` with O(1) coalescing of adjacent free blocks
- [x] Best-fit placement policy (benchmarked against first-fit)
- [x] `mm_realloc()`
- [ ] Large-allocation path via `mmap()`
- [ ] Valgrind / AddressSanitizer validation
- [ ] Fragmentation comparison: first-fit vs best-fit
     ## Fragmentation Comparison: First-Fit vs Best-Fit

Test: create 3 free "holes" of sizes 224, 416, and 128 bytes (separated by
allocated blocks), then request a 96-byte allocation that fits in all three.

| Policy    | Block chosen | Size  | Leftover fragment | Larger holes preserved? |
|-----------|-------------|-------|--------------------|--------------------------|
| First-fit | A-hole      | 224   | 128 bytes          | No — took the biggest hole |
| Best-fit  | E-hole      | 128   | 32 bytes           | Yes — 224 and 416 holes left intact |

First-fit is faster per allocation (stops at the first match) but can
consume large free blocks unnecessarily. Best-fit scans the entire free
list but preserves larger blocks for future large requests, reducing
fragmentation at the cost of search time.

## Design

Each block (free or allocated) is wrapped in an 8-byte header and 8-byte
footer encoding its size and allocation status, packed into a single word
(`size | alloc_bit`). This "boundary tag" technique allows O(1) traversal
in both directions across the heap.

Sentinel prologue and epilogue blocks bookend the heap so that
forward/backward traversal never reads out of bounds.

## Build & Test
\`\`\`bash
make test
\`\`\`

## Progress Log
- Heap setup via `sbrk()`: prologue/epilogue sentinel blocks, `extend_heap()`
- `mm_malloc()`: first-fit search, block splitting, heap growth on miss
- Best-fit placement policy (`find_fit_best`) added alongside first-fit, switchable at compile time via `-DPLACEMENT_POLICY=1`; both verified against the full test suite

## Reference
Based on the allocator design in *Computer Systems: A Programmer's Perspective*, §9.9.