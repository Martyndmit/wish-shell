#ifndef WISH_PARSER_H
#define WISH_PARSER_H

/*
 * One simple command, e.g. "ls -la /tmp".
 *
 * The strings point into the input line: strsep() splits the line in place,
 * so nothing is copied and the strings stay valid until the next line is read.
 */
struct command {
    char **argv;    /* NULL-terminated argument vector; argv[0] is the program */
    int argc;       /* number of words in argv */
};

/*
 * Splits `line` (modifying it) into whitespace-separated words.
 * Returns 1 if a command was parsed into `cmd`, 0 if the line is empty.
 */
int parse_command(char *line, struct command *cmd);

/* Frees the memory allocated by parse_command(). */
void free_command(struct command *cmd);

#endif
