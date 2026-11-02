/*
 * wish — the Wisconsin shell (OSTEP "processes-shell" project).
 */
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *line = NULL;  /* getline() allocates and grows this buffer */
    size_t cap = 0;

    for (;;) {
        printf("wish> ");
        fflush(stdout);

        if (getline(&line, &cap, stdin) < 0)
            break;      /* EOF (Ctrl-D): exit gracefully */

        /* TODO: parse and execute the line */
    }

    free(line);
    exit(0);
}
