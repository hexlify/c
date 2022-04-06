#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

// void parseprocs(char **procs, FILE *f)
// {
//     char *line = NULL;
//     size_t len = 0;
//     int i = 0;
//     while (getline(&line, &len, f) != -1)
//     {
//         line[strcspn(line, "\n")] = 0;
//         procs[i] = line;
//         i++;
//     }

//     if (line)
//         free(line);
// }

// int main(int argc, char **argv)
// {
//     if (argc == 1)
//     {
//         printf("Provide configuration filename\n");
//         exit(1);
//     }

//     FILE *f = fopen(argv[1], "r");
//     if (f == NULL)
//     {
//         printf("Configuration file not found\n");
//         exit(1);
//     }

//     char ch;
//     int procc = 0;
//     while ((ch = fgetc(f)) != EOF)
//     {
//         if (ch == '\n')
//             procc++;
//     }
//     procc++;
//     char *procs[procc];

//     rewind(f);
//     parseprocs(procs, f);
//     fclose(f);

//     // ----

//     if (getppid() != 1)
//     {
//         signal(SIGTSTP, SIG_IGN);
//         signal(SIGTTIN, SIG_IGN);
//         signal(SIGTTOU, SIG_IGN);

//         if (fork() != 0)
//             exit(0);

//         setsid();
//     }

//     struct rlimit flim;
//     getrlimit(RLIMIT_NOFILE, &flim);

//     for (int fd = 0; fd < flim.rlim_max; fd++)
//         close(fd);

//     chdir("/");
// }

// int main()
// {
//     int pid = fork();
//     if (pid > 0)
//     {
//         int status;
//         printf("I'm parent=%i and I waiting for my child=%i\n", getpid(), pid);
//         wait(&status);
//         printf("He done with status=%i\n", WEXITSTATUS(status));
//     }
//     else if (pid == 0)
//     {
//         int err = execlp("slep", "sleep", "2h", NULL);
//         if (err == -1)
//         {
//             printf("Exec return -1, errno=%i\n", errno);
//             return 1;
//         }
//     }
//     else
//     {
//         printf("Error happened!\n");
//     }
// }

#define PROGS_COUNT 4
#define LOG_FILE "/tmp/myinit.log"
#define DELAY 60

#define PERM_644 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define LOGMODE O_WRONLY | O_APPEND | O_CREAT

void fork_prog(char **prog, int *spid, int delay)
{
    int pid = fork();
    if (pid == 0)
    {
        int in = open(prog[2], O_RDONLY);
        int out = open(prog[3], LOGMODE, PERM_644);
        if (in == -1 || out == -1)
        {
            printf("%s %s (%i):\tFailed to open input/output files, errno=%i\n", prog[0], prog[1], getpid(), errno);
            exit(1);
        }

        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);

        close(in);
        close(out);

        if (delay)
        {
            sleep(DELAY);
        }

        int err = execl(prog[0], prog[0], prog[1], NULL);
        if (err == -1)
        {
            printf("%s %s (%i):\tFailed to exec, errno=%i\n", prog[0], prog[1], getpid(), errno);
            exit(1);
        }
    }
    else if (pid > 0)
    {
        *spid = pid;
        if (delay)
        {
            printf("%s %s (%i):\tProcess started (delayed=%i seconds)\n", prog[0], prog[1], pid, DELAY);
        }
        else
        {
            printf("%s %s (%i):\tProcess started\n", prog[0], prog[1], pid);
        }
    }
    else
    {
        printf("%s %s:\tFailed to fork, errno=%i\n", prog[0], prog[1], errno);
        exit(1);
    }
}

int main()
{
    int out = open(LOG_FILE, LOGMODE, PERM_644);
    dup2(out, STDOUT_FILENO);
    close(out);

    char *progs[PROGS_COUNT][4] = {
        {"/tmp/mysleep.sh", "4s", "/tmp/in/1", "/tmp/out/1"},
        {"/tmp/mysleep.sh", "10s", "/tmp/in/2", "/tmp/out/2"},
        {"/tmp/mysleep.sh", "3s", "/tmp/in/3", "/tmp/out/3"},
        {"trash", "1s", "/tmp/in/4", "/tmp/out/4"},
    };
    int pids[PROGS_COUNT];

    for (size_t i = 0; i < PROGS_COUNT; i++)
    {
        fork_prog(progs[i], &pids[i], 0);
    }

    int wstatus;
    for (;;)
    {
        int pid = wait(&wstatus);
        int pi = 0;
        for (; pi < PROGS_COUNT; pi++)
        {
            if (pids[pi] == pid)
            {
                break;
            }
        }
        int code = WEXITSTATUS(wstatus);
        printf("%s %s (%i):\tProcess finished with code %i\n", progs[pi][0], progs[pi][1], pid, code);
        fork_prog(progs[pi], &pids[pi], code != 0);
    }

    // wait над пидами
}