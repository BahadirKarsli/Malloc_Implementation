#include "mymalloc.h"
#include <stdio.h>
#include <unistd.h> // for sbrk()

Strategy strategy = FIRST_FIT; // Default strategy
ListType listtype = UNORDERED_LIST; // Default to LIFO for simplicity
Block *free_list = NULL;
Block *heap_start = NULL;
Block *heap_end = NULL;
Block *last_freed = NULL;

/** Allocates memory and returns a pointer to the data[] field */
void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    // Calculate required size in 16-byte blocks
    size_t size_in_16blocks = numberof16blocks(size);
    if (size_in_16blocks == 0) size_in_16blocks = 1; // Minimum 1 block

    // Initialize heap if not already done
    if (free_list == NULL) {
        heap_start = (Block *)sbrk(HEAP_SIZE);
        if (heap_start == (void *)-1) {
            return NULL; // sbrk failed
        }
        heap_end = heap_start;
        heap_start->info.size = numberof16blocks(HEAP_SIZE - sizeof(Block));
        heap_start->info.isfree = 1;
        heap_start->next = NULL;
        heap_start->prev = NULL;
        free_list = heap_start;
        last_freed = heap_start;
    }

    // Find a suitable block (First Fit strategy)
    Block *block = free_list;
    while (block != NULL) {
        if (block->info.isfree && block->info.size >= size_in_16blocks) {
            break;
        }
        block = block->next;
    }

    // If no block found, extend the heap
    if (block == NULL) {
        block = (Block *)sbrk(HEAP_SIZE);
        if (block == (void *)-1) {
            return NULL; // sbrk failed
        }
        block->info.size = numberof16blocks(HEAP_SIZE - sizeof(Block));
        block->info.isfree = 1;
        block->next = free_list;
        block->prev = NULL;
        if (free_list != NULL) {
            free_list->prev = block;
        }
        free_list = block;
        heap_end = block;
    }

    // Split block if it's larger than needed
    if (block->info.size > size_in_16blocks + 1) { // +1 for metadata of new block
        block = split_block(block, size_in_16blocks);
    }

    // Mark block as allocated and remove from free list
    block->info.isfree = 0;
    if (block->prev) block->prev->next = block->next;
    if (block->next) block->next->prev = block->prev;
    if (block == free_list) free_list = block->next;
    block->next = NULL;
    block->prev = NULL;

    return block->data; // Return pointer to data[]
}

/** Frees a block and adds it back to the free list */
void myfree(void *p) {
    if (p == NULL) return;

    // Get the block from the data pointer
    Block *block = (Block *)((char *)p - offsetof(Block, data));
    block->info.isfree = 1;

    // Coalesce with adjacent blocks
    block = left_coalesce(block);
    block = right_coalesce(block);

    // Add to free list (LIFO)
    block->next = free_list;
    block->prev = NULL;
    if (free_list != NULL) {
        free_list->prev = block;
    }
    free_list = block;
    last_freed = block;
}

/** Splits a block into two, returning the allocated portion */
Block *split_block(Block *b, size_t size) {
    if (b->info.size <= size + 1) return b; // Not enough space to split

    Block *new_block = (Block *)((char *)b + sizeof(Block) + size * 16);
    new_block->info.size = b->info.size - size - 1; // Remaining size
    new_block->info.isfree = 1;
    new_block->next = b->next;
    new_block->prev = b->prev;

    // Update free list pointers
    if (b->prev) b->prev->next = new_block;
    if (b->next) b->next->prev = new_block;
    if (b == free_list) free_list = new_block;

    b->info.size = size;
    b->next = NULL;
    b->prev = NULL;

    return b;
}

/** Coalesces block with its left neighbor if free */
Block *left_coalesce(Block *b) {
    Block *left = prev_block_in_addr(b);
    if (left != NULL && left->info.isfree) {
        left->info.size += b->info.size + 1; // +1 for Block header
        if (b->next) b->next->prev = b->prev;
        if (b->prev) b->prev->next = b->next;
        if (b == free_list) free_list = b->next;
        return left;
    }
    return b;
}

/** Coalesces block with its right neighbor if free */
Block *right_coalesce(Block *b) {
    Block *right = next_block_in_addr(b);
    if (right != NULL && right->info.isfree) {
        b->info.size += right->info.size + 1; // +1 for Block header
        b->next = right->next;
        if (right->next) right->next->prev = b;
        if (right == free_list) free_list = b;
        return b;
    }
    return b;
}

/** Returns the next block in the free list */
Block *next_block_in_freelist(Block *b) {
    return b ? b->next : NULL;
}

/** Returns the previous block in the free list */
Block *prev_block_in_freelist(Block *b) {
    return b ? b->prev : NULL;
}

/** Returns the next block in memory address order */
Block *next_block_in_addr(Block *b) {
    Block *next = (Block *)((char *)b + sizeof(Block) + b->info.size * 16);
    if ((void *)next < (void *)sbrk(0)) return next;
    return NULL;
}

/** Returns the previous block in memory address order */
Block *prev_block_in_addr(Block *b) {
    if (b == heap_start) return NULL;
    Block *current = heap_start;
    while (current != NULL && next_block_in_addr(current) != b) {
        current = next_block_in_addr(current);
    }
    return current;
}

/** Calculates the number of 16-byte blocks needed */
uint64_t numberof16blocks(size_t size_inbytes) {
    return (size_inbytes + 15) / 16; // Round up
}

/** Prints the heap's metadata */
void printheap() {
    Block *current = heap_start;
    while (current != NULL) {
        printf("---------------\n");
        printf("Free: %d\n", current->info.isfree);
        printf("Size: %" PRIu64 "\n", current->info.size * 16); // Size in bytes
        current = next_block_in_addr(current);
    }
    printf("---------------\n");
}

ListType getlisttype() {
    return listtype;
}

int setlisttype(ListType lt) {
    listtype = lt;
    return 1;
}

Strategy getstrategy() {
    return strategy;
}

int setstrategy(Strategy s) {
    strategy = s;
    return 1;
}