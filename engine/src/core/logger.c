#include "logger.h"

#include "platform/platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

b8 initialize_logging() {
    return TRUE;
}

void shutdown_logging() {
}

void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {
        "[FATAL]",
        "[ERROR]",
        "[WARN]",
        "[INFO]",
        "[DEBUG]",
        "[TRACE]"};

    const i32 msg_length = 32000;
    char out_message[msg_length];
    memset(out_message, 0, sizeof(out_message));

    // Format original message.
    // Note: MS's headers override the GCC/Clang va_list type with a "teypdef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, sizeof(out_message), message, arg_ptr);
    va_end(arg_ptr);

    char formatted_buffer[msg_length];
    sprintf(formatted_buffer, "%s %s\n", level_strings[level], out_message);

    b8 is_error = level <= LOG_LEVEL_ERROR;
    if (is_error) {
        platform_console_write_error(formatted_buffer, level);
    } else {
        platform_console_write(formatted_buffer, level);
    }
}