#include "containers/darray.h"

#include "core/kmemory.h"
#include "core/logger.h"

void* _darray_create(u64 length, u64 element_size) {
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * element_size;
    u64* new_array = kallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_ELEMENT_SIZE] = element_size;
    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_ELEMENT_SIZE];
    kfree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darray_resize(void* array) {
     u64 length = darray_length(array);
     u64 element_size = darray_element_size(array);
     void* temp = _darray_create(darray_capacity(array) * DARRAY_RESIZE_FACTOR, element_size);
     kcopy_memory(temp, array, length * element_size);

     _darray_field_set(temp, DARRAY_LENGTH, length);
     _darray_destroy(array);
     return temp;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 element_size = darray_element_size(array);
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }
    u64 addr = (u64)array;
    addr += length * element_size;
    kcopy_memory((void*)addr, value_ptr, element_size);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 element_size = darray_element_size(array);
    u64 addr = (u64)array;
    addr += (length - 1) * element_size;
    kcopy_memory(dest, (void*)addr, element_size);
    darray_field_set(array, DARRAY_LENGTH, length - 1);
}


void* _darray_pop_at(void* array, u64 index, void* dest) {
    u64 length = darray_length(array);
    if (index >= length) {
        KERROR("Index out the bounds of the array Length: %i Index: %i", length, index);
        return array;
    }
    u64 addr = (u64)array;
    u64 element_size = darray_element_size(array);
    kcopy_memory(dest, (void*)(addr + (index * element_size)), element_size);


    // TODO: Maybe make this a swap and remove back?
    // If not on the last element, snip out the entry and copy the rest inward
    if (index != length - 1) {
        kcopy_memory(
            (void*)(addr + (index * element_size)),
            (void*)(addr + ((index + 1) * element_size)),
            (length - index ) * element_size
        );
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}

void* _darray_insert_at(void* array, u64 index, void* value_ptr) {
    u64 length = darray_length(array);
    if (index > length) {
        KERROR("Index out the bounds of the array Length: %i Index: %i", length, index);
        return array;
    }

    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    u64 element_size = darray_element_size(array);
    if(index != length) {
        kcopy_memory(
            (void*)(addr + ((index + 1) * element_size)),
            (void*)(addr + (index * element_size)),
            (length - index) * element_size
        );
    }
    kcopy_memory((void*)(addr + (index * element_size)), value_ptr, element_size);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}
