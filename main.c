#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXPROC 2

//массив статусов дочерних процессов
//чтобы не обнулять массив объявляем его внешним
pid_t pid_list[MAXPROC];
//на случай непредвиденных обстоятельств храним число процессов
int pid_count = 0;

int main()
{
    int i, p;
    pid_t cpid;

    for (p = 0; p < MAXPROC; p++)
    {
        cpid = fork();
        switch (cpid)
        {
        case -1:
            printf("Fork failed; cpid == -1\n");
            break;
        case 0:
            cpid = getpid(); // global PID
            for (i = 0; i < 5; i++)
            {
                printf("%d: do something with child %d, pid = %d\n", i, p, cpid);
                sleep(p + 1);
            }
            printf("Exit child %d, pid = %d\n", p, cpid);
            exit(0);
        default:
            pid_list[p] = cpid;
            pid_count++;
        }
    }

    while (pid_count)
    {
        cpid = waitpid(-1, NULL, 0); //ждем любого завершенного потомка
        for (p = 0; p < MAXPROC; p++)
        {
            if (pid_list[p] == cpid)
            {
                //делаем что-то по завершении дочернего процесса
                printf("Child number %d pid %d finished\n", p, cpid);
                pid_list[p] = 0;
                pid_count--;
            }
        }
    }

    return 0;
}
