#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 128
#define DELIMS " \t\r\n"

/* Split line into words. The words point INSIDE line (strtok
 * overwrites the separators with '\0'), so nothing extra to free.
 * argv always ends with NULL. Returns the number of words. */
static int parse_line(char *line, char **argv)
{
    int argc = 0;
    char *tok = strtok(line, DELIMS);

    while (tok != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, DELIMS);
    }
    argv[argc] = NULL;
    return argc;
}

int main(void)
{
    char *line = NULL;
    size_t cap = 0;
    char *argv[MAX_ARGS];

    while (1) {
        printf("msh[0]> ");
        fflush(stdout);

        ssize_t n = getline(&line, &cap, stdin);
        if (n == -1) {
            printf("\n");
            break;
        }

        int argc = parse_line(line, argv);
        if (argc == 0)          /* blank line: just show a new prompt */
            continue;

        /* temporary debug print, removed in Task 3 */
        for (int i = 0; argv[i] != NULL; i++)
            printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }

    free(line);
    return 0;
}
