#include <core/logger.h>
#include <core/asserts.h>

int main(void) {
    KINFO("PI: %f", 3.14159265359f);
    KERROR("PI: %f", 3.14159265359f);
    KFATAL("PI: %f", 3.14159265359f);
    KWARN("PI: %f", 3.14159265359f);
    KDEBUG("PI: %f", 3.14159265359f);
    KTRACE("PI: %f", 3.14159265359f);
    KASSERT(1 == 0);

    return 0;
}