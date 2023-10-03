#pragma once

#include "defines.h"

KAPI u64 string_length(const char* str);

KAPI char* string_duplicate(const char* str);

// Case-sensitive string comparision.
KAPI b8 string_equal(const char* strA, const char* strB);
