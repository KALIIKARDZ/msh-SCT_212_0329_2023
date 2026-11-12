#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_ARGS 128
#define DELIMS " \t\r\n"

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

static int run_external(char **argv)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);        /* child obeys Ctrl-C */
        execvp(argv[0], argv);
        fprintf(stderr, "msh: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    int st = 0;
    if (waitpid(pid, &st, 0) < 0) {
        perror("waitpid");
        return 1;
    }

    if (WIFEXITED(st))
        return WEXITSTATUS(st);
    if (WIFSIGNALED(st))
        return 128 + WTERMSIG(st);
    return 1;
}

int main(void)
{
    signal(SIGINT, SIG_IGN);            /* shell survives Ctrl-C (Task 8) */

    char *line = NULL;
    size_t cap = 0;
    int status = 0;
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
        if (argc == 0)
            continue;

        status = run_external(argv);
        printf("[debug] last status = %d\n", status);
    }

    free(line);
    return 0;
}
