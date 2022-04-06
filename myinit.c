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
#include <wordexp.h>

#define LOG_FILE "/tmp/myinit.log"
#define DELAY 60

#define PERM_644 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#define LOGMODE O_WRONLY | O_APPEND | O_CREAT

int pcount = 0;
wordexp_t *progs;
int *pids;
int logfd;

int readconf(int arc, char **argv)
{
    FILE *conf = fopen(argv[1], "r");
    pcount = 1;

    while (!feof(conf))
    {
        if (fgetc(conf) == '\n')
            pcount++;
    }
    rewind(conf);

    // main strcutures allocation
    progs = malloc(sizeof(wordexp_t) * pcount);
    pids = malloc(sizeof(int) * pcount);

    char *l = NULL;
    size_t n = 0;
    for (size_t i = 0; i < pcount; i++)
    {
        getline(&l, &n, conf);

        // removes \n from end of string
        int sub = strcspn(l, "\n");
        char *ll = malloc(sizeof(char) * (sub + 1));
        strncpy(ll, l, sub);

        // parsing shell args string
        wordexp_t p;
        wordexp(ll, &p, 0);

        progs[i] = p;
        pids[i] = 0;
        free(ll);
    }
    free(l);
}

// void _fork(char **prog, int *spid, int delay)
// {
//     int pid = fork();
//     if (pid == 0)
//     {
//         int in = open(prog[2], O_RDONLY);
//         int out = open(prog[3], LOGMODE, PERM_644);
//         if (in == -1 || out == -1)
//         {
//             dprintf(logfd, "%s %s (%i):\tFailed to open input/output files, errno=%i\n", prog[0], prog[1], getpid(), errno);
//             exit(1);
//         }
//
//         dup2(in, STDIN_FILENO);
//         dup2(out, STDOUT_FILENO);
//
//         close(in);
//         close(out);
//
//         if (delay)
//         {
//             sleep(DELAY);
//         }
//
//         int err = execl(prog[0], prog[0], prog[1], NULL);
//         if (err == -1)
//         {
//             printf("%s %s (%i):\tFailed to exec, errno=%i\n", prog[0], prog[1], getpid(), errno);
//             exit(1);
//         }
//     }
//     else if (pid > 0)
//     {
//         *spid = pid;
//         if (delay)
//             dprintf(logfd, "%s %s (%i):\tProcess started (delayed for %i seconds)\n", prog[0], prog[1], pid, DELAY);
//         else
//             dprintf(logfd, "%s %s (%i):\tProcess started\n", prog[0], prog[1], pid);
//     }
//     else
//     {
//         dprintf(logfd, "%s %s:\tFailed to fork, errno=%i\n", prog[0], prog[1], errno);
//         exit(1);
//     }
// }
//
// void startchilds()
// {
//     for (size_t i = 0; i < PROGS_COUNT; i++)
//         _fork(progs[i], &pids[i], 0);
// }

// void stopchilds()
// {
//     int wstatus;
//     for (size_t i = 0; i < pcount; i++)
//     {
//         kill(pids[i], SIGTERM);
//         wait(&wstatus);
//         dprintf(logfd, "%s %s (%i):\tProcess interrupted\n", progs[i][0], progs[i][1], pids[i]);
//     }
// }

void gracefully_term(int signum)
{
    dprintf(logfd, "Gracefully terminating\n");
    stopchilds();
    close(logfd);

    // free resources
    for (size_t i = 0; i < pcount; i++)
        wordfree(&progs[i]);
    free(pids);
    free(progs);

    exit(0);
}

// void sighup_handler(int signum)
// {
//     stopchilds();
//     // TODO: reread config
//     // TODO: reset pids arr
//     dprintf(logfd, "Config re-read\n");
//     startchilds();
// }

// int main1()
// {
//     // clear pids arr, because this can cause killing not our child process by sighup
//     for (size_t i = 0; i < PROGS_COUNT; i++)
//         pids[i] = 0;

//     // daemonize process
//     if (getppid() != 1)
//     {
//         signal(SIGTSTP, SIG_IGN);
//         signal(SIGTTIN, SIG_IGN);
//         signal(SIGTTOU, SIG_IGN);

//         if (fork() != 0)
//             exit(0);

//         setsid();
//     }

//     // close all file descriptors except STDIN STDOUT STDERR
//     struct rlimit flim;
//     getrlimit(RLIMIT_NOFILE, &flim);
//     for (int fd = 3; fd < flim.rlim_max; fd++)
//         close(fd);

//     chdir("/");

//     // setup logging file
//     logfd = open(LOG_FILE, LOGMODE, PERM_644);

//     // setup signals handlers
//     signal(SIGTERM, &gracefully_term);
//     signal(SIGHUP, &sighup_handler);

//     startchilds();

//     int wstatus;
//     for (;;)
//     {
//         int pid = wait(&wstatus);
//         int pi = 0;
//         for (; pi < PROGS_COUNT; pi++)
//         {
//             if (pids[pi] == pid)
//                 break;
//         }
//         int code = WEXITSTATUS(wstatus);

//         dprintf(logfd, "%s %s (%i):\tProcess finished with code %i\n", progs[pi][0], progs[pi][1], pid, code);
//         _fork(progs[pi], &pids[pi], code != 0);
//     }
// }