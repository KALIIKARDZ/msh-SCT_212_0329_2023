#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
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
    printf("Syntax:\n");
    printf("  cmd > file   redirect stdout to file (truncate)\n");
    printf("  cmd &        run in background\n");
    printf("Anything else is run as an external command via fork/exec.\n");
}

static void reap_background(void)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

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
    return -1;
}

static int redirect_stdout(const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "msh: %s: %s\n", filename, strerror(errno));
        return -1;
    }
    if (dup2(fd, STDOUT_FILENO) < 0) {
        fprintf(stderr, "msh: dup2: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int run_external(char **argv, int background, const char *redirect_to)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);        /* child obeys Ctrl-C (Task 8) */
        if (redirect_to != NULL && redirect_stdout(redirect_to) < 0)
            _exit(1);
        execvp(argv[0], argv);
        fprintf(stderr, "msh: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }

    if (background) {
        printf("[bg] %d\n", (int)pid);
        return 0;
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
    int should_exit = 0;

    while (!should_exit) {
        reap_background();
        printf("msh[%d]> ", status);
        fflush(stdout);

        ssize_t n = getline(&line, &cap, stdin);
        if (n == -1) {
            printf("\n");
            break;
        }

        int argc = parse_line(line, argv);
        if (argc == 0)
            continue;

        int background = 0;
        if (strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[--argc] = NULL;
            if (argc == 0)
                continue;
        }

        const char *redirect_to = NULL;
        int syntax_error = 0;
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") != 0)
                continue;
            if (i == 0) {
                fprintf(stderr, "msh: syntax error: no command before '>'\n");
                syntax_error = 1;
            } else if (i + 1 >= argc) {
                fprintf(stderr, "msh: syntax error: expected filename after '>'\n");
                syntax_error = 1;
            } else if (i + 2 < argc) {
                fprintf(stderr, "msh: syntax error: unexpected token after filename\n");
                syntax_error = 1;
            } else {
                redirect_to = argv[i + 1];
                argv[i] = NULL;
                argc = i;
            }
            break;
        }
        if (syntax_error) {
            status = 1;
            continue;
        }

        int rc = run_builtin(argv, argc, &should_exit);
        if (rc >= 0)
            status = rc;
        else
            status = run_external(argv, background, redirect_to);
    }

    free(line);
    return 0;
}
