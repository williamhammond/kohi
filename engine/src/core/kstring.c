#include <core/kstring.h>
#include <core/kmemory.h>


#include <string.h>

u64 string_length(const char* str) {
    return strlen(str);
}

char* string_duplicate(const char* str) {
    u64 length = string_length(str);
    char* copy = kallocate(length + 1, MEMORY_TAG_STRING);
    kcopy_memory(copy, str, length);
    return copy;
}

b8 string_equal(const char* strA, const char* strB) {
    return strcmp(strA, strB) == 0;
}