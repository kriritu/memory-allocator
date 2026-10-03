# Custom Memory Allocator in C

A from-scratch implementation of `malloc()`, `free()`, and `realloc()` in C,
built directly on `sbrk()` and `mmap()` — no standard library heap functions.

## Features

- [x] Heap initialization with prologue/epilogue sentinel blocks
- [x] Implicit free list with header/footer boundary tags
- [x] `mm_malloc()` — first-fit and best-fit placement (switchable at compile time)
- [x] `mm_free()` with O(1) coalescing of adjacent free blocks (all 4 cases)
- [x] `mm_realloc()` with in-place growth and copy fallback
- [x] Large-allocation path via `mmap()`/`munmap()` for requests ≥ 128KB
- [x] Validated with Valgrind and AddressSanitizer (0 errors across all tests)
- [x] Fragmentation comparison: first-fit vs best-fit

## Design

### Block layout
Each block (free or allocated) is wrapped in an 8-byte header and 8-byte
footer encoding size and allocation status, packed into one word
(`size | alloc_bit`). This "boundary tag" technique allows O(1) traversal
in both directions across the heap — essential for constant-time coalescing.

Sentinel prologue and epilogue blocks bookend the heap so forward/backward
traversal never reads out of bounds.

### Placement policy
Two strategies are implemented and switchable via a compile-time flag:
- **First-fit**: returns the first free block large enough. Fast — stops
  scanning as soon as a candidate is found.
- **Best-fit**: scans the entire free list and returns the smallest block
  that still fits. Slower per call, but reduces fragmentation by preserving
  larger free blocks for future large requests.

Build either with:
```bash
make test              # first-fit
make test-bestfit       # best-fit
```

### Large allocations
Requests ≥ 128KB bypass the implicit free list entirely and go straight to
`mmap()`, getting their own independent OS mapping. `mm_free()` detects
these and calls `munmap()`, returning the memory to the OS immediately —
something `sbrk()` alone can never do for memory in the middle of the heap.

### Realloc
`mm_realloc()` tries to grow in place first (absorbing a free next-neighbor
via the same boundary-tag check used in coalescing) before falling back to
allocate-new + copy + free-old. This avoids an unnecessary copy whenever
possible.

## Fragmentation Comparison: First-Fit vs Best-Fit

Test setup: create 3 free "holes" of sizes 224, 416, and 128 bytes
(separated by allocated blocks), then request a 96-byte allocation that
fits in all three.

| Policy    | Block chosen | Size | Leftover fragment | Larger holes preserved? |
|-----------|-------------|------|--------------------|--------------------------|
| First-fit | A-hole      | 224  | 128 bytes          | No — consumed the biggest hole |
| Best-fit  | E-hole      | 128  | 32 bytes           | Yes — 224 and 416 holes left intact |

First-fit is faster per allocation (stops at the first match) but can
consume large free blocks unnecessarily. Best-fit scans the entire free
list but preserves larger blocks for future large requests, trading
search time for lower fragmentation.

## Validation

Tested under both Valgrind (Memcheck) and AddressSanitizer across every
core operation — heap init, malloc under both placement policies, free
with full boundary-tag coalescing, realloc (in-place and fallback paths),
and the mmap large-allocation path. **0 errors detected by either tool.**

```bash
make valgrind-test   # Valgrind Memcheck
make asan-test        # AddressSanitizer
```

## Build & Test

```bash
make test              # build + run full test suite (first-fit)
make test-bestfit       # build + run with best-fit policy
make test-frag          # fragmentation comparison test (first-fit)
make test-frag-bestfit  # fragmentation comparison test (best-fit)
make test-mmap          # large-allocation (mmap) test
make valgrind-test       # Valgrind validation
make asan-test           # AddressSanitizer validation
make clean               # remove build artifacts
```

## Known Limitations / Future Improvements

- Implicit free list means `find_fit`/`find_fit_best` are O(n) in the
  number of blocks; an explicit free list (storing next/prev pointers in
  freed blocks) would make allocation faster at the cost of a larger
  minimum block size.
- The mmap-block detection in `mm_free()` reads a stored size value just
  before the payload pointer rather than using a dedicated tag bit — works
  correctly here, but a cleaner design would reserve an explicit flag bit
  in a proper header, consistent with the rest of the allocator.
- No thread safety — a real allocator would need locking or per-thread
  arenas for concurrent use.

## What I Learned

Building this from CS:APP §9.9 made the theory concrete in a way reading
alone doesn't. The hardest bug was a silent placement failure: `mm_malloc`
compiled and ran, but a working `find_fit` result was being discarded
because the `#if PLACEMENT_POLICY`/`#else`/`#endif` block's braces didn't
actually wrap the intended code — the `if` body was accidentally empty,
so every allocation fell through to `extend_heap` regardless of whether a
valid free block existed. Tracking it down meant verifying each layer
independently (NULL checks, `find_fit` in isolation, `place`'s arithmetic)
with gdb and targeted debug prints before finding the real cause — a good
reminder that "it compiles and mostly works" isn't the same as correct.

## Reference

Based on the allocator design in *Computer Systems: A Programmer's
Perspective* (Bryant & O'Hallaron), §9.9.