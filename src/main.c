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
#include <sys/stat.h>

#include "executor.h"
#include "parser.h"
#include "path.h"
#include "util.h"

/* Opens the batch file; returns NULL if it cannot be used as one. */
static FILE *open_batch_file(const char *name)
{
    FILE *file = fopen(name, "r");
    struct stat st;

    if (file == NULL)
        return NULL;

    /* fopen() succeeds on a directory, but reading from it fails. */
    if (fstat(fileno(file), &st) < 0 || S_ISDIR(st.st_mode)) {
        fclose(file);
        return NULL;
    }

    /* Programs started by the shell must not inherit the batch file. */
    if (fcntl(fileno(file), F_SETFD, FD_CLOEXEC) < 0)
        wish_error();

    return file;
}

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    int interactive = 1;

    if (argc > 2) {             /* more than one batch file */
        wish_error();
        exit(1);
    }
    if (argc == 2) {
        input = open_batch_file(argv[1]);
        if (input == NULL) {    /* bad batch file */
            wish_error();
            exit(1);
        }
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

        struct cmdline cl;
        if (parse_line(line, &cl) < 0) {
            wish_error();       /* syntax error: the whole line is skipped */
            continue;
        }
        should_exit = execute_cmdline(&cl);
        free_cmdline(&cl);
    }

    free(line);
    path_free();
    if (input != stdin)
        fclose(input);
    exit(0);
}
