#include "builtins.h"

#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include "path.h"
#include "util.h"

typedef int (*builtin_fn)(const struct command *cmd);

/* exit: takes no arguments. */
static int builtin_exit(const struct command *cmd)
{
    if (cmd->argc != 1) {
        wish_error();
        return 0;
    }
    return 1;
}

/* cd DIR: exactly one argument; a failed chdir() is an error too. */
static int builtin_cd(const struct command *cmd)
{
    if (cmd->argc != 2 || chdir(cmd->argv[1]) != 0)
        wish_error();
    return 0;
}

/* path [DIR]...: replaces the search path; without arguments it becomes empty. */
static int builtin_path(const struct command *cmd)
{
    path_set(cmd->argv + 1, cmd->argc - 1);
    return 0;
}

static const struct {
    const char *name;
    builtin_fn run;
} builtins[] = {
    { "exit", builtin_exit },
    { "cd",   builtin_cd   },
    { "path", builtin_path },
};

static builtin_fn find_builtin(const char *name)
{
    for (size_t i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++)
        if (strcmp(builtins[i].name, name) == 0)
            return builtins[i].run;
    return NULL;
}

int is_builtin(const char *name)
{
    return find_builtin(name) != NULL;
}

int run_builtin(const struct command *cmd)
{
    builtin_fn run = find_builtin(cmd->argv[0]);
    return run != NULL ? run(cmd) : 0;
}
