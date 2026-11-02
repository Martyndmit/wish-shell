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

/* A whole input line: commands separated by '&', which run in parallel. */
struct cmdline {
    struct command *cmds;
    int count;      /* 0 for a blank line or a line like "&" */
};

/*
 * Parses `line` (modifying it) into `cl`.
 * Returns 0 on success and -1 if any command has a syntax error; in that
 * case the whole line is rejected and nothing needs to be freed.
 */
int parse_line(char *line, struct cmdline *cl);

/* Frees the memory allocated by parse_line(). */
void free_cmdline(struct cmdline *cl);

#endif
