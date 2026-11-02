#ifndef WISH_UTIL_H
#define WISH_UTIL_H

#include <stddef.h>

/* Prints "the one and only error message" required by the specification. */
void wish_error(void);

/*
 * Memory helpers: like malloc/realloc/strdup, but if memory runs out they
 * print the error message and terminate the shell, so callers never get NULL.
 */
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *s);

#endif
