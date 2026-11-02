#ifndef WISH_PARSER_H
#define WISH_PARSER_H

/*
 * One simple command, e.g. "ls -la /tmp > out".
 *
 * The strings point into the input line: strsep() splits the line in place,
 * so nothing is copied and the strings stay valid until the next line is read.
 */
struct command {
    char **argv;    /* NULL-terminated argument vector; argv[0] is the program */
    int argc;       /* number of words in argv (at least 1) */
    char *outfile;  /* file after '>', or NULL if output is not redirected */
};

/*
 * Parses `line` (modifying it) into `cmd`.
 * Returns 1 if a command was parsed, 0 if the line is empty and -1 on a
 * syntax error (nothing needs to be freed in the last two cases).
 */
int parse_command(char *line, struct command *cmd);

/* Frees the memory allocated by parse_command(). */
void free_command(struct command *cmd);

#endif
