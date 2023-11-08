#include "linear_allocator.h"

#include "core/kmemory.h"
#include "core/logger.h"

void linear_allocator_create(u64 total_size, void* memory, linear_allocator* out_allocator) {
    if (!out_allocator) {
        return;
    }
    out_allocator->total_size = total_size;
    out_allocator->allocated = 0;
    if (memory) {
        out_allocator->memory = memory;
        out_allocator->owns_memory = true;
    } else {
        out_allocator->owns_memory = false;
        out_allocator->memory = kallocate(total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
}

void linear_allocator_destroy(linear_allocator* allocator) {
    if (!allocator) {
        return;
    }
    if (allocator->owns_memory && allocator->memory) {
        kfree(allocator->memory, allocator->total_size, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
    allocator->memory = NULL;
    allocator->allocated = 0;
    allocator->total_size = 0;
    allocator->owns_memory = false;
}

void* linear_allocator_allocate(linear_allocator* allocator, u64 size) {
    if (!allocator || !allocator->memory) {
        KERROR("linear_allocator_allocate - provided allocator not initialized");
        return NULL;
    }

    if (allocator->allocated + size > allocator->total_size) {
        u64 remaining = allocator->total_size - allocator->allocated;
        KERROR("linear_allocator_allocate tried to allocate %lluB when only %lluB remained", size, remaining);
        return NULL;
    }

    void* block = allocator->memory + allocator->allocated;
    allocator->allocated += size;
    return block;
}

void linear_allocator_free_all(linear_allocator* allocator) {
    if (allocator && allocator->memory) {
        allocator->allocated = 0;
        // TODO: Should we do an owns_memory check here?
        kzero_memory(allocator->memory, allocator->total_size);
    }
}