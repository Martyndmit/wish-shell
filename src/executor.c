#include "executor.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins.h"
#include "path.h"
#include "util.h"

/* Opens the target of '>': it is created if missing and truncated if it exists. */
static int open_output(const char *file)
{
    return open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

/*
 * Called in the child: sends both stdout and stderr to `file`. Redirecting
 * stderr as well is the "twist" of the specification (ordinary shells
 * redirect only stdout with '>').
 */
static void redirect_child_output(const char *file)
{
    int fd = open_output(file);
    if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0 || dup2(fd, STDERR_FILENO) < 0) {
        wish_error();
        _exit(1);
    }
    if (fd > STDERR_FILENO)
        close(fd);      /* descriptors 1 and 2 now refer to the file */
}

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
        /* Child: set up redirection, then replace this process image. */
        if (cmd->outfile != NULL)
            redirect_child_output(cmd->outfile);
        execv(program, cmd->argv);

        /* execv() returns only if it failed. */
        wish_error();
        /*
         * _exit(), not exit(): exit() would also run the stdio cleanup in
         * this copy of the shell. For the batch file that moves the file
         * offset, which is shared with the parent, back to the current
         * line, and the parent would then execute some lines twice.
         */
        _exit(1);
    }

    /* Parent */
    free(program);
    return pid;
}

/* Puts the saved descriptor `saved` back as `target` and closes the copy. */
static void restore_fd(int saved, int target)
{
    if (saved < 0)
        return;
    if (dup2(saved, target) < 0)
        wish_error();
    close(saved);
}

/*
 * Runs a built-in command inside the shell. Returns 1 if the shell has to exit.
 *
 * Redirection of built-ins is left open by the specification; wish treats it
 * like for any other command. Since a built-in runs in the shell process
 * itself, the shell temporarily points its own stdout/stderr to the file:
 * dup() saves the original descriptors, dup2() redirects them, and
 * afterwards they are restored.
 */
static int execute_builtin(const struct command *cmd)
{
    if (cmd->outfile == NULL)
        return run_builtin(cmd);

    int fd = open_output(cmd->outfile);
    if (fd < 0) {
        wish_error();
        return 0;
    }

    fflush(stdout);
    int saved_out = dup(STDOUT_FILENO);
    int saved_err = dup(STDERR_FILENO);
    int ok = saved_out >= 0 && saved_err >= 0 &&
             dup2(fd, STDOUT_FILENO) >= 0 && dup2(fd, STDERR_FILENO) >= 0;
    if (fd > STDERR_FILENO)
        close(fd);

    int should_exit = ok ? run_builtin(cmd) : 0;

    fflush(stdout);
    restore_fd(saved_out, STDOUT_FILENO);
    restore_fd(saved_err, STDERR_FILENO);
    if (!ok)
        wish_error();
    return should_exit;
}

int execute_cmdline(const struct cmdline *cl)
{
    if (cl->count == 0)
        return 0;

    pid_t *pids = xmalloc((size_t)cl->count * sizeof(*pids));
    int started = 0;
    int should_exit = 0;

    /* 1. Start all commands without waiting for any of them... */
    for (int i = 0; i < cl->count; i++) {
        const struct command *cmd = &cl->cmds[i];

        if (is_builtin(cmd->argv[0])) {
            if (execute_builtin(cmd))
                should_exit = 1;    /* exit only after the whole line is done */
        } else {
            pid_t pid = launch(cmd);
            if (pid > 0)
                pids[started++] = pid;
        }
    }

    /* 2. ...and only then wait until every started process has finished. */
    for (int i = 0; i < started; i++)
        if (waitpid(pids[i], NULL, 0) < 0)
            wish_error();

    free(pids);
    return should_exit;
}
