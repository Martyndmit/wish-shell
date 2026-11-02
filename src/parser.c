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

int parse_command(char *line, struct command *cmd)
{
    cmd->argv = split_words(line, &cmd->argc);
    if (cmd->argc == 0) {               /* blank line */
        free_command(cmd);
        return 0;
    }
    return 1;
}

void free_command(struct command *cmd)
{
    free(cmd->argv);
    cmd->argv = NULL;
    cmd->argc = 0;
}
