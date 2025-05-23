# Memory Allocator

## 1. Explanation of External Fragmentation
----------------------------------------
External fragmentation occurs when free memory blocks are scattered throughout the heap, but none of them are large enough to satisfy a memory allocation request, even though the total free memory might be sufficient. In this implementation, I used the First Fit strategy for block allocation and a LIFO (Last In, First Out) organization for the free list.

With First Fit, the allocator searches the free list from the beginning and uses the first block that is large enough to satisfy the request. This can lead to external fragmentation because smaller blocks may remain unused at the end of the heap after larger blocks are allocated and split. For example, if a large block is split to allocate a small request, the remaining free portion might not be large enough for future larger requests, even though the total free space is significant. Over time, as blocks are allocated and freed without proper coalescing, the heap can become fragmented with small, non-contiguous free blocks.

To mitigate this, my implementation includes full coalescing (merging with both left and right adjacent free blocks when a block is freed). This reduces external fragmentation by combining adjacent free blocks into larger ones, making it more likely that future allocation requests can be satisfied. However, since First Fit doesn't prioritize finding the "best" or "worst" fit, some fragmentation may still occur compared to strategies like Best Fit, which could minimize leftover space more effectively.

## 2. Example Inputs and Outputs
-----------------------------
To demonstrate the allocator's behavior, I tested it with a simple program. Below are the inputs and the resulting outputs from the printheap() function.

Test Program:
#include "mymalloc.h"
int main() {
    void *p1 = mymalloc(32);  // Allocate 32 bytes
    printheap();
    void *p2 = mymalloc(64);  // Allocate 64 bytes
    printheap();
    myfree(p1);              // Free the first allocation
    printheap();
    return 0;
}

## Output 1: After mymalloc(32)
---------------
Free: 0
Size: 32
---------------
Free: 1
Size: 960
---------------

Explanation: The heap is initialized with 1024 bytes (HEAP_SIZE). The first mymalloc(32) request allocates a 32-byte block (2 * 16-byte blocks, including alignment). The remaining 960 bytes (1024 - 32 - 32 bytes for Block header) stay in a free block.

## Output 2: After mymalloc(64)
---------------
Free: 0
Size: 32
---------------
Free: 0
Size: 64
---------------
Free: 1
Size: 896
---------------

Explanation: The second mymalloc(64) request splits the 960-byte free block into a 64-byte allocated block (4 * 16-byte blocks) and a remaining 896-byte free block (960 - 64 - 32 bytes for the new Block header).

## Output 3: After myfree(p1)
---------------
Free: 1
Size: 32
---------------
Free: 0
Size: 64
---------------
Free: 1
Size: 896
---------------

Explanation: Freeing p1 (the 32-byte block) adds it back to the free list. Since it has no free left neighbor (it’s at the heap start) and its right neighbor (64-byte block) is allocated, no coalescing occurs. The heap now has two separate free blocks (32 bytes and 896 bytes), showing potential external fragmentation if larger allocations are requested later.

## 3. Observations
---------------
The First Fit strategy is simple and fast but can lead to fragmentation, as seen in the final state with two non-contiguous free blocks. Full coalescing helps by merging adjacent free blocks when possible, but fragmentation still depends on the allocation/free pattern. For instance, if p2 were freed next, the 32-byte and 64-byte blocks could coalesce into a larger free block, reducing fragmentation.

Optional features like Best Fit or address-ordered free list could further optimize fragmentation but were not implemented here to prioritize base functionality.
