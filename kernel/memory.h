#ifndef MEMORY_H
#define MEMORY_H

#include "../include/types.h"

typedef struct mem_block {
    size_t size;
    bool used;
    struct mem_block *next;
} mem_block_t;

void memory_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void memory_stats(uint32_t *total, uint32_t *used, uint32_t *free);

#endif
