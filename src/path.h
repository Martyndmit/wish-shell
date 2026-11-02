#ifndef WISH_PATH_H
#define WISH_PATH_H

/*
 * The shell's search path: the list of directories where programs are looked
 * up, in order. The shell does not implement `ls` & co. itself — it only finds
 * the executable in one of these directories and runs it.
 */

/* Sets the initial search path, which contains one directory: /bin. */
void path_init(void);

/* Replaces the search path with copies of dirs[0..n-1]; n may be 0. */
void path_set(char *const dirs[], int n);

/*
 * Looks for an executable file `name` in the search path directories.
 * Returns the full path ("dir/name", allocated — the caller frees it)
 * or NULL if no directory contains such an executable.
 */
char *path_resolve(const char *name);

/* Frees the search path (it becomes empty). */
void path_free(void);

#endif
