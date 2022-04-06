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

#define PROGS_COUNT 3
#define TIMEOUT 60

void fork_prog(char **prog, int *spid)
{
    int pid = fork();
    if (pid == 0)
    {
        int err = execl(prog[0], prog[0], prog[1], NULL);
        if (err == -1)
        {
            char *desc = "";
            if (errno == ENOENT)
            {
                desc = "(Executable file not found)";
            }
            printf("Failed to exec %s, errno=%i %s\n", prog[0], errno, desc);
            exit(1);
        }
    }
    else if (pid > 0)
    {
        *spid = pid;
    }
    else
    {
        printf("Failed to fork, errno=%i\n", errno);
    }
}

int main()
{
    char *progs[PROGS_COUNT][4] = {
        {"/tmp/mysleep.sh", "4s", "/tmp/in/1", "/tmp/out/1"},
        {"/tmp/mysleep.sh", "1s", "/tmp/in/2", "/tmp/out/2"},
        {"/tmp/mysleep.sh", "3s", "/tmp/in/3", "/tmp/out/3"},
        // {"trash", "", "/tmp/in/4", "/tmp/out/4"},
    };
    int pids[PROGS_COUNT];

    for (size_t i = 0; i < PROGS_COUNT; i++)
    {
        fork_prog(progs[i], &pids[i]);
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
        printf("Program pid=%i exec=%s args=%s finished with code %i\n", pid, progs[pi][0], progs[pi][1], WEXITSTATUS(wstatus));

        fork_prog(progs[pi], &pids[pi]);
    }
}