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

static void print_help(void)
{
    printf("msh - minimal shell\n");
    printf("Built-ins:\n");
    printf("  cd [dir]   change directory (no arg goes to $HOME)\n");
    printf("  help       show this help\n");
    printf("  exit       leave the shell\n");
    printf("Anything else is run as an external command via fork/exec.\n");
}

/*
 * Run a built-in if argv[0] matches one.
 *   -1  -> not a built-in; caller runs it via fork/exec
 *    0  -> built-in succeeded
 *   >0  -> built-in failed with that status
 * Sets *should_exit=1 when the shell should terminate.
 */
static int run_builtin(char **argv, int argc, int *should_exit)
{
    if (strcmp(argv[0], "exit") == 0) {
        *should_exit = 1;
        return 0;
    }

    if (strcmp(argv[0], "help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[0], "cd") == 0) {
        const char *target;
        if (argc < 2) {
            target = getenv("HOME");
            if (target == NULL) {
                fprintf(stderr, "msh: cd: HOME not set\n");
                return 1;
            }
        } else {
            target = argv[1];
        }
        if (chdir(target) != 0) {
            fprintf(stderr, "msh: cd: %s: %s\n", target, strerror(errno));
            return 1;
        }
        return 0;
    }

    return -1;                          /* not a built-in */
}

static int run_external(char **argv)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
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
    signal(SIGINT, SIG_IGN);

    char *line = NULL;
    size_t cap = 0;
    int status = 0;
    char *argv[MAX_ARGS];
    int should_exit = 0;

    while (!should_exit) {
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

        int rc = run_builtin(argv, argc, &should_exit);
        if (rc >= 0) {
            status = rc;
        } else {
            status = run_external(argv);
        }

        printf("[debug] last status = %d\n", status);
    }

    free(line);
    return 0;
}
