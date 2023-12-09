#include "logger.h"

#include "core/kmemory.h"
#include "core/kstring.h"

#include "platform/filesystem.h"
#include "platform/platform.h"
#include <stdarg.h>

typedef struct logger_system_state {
    file_handle log_file_handle;
} logger_system_state;

static logger_system_state* state_ptr;

void append_to_file(const char* message) {
    if (state_ptr && state_ptr->log_file_handle.is_valid) {
        u64 length = string_length(message);
        u64 written = 0;
        if (!filesystem_write(&state_ptr->log_file_handle, length, message, &written)) {
            platform_console_write_error("ERROR writing to console.log", LOG_LEVEL_ERROR);
        }
    }
}

b8 initialize_logging(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(logger_system_state);
    if (state == NULL) {
        return true;
    }
    state_ptr = state;

    if (!filesystem_open("C:\\Users\\willi\\Code\\kohi\\bin\\console.log", FILE_MODE_WRITE, false, &state_ptr->log_file_handle)) {
        platform_console_write_error("ERROR: Unable to open console.log for writing", LOG_LEVEL_ERROR);
        return false;
    }

    return true;
}

void shutdown_logging(void* state) {
    state_ptr = NULL;
}

void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {
        "[FATAL]",
        "[ERROR]",
        "[WARN]",
        "[INFO]",
        "[DEBUG]",
        "[TRACE]"};

    char out_message[32000];
    kzero_memory(out_message, sizeof(out_message));

    // Format original message.
    // Note: MS's headers override the GCC/Clang va_list type with a "teypdef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    string_format_v(out_message, message, arg_ptr);
    va_end(arg_ptr);

    string_format(out_message, "%s/%s\n", level_strings[level], out_message);

    b8 is_error = level <= LOG_LEVEL_ERROR;
    if (is_error) {
        platform_console_write_error(out_message, level);
    } else {
        platform_console_write(out_message, level);
    }

    append_to_file(out_message);
}