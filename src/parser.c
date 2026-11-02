#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "util.h"

/* Word separators; '\r' also makes batch files with Windows line endings work. */
#define WHITESPACE " \t\r\n\v\f"

/*
 * Splits `s` into words with strsep(). strsep() returns an empty token for
 * every pair of adjacent separators, so empty tokens are skipped.
 * Returns a NULL-terminated array (allocated); the words point into `s`.
 */
static char **split_words(char *s, int *count)
{
    int cap = 4;
    int n = 0;
    char **words = xmalloc((size_t)cap * sizeof(*words));
    char *token;

    while ((token = strsep(&s, WHITESPACE)) != NULL) {
        if (*token == '\0')
            continue;
        if (n + 1 == cap) {             /* keep a slot for the final NULL */
            cap *= 2;
            words = xrealloc(words, (size_t)cap * sizeof(*words));
        }
        words[n++] = token;
    }
    words[n] = NULL;
    *count = n;
    return words;
}

static void free_command(struct command *cmd)
{
    free(cmd->argv);
    cmd->argv = NULL;
    cmd->argc = 0;
}

/*
 * Parses one command: the text between '&' separators.
 * Returns 1 if a command was parsed into `cmd`, 0 if the text is blank and
 * -1 on a syntax error (nothing needs to be freed in the last two cases).
 */
static int parse_command(char *text, struct command *cmd)
{
    /*
     * Split at the first '>'. The operator needs no whitespace around it,
     * so "ls>out" works too. `target` is NULL if there is no '>'.
     */
    char *target = text;
    char *command_part = strsep(&target, ">");

    if (target != NULL && strchr(target, '>') != NULL)
        return -1;                      /* more than one '>' */

    cmd->argv = split_words(command_part, &cmd->argc);
    cmd->outfile = NULL;

    if (target == NULL) {               /* no redirection */
        if (cmd->argc == 0) {           /* blank */
            free_command(cmd);
            return 0;
        }
        return 1;
    }

    /* "command [args] > file": a command and exactly one file are required. */
    int nfiles;
    char **files = split_words(target, &nfiles);
    if (cmd->argc == 0 || nfiles != 1) {
        free(files);
        free_command(cmd);
        return -1;
    }
    cmd->outfile = files[0];
    free(files);
    return 1;
}

int parse_line(char *line, struct cmdline *cl)
{
    int cap = 2;
    cl->cmds = xmalloc((size_t)cap * sizeof(*cl->cmds));
    cl->count = 0;

    /* Every piece between '&' separators is a separate command. */
    char *rest = line;
    char *piece;
    while ((piece = strsep(&rest, "&")) != NULL) {
        struct command cmd;
        int parsed = parse_command(piece, &cmd);
        if (parsed < 0) {
            free_cmdline(cl);
            return -1;
        }
        if (parsed == 0)
            continue;       /* blank piece: "&", "cmd &" and "& cmd" are fine */

        if (cl->count == cap) {
            cap *= 2;
            cl->cmds = xrealloc(cl->cmds, (size_t)cap * sizeof(*cl->cmds));
        }
        cl->cmds[cl->count++] = cmd;
    }
    return 0;
}

void free_cmdline(struct cmdline *cl)
{
    for (int i = 0; i < cl->count; i++)
        free_command(&cl->cmds[i]);
    free(cl->cmds);
    cl->cmds = NULL;
    cl->count = 0;
}
