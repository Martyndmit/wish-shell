#include "util.h"

#include <unistd.h>

void wish_error(void)
{
    static const char error_message[] = "An error has occurred\n";

    /*
     * write(2) is unbuffered: the message appears immediately and can never
     * be duplicated by a fork() the way pending stdio buffers can.
     */
    if (write(STDERR_FILENO, error_message, sizeof(error_message) - 1) < 0) {
        /* stderr itself is broken: there is nowhere left to report it */
    }
}
