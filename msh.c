#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char   *line   = NULL;
    size_t  cap    = 0;
    int     status = 0;

    while (1) {
        printf("msh[%d]> ", status);
        fflush(stdout);

        ssize_t n = getline(&line, &cap, stdin);
        if (n == -1) {          /* Ctrl-D or error */
            putchar('\n');
            break;
        }

        /* Task 1 only: prove getline read the line */
        printf("you typed: %s", line);
    }

    free(line);
    return 0;
}
