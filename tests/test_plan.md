# msh test plan

Tested on WSL Ubuntu, building with the Makefile (`gcc -Wall -Wextra`).
Prompts show the status of the previous command, so the `msh[N]>` value after
each test is part of the result.

## Tests given in the assignment

| # | Input | Expected | What happened | Result |
|---|-------|----------|---------------|--------|
| 1 | blank line | new prompt, no error | prompt came back, status unchanged | PASS |
| 2 | `echo hello world` | prints `hello world`, status 0 | printed it, prompt `msh[0]>` | PASS |
| 3 | `ls /nonexistent` | error from ls, non-zero status | `ls: cannot access '/nonexistent': No such file or directory`, prompt `msh[2]>` | PASS |
| 4 | `nosuchcommand` | error message, status 127, shell survives | `msh: nosuchcommand: No such file or directory`, prompt `msh[127]>` | PASS |
| 5 | `cd /tmp` then `pwd` | prints `/tmp` | `pwd` printed `/tmp` | PASS |
| 6 | `sleep 3 &` | prompt returns at once, PID shown | `[bg] <pid>` printed and the prompt came straight back | PASS |
| 7 | `echo hi > f.txt`, then `cat f.txt` | file contains `hi` | `cat` printed `hi`. Running `echo bye > f.txt` afterwards left only `bye`, so the file is truncated | PASS |
| 8 | `sleep 100`, then Ctrl-C | sleep stops, shell survives, status 130 | `^C`, new line, prompt `msh[130]>`, shell still running | PASS |
| 9 | Ctrl-D at the prompt | shell exits cleanly | newline printed, back at the bash prompt | PASS |

## My own tests

| # | Input | Expected | What happened | Result |
|---|-------|----------|---------------|--------|
| 10 | `help` | usage text, status 0 | 9 lines of usage printed, prompt `msh[0]>` | PASS |
| 11 | `cd` with no argument, then `pwd` | goes to `$HOME` | `pwd` printed `/home/kaliikardz` | PASS |
| 12 | `cd /nonexistent` | error, status 1, directory unchanged | `msh: cd: /nonexistent: No such file or directory`, prompt `msh[1]>`, `pwd` unchanged | PASS |
| 13 | `echo hi >` | syntax error, nothing runs | `msh: syntax error: expected filename after '>'`, prompt `msh[1]>` | PASS |
| 14 | `> f3.txt` | syntax error, no file created | `msh: syntax error: no command before '>'`, prompt `msh[1]>` | PASS |
| 15 | `echo hi > f.txt extra` | syntax error, nothing runs | `msh: syntax error: unexpected token after filename`, prompt `msh[1]>` | PASS |

| 16 | `sleep 1 &`, wait 2 s, press Enter, then `ps` | no zombie left behind | no `<defunct>` entry, the job had been reaped | PASS |
| 17 | `sleep 100 &`, then Ctrl-C at the prompt | shell and background job both survive | prompt came back, `ps` still listed the sleep | PASS |

## Other checks

- `make` from a fresh clone builds with no warnings.
- Ctrl-D leaves the shell with exit code 0.
