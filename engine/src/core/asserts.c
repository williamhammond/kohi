#include "asserts.h"
#include "logger.h"

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion failed: %s\n\t%s\n\t%s:%d", expression, message, file, line);
}