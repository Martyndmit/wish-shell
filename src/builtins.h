#ifndef WISH_BUILTINS_H
#define WISH_BUILTINS_H

#include "parser.h"

/*
 * Built-in commands are implemented by the shell itself and run inside the
 * shell process, not in a child: e.g. `cd` has to change the working
 * directory of the shell, which a child process cannot do.
 */

/* Returns 1 if `name` is a built-in command (exit, cd, path), 0 otherwise. */
int is_builtin(const char *name);

/*
 * Runs a built-in command.
 * Returns 1 if the shell has to exit (`exit`), 0 otherwise.
 */
int run_builtin(const struct command *cmd);

#endif
