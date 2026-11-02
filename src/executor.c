#include "executor.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "path.h"
#include "util.h"

/*
 * Starts the program in a child process.
 * Returns the child's pid, or -1 if the program could not be started
 * (the error message has already been printed in that case).
 */
static pid_t launch(const struct command *cmd)
{
    char *program = path_resolve(cmd->argv[0]);
    if (program == NULL) {          /* not found in any path directory */
        wish_error();
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        wish_error();
        free(program);
        return -1;
    }

    if (pid == 0) {
        /* Child: replace this process image with the program. */
        execv(program, cmd->argv);

        /* execv() returns only if it failed. */
        wish_error();
        /*
         * _exit(), not exit(): exit() would run the stdio cleanup of the
         * copy of the shell's memory, which for the batch file moves the
         * file offset shared with the parent back — the parent would then
         * read some lines twice.
         */
        _exit(1);
    }

    /* Parent */
    free(program);
    return pid;
}

int execute_command(const struct command *cmd)
{
    if (is_builtin(cmd->argv[0]))
        return run_builtin(cmd);

    pid_t pid = launch(cmd);
    if (pid > 0 && waitpid(pid, NULL, 0) < 0)
        wish_error();
    return 0;
}
