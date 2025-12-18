/* ================================================
 * kernel/memory.c
 * ================================================ */
#include "memory.h"
#include "kernel.h"

#define HEAP_SIZE 0x100000  // 1MB

static uint8_t heap[HEAP_SIZE] __attribute__((aligned(16)));
static mem_block_t *first_block = NULL;
static bool memory_initialized = false;

void memory_init(void) {
    first_block = (mem_block_t*)heap;
    first_block->size = HEAP_SIZE - sizeof(mem_block_t);
    first_block->used = false;
    first_block->next = NULL;
    memory_initialized = true;
}

static mem_block_t *find_free_block(size_t size) {
    mem_block_t *current = first_block;
    while (current) {
        if (!current->used && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void split_block(mem_block_t *block, size_t size) {
    if (block->size >= size + sizeof(mem_block_t) + 16) {
        mem_block_t *new_block = (mem_block_t*)((uint8_t*)block + sizeof(mem_block_t) + size);
        new_block->size = block->size - size - sizeof(mem_block_t);
        new_block->used = false;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

void *kmalloc(size_t size) {
    if (!memory_initialized || size == 0) return NULL;
    size = (size + 3) & ~3;
    mem_block_t *block = find_free_block(size);
    if (!block) return NULL;
    split_block(block, size);
    block->used = true;
    return (void*)((uint8_t*)block + sizeof(mem_block_t));
}

void kfree(void *ptr) {
    if (!ptr || !memory_initialized) return;
    mem_block_t *block = (mem_block_t*)((uint8_t*)ptr - sizeof(mem_block_t));
    block->used = false;
}

void memory_stats(uint32_t *total, uint32_t *used, uint32_t *free) {
    *total = HEAP_SIZE;
    *used = 0;
    *free = 0;
    mem_block_t *current = first_block;
    while (current) {
        if (current->used) {
            *used += current->size + sizeof(mem_block_t);
        } else {
            *free += current->size + sizeof(mem_block_t);
        }
        current = current->next;
    }
}
