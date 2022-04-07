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

void readconf(char *filename)
{
    dprintf(logfd, "Reading %s config file\n", filename);

    FILE *conf = fopen(filename, "r");
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

void parse(wordexp_t w, char **filename, char **stdin, char **stdout)
{
    *filename = w.we_wordv[0];

    // copy stdin to new string because this value will be NULLed
    char *parsed_stdin = w.we_wordv[w.we_wordc - 2];
    *stdin = malloc(sizeof(char) * (strlen(parsed_stdin) + 1));
    *stdin = strcpy(*stdin, parsed_stdin);

    *stdout = w.we_wordv[w.we_wordc - 1];

    // execv takes args that ends with NULL
    w.we_wordv[w.we_wordc - 2] = NULL;
}

void _fork(wordexp_t w, int *spid, int delay)
{
    char *filename, *stdin, *stdout;
    parse(w, &filename, &stdin, &stdout);
    char **args = w.we_wordv;

    int pid = fork();
    if (pid == 0)
    {
        int in = open(stdin, O_RDONLY);
        free(stdin);
        int out = open(stdout, LOGMODE, PERM_644);
        if (in == -1 || out == -1)
        {
            dprintf(logfd, "%s %s (%i):\tFailed to open input/output files, errno=%i\n", args[0], args[1], getpid(), errno);
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

        int err = execv(filename, args);
        if (err == -1)
        {
            printf("%s %s (%i):\tFailed to exec, errno=%i\n", args[0], args[1], getpid(), errno);
            exit(1);
        }
    }
    else if (pid > 0)
    {
        free(stdin);
        *spid = pid;
        if (delay)
            dprintf(logfd, "%s %s (%i):\tProcess started (delayed for %i seconds)\n", args[0], args[1], pid, DELAY);
        else
            dprintf(logfd, "%s %s (%i):\tProcess started\n", args[0], args[1], pid);
    }
    else
    {
        dprintf(logfd, "%s %s:\tFailed to fork, errno=%i\n", args[0], args[1], errno);
        free(stdin);
        exit(1);
    }
}

void startchilds()
{
    for (size_t i = 0; i < pcount; i++)
        _fork(progs[i], &pids[i], 0);
}

void stopchilds()
{
    int wstatus;
    for (size_t i = 0; i < pcount; i++)
    {
        kill(pids[i], SIGTERM);
        wait(&wstatus);
        dprintf(logfd, "%s %s (%i):\tProcess interrupted\n", progs[i].we_wordv[0], progs[i].we_wordv[1], pids[i]);
    }
}

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

void sighup_handler(int signum)
{
    stopchilds();
    // TODO: reread config
    // TODO: reset pids arr
    dprintf(logfd, "Config re-read\n");
    startchilds();
}

int main(int arc, char **argv)
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

    // TODO: check are all paths valid
    chdir("/");

    // setup logging file
    logfd = open(LOG_FILE, LOGMODE, PERM_644);

    readconf(argv[1]);

    // setup signals handlers
    signal(SIGTERM, &gracefully_term);
    signal(SIGHUP, &sighup_handler);

    startchilds();

    int wstatus;
    for (;;)
    {
        int pid = wait(&wstatus);
        int pi = 0;
        for (; pi < pcount; pi++)
        {
            if (pids[pi] == pid)
                break;
        }
        int code = WEXITSTATUS(wstatus);

        dprintf(logfd, "%s %s (%i):\tProcess finished with code %i\n", progs[pi].we_wordv[0], progs[pi].we_wordv[1], pid, code);
        _fork(progs[pi], &pids[pi], code != 0);
    }
}