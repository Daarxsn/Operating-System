/*
 * heap.c
 * XyrisOS Kernel
 *
 * Growing first-fit kernel heap backed by physical pages through the HHDM.
 */

#include "heap.h"
#include "pmm.h"
#include "hhdm.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEAP_SEGMENT_PAGES 32
#define HEAP_ALIGNMENT 16

#define HEAP_SEGMENT_SIZE \
    ((size_t)HEAP_SEGMENT_PAGES * PAGE_SIZE)

typedef struct __attribute__((aligned(16))) heap_block
{
    size_t size;
    bool free;
    struct heap_block *next;
} heap_block_t;

typedef struct heap_segment
{
    uint8_t *start;
    uint8_t *end;
    heap_block_t *first;
    struct heap_segment *next;
} heap_segment_t;

static heap_segment_t *heap_segments = NULL;

static size_t align16(size_t size)
{
    if (size > SIZE_MAX - (HEAP_ALIGNMENT - 1))
        return 0;

    return (size + (HEAP_ALIGNMENT - 1)) &
           ~(size_t)(HEAP_ALIGNMENT - 1);
}

static heap_block_t *block_from_payload(void *ptr)
{
    if (ptr == NULL)
        return NULL;

    for (heap_segment_t *segment = heap_segments;
         segment != NULL;
         segment = segment->next)
    {
        for (heap_block_t *block = segment->first;
             block != NULL;
             block = block->next)
        {
            if ((void *)(block + 1) == ptr)
                return block;
        }
    }

    return NULL;
}

static void split_block(heap_block_t *block, size_t size)
{
    if (block == NULL || block->size < size)
        return;

    size_t remaining = block->size - size;

    if (remaining <= sizeof(heap_block_t) + HEAP_ALIGNMENT)
        return;

    heap_block_t *next =
        (heap_block_t *)((uint8_t *)(block + 1) + size);

    next->size = remaining - sizeof(heap_block_t);
    next->free = true;
    next->next = block->next;

    block->size = size;
    block->next = next;
}

static void coalesce_segment(heap_segment_t *segment)
{
    if (segment == NULL)
        return;

    heap_block_t *block = segment->first;

    while (block != NULL && block->next != NULL)
    {
        heap_block_t *next = block->next;

        uint8_t *block_end =
            (uint8_t *)(block + 1) + block->size;

        if (block->free && next->free &&
            block_end == (uint8_t *)next)
        {
            block->size +=
                sizeof(heap_block_t) + next->size;

            block->next = next->next;
            continue;
        }

        block = next;
    }
}

static heap_segment_t *create_segment(void)
{
    phys_addr_t physical =
        pmm_alloc_pages(HEAP_SEGMENT_PAGES);

    if (physical == 0)
        return NULL;

    uint8_t *start =
        (uint8_t *)phys_to_virt(physical);

    heap_segment_t *segment =
        (heap_segment_t *)start;

    size_t header_size =
        (sizeof(heap_segment_t) + HEAP_ALIGNMENT - 1) &
        ~(size_t)(HEAP_ALIGNMENT - 1);

    if (header_size + sizeof(heap_block_t) + HEAP_ALIGNMENT >=
        HEAP_SEGMENT_SIZE)
    {
        pmm_free_pages(physical, HEAP_SEGMENT_PAGES);
        return NULL;
    }

    segment->start = start;
    segment->end = start + HEAP_SEGMENT_SIZE;
    segment->next = NULL;

    segment->first =
        (heap_block_t *)(start + header_size);

    segment->first->size =
        HEAP_SEGMENT_SIZE -
        header_size -
        sizeof(heap_block_t);

    segment->first->free = true;
    segment->first->next = NULL;

    return segment;
}

void heap_init(void)
{
    if (heap_segments != NULL)
        return;

    heap_segment_t *segment = create_segment();

    if (segment == NULL)
        return;

    heap_segments = segment;
}

void *kmalloc(size_t size)
{
    size_t aligned = align16(size);

    if (aligned == 0)
        return NULL;

    if (heap_segments == NULL)
        heap_init();

    for (;;)
    {
        for (heap_segment_t *segment = heap_segments;
             segment != NULL;
             segment = segment->next)
        {
            for (heap_block_t *block = segment->first;
                 block != NULL;
                 block = block->next)
            {
                if (!block->free || block->size < aligned)
                    continue;

                split_block(block, aligned);
                block->free = false;

                return (void *)(block + 1);
            }
        }

        heap_segment_t *segment = create_segment();

        if (segment == NULL)
            return NULL;

        segment->next = heap_segments;
        heap_segments = segment;
    }
}

void *kcalloc(size_t count, size_t size)
{
    if (count != 0 &&
        size > SIZE_MAX / count)
    {
        return NULL;
    }

    size_t total = count * size;

    if (total == 0)
        return NULL;

    uint8_t *ptr = (uint8_t *)kmalloc(total);

    if (ptr == NULL)
        return NULL;

    for (size_t i = 0; i < total; i++)
        ptr[i] = 0;

    return ptr;
}

void *krealloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return kmalloc(size);

    if (size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = block_from_payload(ptr);

    if (block == NULL || block->free)
        return NULL;

    size_t aligned = align16(size);

    if (aligned == 0)
        return NULL;

    if (block->size >= aligned)
    {
        split_block(block, aligned);
        return ptr;
    }

    void *replacement = kmalloc(size);

    if (replacement == NULL)
        return NULL;

    size_t copy_size = block->size;

    if (copy_size > size)
        copy_size = size;

    uint8_t *dst = (uint8_t *)replacement;
    uint8_t *src = (uint8_t *)ptr;

    for (size_t i = 0; i < copy_size; i++)
        dst[i] = src[i];

    kfree(ptr);

    return replacement;
}

void kfree(void *ptr)
{
    if (ptr == NULL || heap_segments == NULL)
        return;

    for (heap_segment_t *segment = heap_segments;
         segment != NULL;
         segment = segment->next)
    {
        for (heap_block_t *block = segment->first;
             block != NULL;
             block = block->next)
        {
            if ((void *)(block + 1) != ptr)
                continue;

            if (block->free)
                return;

            block->free = true;
            coalesce_segment(segment);
            return;
        }
    }
}
