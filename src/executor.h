#ifndef WISH_EXECUTOR_H
#define WISH_EXECUTOR_H

#include "parser.h"

/*
 * Executes one command: a built-in runs inside the shell, any other program
 * in a child process, and the shell waits for it to finish.
 * Returns 1 if the shell has to exit, 0 otherwise.
 */
int execute_command(const struct command *cmd);

#endif
