#ifndef WISH_EXECUTOR_H
#define WISH_EXECUTOR_H

#include "parser.h"

/* Runs the program in a child process and waits for it to finish. */
void execute_command(const struct command *cmd);

#endif
