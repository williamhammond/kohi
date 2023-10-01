#pragma once

#include "defines.h"
/*
* Memory Layout
* u64 capacity = number of elements that can be held
* u64 length = number of elements currently held
* u64 element_size = size of each element
* void* elements
*/

enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_ELEMENT_SIZE,
    DARRAY_FIELD_LENGTH
};

// These are actually private helper methods, but we need to export them because
// they're used by the macros.
KAPI void* _darray_create(u64 length, u64 element_size);
KAPI void _darray_destroy(void* darray);

KAPI u64 _darray_field_get(void* array, u64 field);
KAPI void _darray_field_set(void* array, u64 field, u64 value);

KAPI void* _darray_resize(void* array);

KAPI void* _darray_push(void* array, const void* value_ptr);
KAPI void _darray_pop(void* array, void* dest);

KAPI void* _darray_pop_at(void* array, u64 index, void* dest);
KAPI void* _darray_insert_at(void* array, u64 index, void* dest);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define darray_create(type) \
    _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define darray_reserve(array, length) \
    _darray_create(capacity, sizeof(type))

#define darray_destroy(array) _darray_destroy(array)

// could use __auto_type for the temp variable, but intellisense
// for Vscode flags it as an unknown type. typeof() seems to work fine though.
// Both are GNU extensions.
#define darray_push(array, value)           \
    {                                       \
        typeof(value) temp = value;         \
        array = _darray_push(array, &temp); \
    }

#define darray_insert_at(array, index, value)       \
    {                                               \
        typeof(value) temp = value;                 \
        array = _darray_push(array, index, &temp);  \
    }

#define darray_pop_at(array, index, value)      \
    {                                           \
        _darray_pop_at(array, index, value);    \
    }


#define darray_field_set(array, field, value) _darray_field_set(array, field, value)

#define darray_clear(array) \
    _darray_field_set(array, DARRAY_LENGTH, 0)

#define darray_capacity(array) \
    _darray_field_get(array, DARRAY_CAPACITY)

#define darray_length(array) \
    _darray_field_get(array, DARRAY_LENGTH)

#define darray_element_size(array) \
    _darray_field_get(array, DARRAY_ELEMENT_SIZE)

#define darray_length_set(array, value) \
    _darray_field_set(array, DARRAY_LENGTH, value)
