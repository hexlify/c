#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    if (getppid() != 1)
    {
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

        if (fork() != 0)
        {
            exit(0);
        }

        setsid();
    }

    struct rlimit flim;
    getrlimit(RLIMIT_NOFILE, &flim);

    for (int fd = 0; fd < flim.rlim_max; fd++)
    {
        close(fd);
    }

    chdir("/");
}