#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXPROC 2
#define BOOL char
#define FALSE 0
#define TRUE 1
//массив статусов дочерних процессов
//чтобы не обнулять массив объявляем его внешним
pid_t pid_list[MAXPROC];
//на случай непредвиденных обстоятельств храним число процессов
int pid_count = 0;

// int main()
// {
//     int i, p;
//     pid_t cpid;

//     for (p = 0; p < MAXPROC; p++)
//     {
//         cpid = fork();
//         switch (cpid)
//         {
//         case -1:
//             printf("Fork failed; cpid == -1\n");
//             break;
//         case 0:
//             cpid = getpid(); // global PID
//             for (i = 0; i < 5; i++)
//             {
//                 printf("%d: do something with child %d, pid = %d\n", i, p, cpid);
//                 sleep(p + 1);
//             }
//             printf("Exit child %d, pid = %d\n", p, cpid);
//             exit(0);
//         default:
//             pid_list[p] = cpid;
//             pid_count++;
//         }
//     }

//     while (pid_count)
//     {
//         cpid = waitpid(-1, NULL, 0); //ждем любого завершенного потомка
//         for (p = 0; p < MAXPROC; p++)
//         {
//             if (pid_list[p] == cpid)
//             {
//                 //делаем что-то по завершении дочернего процесса
//                 printf("Child number %d pid %d finished\n", p, cpid);
//                 pid_list[p] = 0;
//                 pid_count--;
//             }
//         }
//     }

//     return 0;
// }
//
// // definition of function
// int plusplus(int);

// int inccounter()
// {
//     static int c = 0; // variable becomes global (file-wide) via static keyword
//     c++;
//     return c;
// }

// // global func
// void donothing()
// {
// }

// // scope reduced to file
// static void local_donothing()
// {
// }

// int main()
// {
//     BOOL s = TRUE;

//     int foo;
//     int bar = 1;
//     int res = foo + bar;
//     printf("res=%d\n", res);

//     int numbers[2] = {1, 2};
//     int sum = 0;
//     for (size_t i = 0; i < 2; i++)
//     {
//         sum += numbers[i];
//     }
//     printf("sum=%d\n", sum);

//     char arr2d[2][3] = {
//         {'a', 'b', 'c'},
//         {'z', 'x', 'y'},
//     };

//     for (size_t i = 0; i < 2; i++)
//     {
//         for (size_t j = 0; j < 3; j++)
//         {
//             printf("%c ", arr2d[i][j]);
//         }
//         printf("\n");
//     }

//     char *s1 = "Unmutable string";
//     char s2[15] = "Mutable string";
//     s2[0] = 'm';

//     printf("s1=%s\n", s1);
//     printf("s2=%s\n", s2);

//     printf("len(s1)=%lu\n", strlen(s1));

//     if (strcmp(s1, s2) != 0)
//     {
//         printf("s1 and s2 are not equal\n");
//     }

//     char ssum[strlen(s1) + strlen(s2)];
//     strcpy(ssum, s2);
//     strcat(ssum, s1);
//     printf("s2+s1=%s\n", ssum);

//     printf("%d\n", plusplus(3));
//     donothing();

//     printf("counter=%d\n", inccounter());
//     printf("counter=%d\n", inccounter());
// }

// // implementation of earlier declared function
// int plusplus(int x)
// {
//     return x + 1;
// }

struct point
{
    int x;
    int y;
};

// basically helps us avoid using struct keyword
typedef struct
{
    int x;
    int y;
} POINT;

typedef struct
{
    char *name;
    int age;
} PERSON;

void move(POINT *p)
{
    p->x++; // same as (*p).x++;
    p->y++;
}

union intParts
{
    int theInt;
    char bytes[sizeof(int)];
};

struct operator
{
    int type;
    union
    {
        int intNum;
        float floatNum;
        double doubleNum;
        int intNum2;
    }; // no name!
    union
    {
        char *sp;
        char sa[10];
    } s;
};

union Coins {
    struct {
        int quarter;
        int dime;
        int nickel;
        int penny;
    }; // anonymous struct acts the same way as an anonymous union, members are on the outer container
    int coins[4];
};


int main()
{
    int a = 11;
    int *ap = &a;
    (*ap)++;
    printf("a=%d\n", *ap);

    struct point p;
    p.x = 10;
    p.y = 10;

    POINT p1 = {.x = 10, .y = 10};
    move(&p1);
    printf("x=%d, y=%d\n", p1.x, p1.y);

    // dunamically allocation
    PERSON *person = (PERSON *)malloc(sizeof(PERSON));
    person->name = "Yegor";
    person->age = 24;

    printf("Name=%s and Age=%d\n", person->name, person->age);
    free(person); // free allocated memory. Now *person points to some garbage

    char arr[] = {'a', 'b', 'c'};
    char *arr2 = (char *)malloc(3 * sizeof(char));

    arr2[0] = 'a';
    arr2[1] = 'b';
    arr2[2] = 'c';

    printf("%c %c\n", *(arr + 1), *(arr2 + 1));
    printf("%s %s\n", (arr + 1), (arr2 + 1)); // both char* type

    printf("%c %c\n", arr[1], arr2[1]);
    printf("%s %s\n", &arr[1], &arr2[1]); // both char* type

    free(arr2);

    // unions
    union intParts parts;
    parts.theInt = 5968145;

    printf("The int is %i\nThe bytes are [%i, %i, %i, %i]\n",
           parts.theInt, parts.bytes[0], parts.bytes[1], parts.bytes[2], parts.bytes[3]);

    int theInt = parts.theInt;
    printf("The int is %i\nThe bytes are [%i, %i, %i, %i]\n",
           theInt, *((char *)&theInt + 0), *((char *)&theInt + 1), *((char *)&theInt + 2), *((char *)&theInt + 3));

    struct operator oper;
    oper.type = 1;
    oper.intNum = 12;
    oper.s.sp = "1234567890";

    printf("intNum=%d, intNum2=%d, floatNum=%f doubleNum=%lf\n", oper.intNum, oper.intNum2, oper.floatNum, oper.doubleNum);
    printf("sa=%s sp=%s\n", oper.s.sa, oper.s.sp); // thought output would be equal
}

// void printInt(int i)
// {
//     printf("%d\n", i);
// }

// int rev(const void *a, const void *b)
// {
//     return *((int *)b) - *((int *)a);
// }

// int main()
// {
//     void (*pf)(int); // pointer to a function
//     pf = &printInt;
//     pf(10);

//     int arr[5] = {1, 2, 3, 4, 5};
//     int (*cmp)(const void *a, const void *b) = &rev;

//     qsort(arr, 5, sizeof(int), cmp);

//     for (size_t i = 0; i < 5; i++)
//     {
//         printf("%d ", arr[i]);
//     }
//     printf("\n");
// }
