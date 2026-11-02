#ifndef WISH_EXECUTOR_H
#define WISH_EXECUTOR_H

#include "parser.h"

/*
 * Executes all commands of one input line.
 *
 * Built-ins run inside the shell, in the order they appear. Other programs
 * are started in child processes without waiting, so they run in parallel;
 * only after all of them have been started does the shell wait for them.
 *
 * Returns 1 if the shell has to exit (`exit` was used), 0 otherwise.
 */
int execute_cmdline(const struct cmdline *cl);

#endif
