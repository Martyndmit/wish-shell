/*
 * wish — the Wisconsin shell (OSTEP "processes-shell" project).
 *
 * Usage:
 *   ./wish             interactive mode: prints the "wish> " prompt
 *   ./wish batch.txt   batch mode: reads commands from the file, no prompt
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "executor.h"
#include "parser.h"
#include "path.h"
#include "util.h"

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    int interactive = 1;

    if (argc > 2) {             /* more than one batch file */
        wish_error();
        exit(1);
    }
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (input == NULL) {    /* bad batch file */
            wish_error();
            exit(1);
        }
        /* Programs started by the shell must not inherit the batch file. */
        if (fcntl(fileno(input), F_SETFD, FD_CLOEXEC) < 0)
            wish_error();
        interactive = 0;
    }

    path_init();

    char *line = NULL;          /* getline() allocates and grows this buffer */
    size_t cap = 0;
    int should_exit = 0;

    while (!should_exit) {
        if (interactive) {
            printf("wish> ");
            fflush(stdout);     /* empty the buffer before any fork() */
        }

        if (getline(&line, &cap, input) < 0)
            break;              /* EOF: exit gracefully */

        struct command cmd;
        if (parse_command(line, &cmd)) {
            should_exit = execute_command(&cmd);
            free_command(&cmd);
        }
    }

    free(line);
    path_free();
    if (input != stdin)
        fclose(input);
    exit(0);
}
