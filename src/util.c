#include "util.h"

#include <stdlib.h>
#include <string.h>
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

static void out_of_memory(void)
{
    wish_error();
    exit(1);
}

void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
        out_of_memory();
    return p;
}

void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (p == NULL)
        out_of_memory();
    return p;
}

char *xstrdup(const char *s)
{
    char *p = strdup(s);
    if (p == NULL)
        out_of_memory();
    return p;
}
