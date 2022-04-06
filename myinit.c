#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define PROGS_COUNT 3
#define LOG_FILE "/tmp/myinit.log"
#define DELAY 60
#define MAX_LOG_STRING 150

#define PERM_644 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define LOGMODE O_WRONLY | O_APPEND | O_CREAT

int logfd;

void fork_prog(char **prog, int *spid, int delay)
{
    int pid = fork();
    if (pid == 0)
    {
        int in = open(prog[2], O_RDONLY);
        int out = open(prog[3], LOGMODE, PERM_644);
        if (in == -1 || out == -1)
        {
            dprintf(logfd, "%s %s (%i):\tFailed to open input/output files, errno=%i\n", prog[0], prog[1], getpid(), errno);
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
            dprintf(logfd, "%s %s (%i):\tProcess started (delayed for %i seconds)\n", prog[0], prog[1], pid, DELAY);
        }
        else
        {
            dprintf(logfd, "%s %s (%i):\tProcess started\n", prog[0], prog[1], pid);
        }
    }
    else
    {
        dprintf(logfd, "%s %s:\tFailed to fork, errno=%i\n", prog[0], prog[1], errno);
        exit(1);
    }
}

void gracefully_term(int signum)
{
    dprintf(logfd, "Got SIGTERM. Gracefully terminating\n");
    close(logfd);
    // TODO wait над пидами
    exit(0);
}

int main()
{
    // daemonize process
    if (getppid() != 1)
    {
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        if (fork() != 0)
            exit(0);

        setsid();
    }

    // close all file descriptors except STDIN STDOUT STDERR
    struct rlimit flim;
    getrlimit(RLIMIT_NOFILE, &flim);
    for (int fd = 3; fd < flim.rlim_max; fd++)
        close(fd);

    chdir("/");

    // setup logging file
    logfd = open(LOG_FILE, LOGMODE, PERM_644);

    signal(SIGTERM, &gracefully_term);

    char *progs[PROGS_COUNT][4] = {
        {"/tmp/mysleep.sh", "10s", "/tmp/in/1", "/tmp/out/1"},
        {"/tmp/mysleep.sh", "90s", "/tmp/in/2", "/tmp/out/2"},
        {"/tmp/mysleep.sh", "20s", "/tmp/in/3", "/tmp/out/3"},
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

        dprintf(logfd, "%s %s (%i):\tProcess finished with code %i\n", progs[pi][0], progs[pi][1], pid, code);
        fork_prog(progs[pi], &pids[pi], code != 0);
    }
}