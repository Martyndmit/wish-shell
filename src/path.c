#include "path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.h"

static char **dirs;     /* the shell's own copies of the directory names */
static int ndirs;

void path_free(void)
{
    for (int i = 0; i < ndirs; i++)
        free(dirs[i]);
    free(dirs);
    dirs = NULL;
    ndirs = 0;
}

void path_set(char *const new_dirs[], int n)
{
    /*
     * The arguments point into the current input line, which the next
     * getline() overwrites, so the path keeps its own copies.
     */
    path_free();
    if (n == 0)
        return;         /* empty path: only built-in commands can run */

    dirs = xmalloc((size_t)n * sizeof(*dirs));
    for (int i = 0; i < n; i++)
        dirs[i] = xstrdup(new_dirs[i]);
    ndirs = n;
}

void path_init(void)
{
    char *initial[] = { "/bin" };
    path_set(initial, 1);
}

/*
 * access(X_OK) as suggested by the specification; stat() additionally
 * rejects directories, for which access(X_OK) also succeeds.
 */
static int is_executable(const char *file)
{
    struct stat st;
    return access(file, X_OK) == 0 && stat(file, &st) == 0 && S_ISREG(st.st_mode);
}

char *path_resolve(const char *name)
{
    for (int i = 0; i < ndirs; i++) {
        size_t len = strlen(dirs[i]) + 1 + strlen(name) + 1;
        char *full = xmalloc(len);
        snprintf(full, len, "%s/%s", dirs[i], name);
        if (is_executable(full))
            return full;
        free(full);
    }
    return NULL;
}
