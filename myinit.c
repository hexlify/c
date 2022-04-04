#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void parseprocs(char **procs, FILE *f)
{
    char *line = NULL;
    size_t len = 0;
    int i = 0;
    while (getline(&line, &len, f) != -1)
    {
        line[strcspn(line, "\n")] = 0;
        procs[i] = line;
        i++;
    }

    if (line)
        free(line);
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("Provide configuration filename\n");
        exit(1);
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("Configuration file not found\n");
        exit(1);
    }

    char ch;
    int procc = 0;
    while ((ch = fgetc(f)) != EOF)
    {
        if (ch == '\n')
            procc++;
    }
    procc++;
    char *procs[procc];

    rewind(f);
    parseprocs(procs, f);
    fclose(f);

    // ----

    if (getppid() != 1)
    {
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        if (fork() != 0)
            exit(0);

        setsid();
    }

    struct rlimit flim;
    getrlimit(RLIMIT_NOFILE, &flim);

    for (int fd = 0; fd < flim.rlim_max; fd++)
        close(fd);

    chdir("/");
}