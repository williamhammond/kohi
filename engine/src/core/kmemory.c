#include "kmemory.h"

#include "core/logger.h"

#include "platform/platform.h"
#include <stdio.h>
// TODO: Use string utilities
#include <string.h>

struct memory_stats {
    u64 total_allocated;
    u64 tagged_allocations[MEMORY_TAG_MAX_TAGS];
};

static const char* memory_tag_strings[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN         ",
    "ARRAY           ",
    "LINEAR_ALLOCATOR",
    "DARRAY          ",
    "DICT            ",
    "RING_QUEUE      ",
    "BST             ",
    "STRING          ",
    "APPLICATION     ",
    "JOB             ",
    "TEXTURE         ",
    "MAT_INST        ",
    "RENDERER        ",
    "GAME            ",
    "TRANSFORM       ",
    "ENTITY          ",
    "ENTITY_NODE     ",
};

typedef struct memory_system_state {
    struct memory_stats stats;
    u64 alloc_count;
} memory_system_state;

static memory_system_state* state_ptr;

void initialize_memory(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(memory_system_state);
    if (state == NULL) {
        return;
    }
    state_ptr = state;
    state_ptr->alloc_count = 0;
    platform_zero_memory(&state_ptr->stats, sizeof(state_ptr->stats));
}

void shutdown_memory(void* state) {
    state_ptr = NULL;
}

KAPI void* kallocate(u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Reclassify this allocation");
    }

    // TODO: Do we need to initalize memory earlier so this isn't necessary?
    if (state_ptr) {
        state_ptr->stats.total_allocated += size;
        state_ptr->stats.tagged_allocations[tag] += size;
        state_ptr->alloc_count++;
    }

    // TODO: Memory alignment
    void* block = platform_allocate(size, false);
    platform_zero_memory(block, size);
    return block;
}

KAPI void kfree(void* block, u64 size, memory_tag tag) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        KWARN("kallocate called using MEMORY_TAG_UNKNOWN. Reclassify this allocation");
    }

    state_ptr->stats.total_allocated -= size;
    state_ptr->stats.tagged_allocations[tag] -= size;
    // TODO: Memory alignment
    platform_free(block, false);
}

KAPI void* kzero_memory(void* block, u64 size) {
    return platform_zero_memory(block, size);
}

KAPI void* kcopy_memory(void* dest, const void* source, u64 size) {
    return platform_copy_memory(dest, source, size);
}

KAPI void* kset_memory(void* dest, i32 value, u64 size) {
    return platform_set_memory(dest, value, size);
}

// Useful for debugging
KAPI char* get_memory_usage_str() {
    const u64 kib = 1024;
    const u64 mib = 1024 * 1024;
    const u64 gib = 1024 * 1024 * 1024;

#define buffer_size 8000
    // static const u64 buffer_size = 8000;
    char buffer[buffer_size] = "System memory use (tagged): \n";
    u64 offset = strlen(buffer);
    for (u32 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
        char unit[4] = "XiB";
        float amount = 1.0f;
        if (state_ptr->stats.tagged_allocations[i] >= gib) {
            amount = (float)state_ptr->stats.tagged_allocations[i] / (float)gib;
            unit[0] = 'G';
        } else if (state_ptr->stats.tagged_allocations[i] >= mib) {
            amount = (float)state_ptr->stats.tagged_allocations[i] / (float)mib;
            unit[0] = 'M';
        } else if (state_ptr->stats.tagged_allocations[i] >= kib) {
            amount = (float)state_ptr->stats.tagged_allocations[i] / (float)kib;
            unit[0] = 'K';
        } else {
            amount = (float)state_ptr->stats.tagged_allocations[i];
            unit[0] = 'B';
            unit[1] = 0;
        }
        i32 length = snprintf(buffer + offset, buffer_size - offset, "%s: %.2f%s\n", memory_tag_strings[i], amount, unit);
        offset += length;
    }

    // TODO: Use custom allocator, use used a fixed size StringBuffer class
    char* out_string = _strdup(buffer);
    return out_string;
}

u64 get_memory_alloc_count() {
    if (state_ptr) {
        return state_ptr->alloc_count;
    }
    return 0;
}
