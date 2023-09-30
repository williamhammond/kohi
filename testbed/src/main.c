#include <core/logger.h>
#include <core/asserts.h>

#include <platform/platform.h>

int main(void) {
    KINFO("PI: %f", 3.14159265359f);
    KERROR("PI: %f", 3.14159265359f);
    KFATAL("PI: %f", 3.14159265359f);
    KWARN("PI: %f", 3.14159265359f);
    KDEBUG("PI: %f", 3.14159265359f);
    KTRACE("PI: %f", 3.14159265359f);

    platform_state state;
    if (platform_startup(&state, "Kohi Engine Testbed", 100, 100, 1280, 720)) {
        while (TRUE) {
            platform_pump_messages(&state);
            platform_sleep(1);
        }
    }

    platform_shutdown(&state);
    
    return 0;
}