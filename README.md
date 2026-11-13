# msh: a minimal Unix shell in C

**Name:** Emmanuel Kalii
**Student ID:** SCT212-0329/2023
**Course:** ICS 2305 Systems Programming

## Build and run

```bash
git clone https://github.com/KALIIKARDZ/msh-SCT_212_0329_2023.git
cd msh-SCT_212_0329_2023
make
./msh
```

`make clean` removes the binary. It builds with `-Wall -Wextra` and no warnings.

## Task checklist

| Task | What it covers | Status |
|------|----------------|--------|
| 1 | Prompt, `getline()` for any line length, clean exit on Ctrl-D, no leaks | complete |
| 2 | Split the line into a NULL-terminated `argv` with `strtok()`, skip blank lines | complete |
| 3 | Run external commands with `fork`, `execvp`, `waitpid`; unknown commands don't kill the shell | complete |
| 4 | Built-ins `cd [dir]` (no argument goes to `$HOME`), `help`, `exit` | complete |
| 5 | Prompt shows the last status as `msh[N]>`, 128 + signal number if a command was killed | complete |
| 6 | Background jobs with `&`: prints the PID, returns at once, reaps with `WNOHANG` | complete |
| 7 | Output redirection with `>` using `open`, `dup2`, `close` in the child, mode 0644 | complete |
| 8 | Ctrl-C stops the foreground command but not the shell | complete |

## Design explanation

msh is one loop. Each time round it reaps any finished background children,
prints the prompt with the last status, and reads a line with `getline()`.
`strtok()` splits the line into `argv`. A trailing `&` is removed and
remembered as a background flag, and a `>` splits off the output filename,
which is also removed from `argv` so the program never sees it.

Before forking I check `argv[0]` against the built-ins. `cd`, `help` and
`exit` have to run inside the shell process itself. `cd` calls `chdir()`,
which only changes the working directory of the process that calls it. If a
forked child did it, the child's directory would change and the shell's would
not, so the change would be lost when the child exited. `exit` has to break
the shell's own loop, so a child can't do that either.

Anything else goes through `fork()`. The child resets `SIGINT`, does the
redirection if there is one (`open`, `dup2` onto stdout, `close`), and calls
`execvp()`, which replaces the child with the new program. If `execvp()`
returns, it failed, so the child prints the error and calls `_exit(127)`. The
redirection is done in the child because the shell's own stdout must keep
pointing at the terminal. The parent then either prints `[bg]` and the PID and
goes back to the prompt, or calls `waitpid()` on that PID and turns the result
into a status: the exit code for a normal exit, or 128 plus the signal number
if the child was killed.

A child that has exited stays in the process table as a zombie until its
parent collects its status. Foreground children are collected straight away by
the `waitpid()` call. Background children are collected at the top of every
loop by `waitpid(-1, NULL, WNOHANG)` inside a `while` loop. `WNOHANG` stops it
blocking, and the loop keeps going until nothing else has finished, so several
finished jobs are cleaned up in one go. A zombie can exist briefly, between a
job finishing and the next prompt, but they never build up.

## Known limitations

- Only `>` is supported. There are no pipes, `<`, `>>` or `2>`, and only one
  redirection per command.
- `>` and `&` must be separate tokens. `echo hi>f` or `sleep 1&` are not
  recognised, because the line is split on whitespace only.
- Built-ins ignore redirection, so `help > h.txt` prints to the terminal.
- No quoting, so a filename with spaces can't be passed. No `$` expansion,
  globbing, history or tab completion.
- No job control (`jobs`, `fg`, `bg`, Ctrl-Z). Background jobs are not
  announced when they finish, and the prompt status after starting one is 0
  whatever happens to the job later.
- Background jobs ignore Ctrl-C so that interrupting a foreground command
  doesn't also kill them. The side effect is that a background job can't be
  stopped with Ctrl-C. Doing this properly needs process groups (`setpgid`).

## Sources and AI declaration

- Man pages: `fork(2)`, `execvp(3)`, `waitpid(2)`, `getline(3)`, `strtok(3)`,
  `chdir(2)`, `signal(2)`, `open(2)`, `dup2(2)`.
- *Operating Systems: Three Easy Pieces*, chapter 5 (Process API):
  https://pages.cs.wisc.edu/~remzi/OSTEP/
- I used AI assistants (Claude and others) to plan the order of the tasks,
  draft code, review it against the assignment and help write these documents.
  I compiled and ran everything myself on WSL Ubuntu, and the results in the
  test plan come from those runs.
