#include "headers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/msg.h>
#define MAX_LINE_LENGTH 100

//global variables:
char x = '0';
char *ChoosenAlgo= &x;
int UPqueue;
int ended = 0;
typedef struct
{
    long mtype;
    int ID;
    int ArrTime;
    int RunTime;
    int Priority;
    int memsize;

} OurMessage;

void ClearAll(int signum)
{
    printf("Cleaning our resources, Bye:) \n");
    msgctl(UPqueue, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}

void IsEnded(int signum)
{
    ended = 1;
}

heap_t *ReadInputFile(const char *File)
{
    FILE *file = fopen(File, "r");
    process p[10];
    char line[MAX_LINE_LENGTH];
    heap_t *AllProcesses = (heap_t *)calloc(1, sizeof(heap_t));
     int count = 0;
     while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        // Ignore comments
        if (line[0] == '#') {
            continue;
        }
        sscanf(line, "%d %d %d %d", &p[count].ID, &p[count].ArrTime, &p[count].RunTime, &p[count].Priority);
        push(AllProcesses, p[count].ArrTime, &p[count]);
        count++;
     }


    fclose(file);
    return AllProcesses;
}


char *AlgorithmNeeded()
{
    int answer;
    answer = -1;

    while (!(answer == 1 || answer == 2 || answer == 3))
    {
        printf("Choose an Algorithm for your CPU, Press a number :)) .\n 1: RR\n 2: SRTN\n 3: SJF\n ");
        scanf("%i", &answer);

        switch (answer)
        {
        case 1:
            printf("We are working with RR now, Enjoy!\n");
            x = '1';
            ChoosenAlgo= &x;
            return ChoosenAlgo;
            break;

        case 2:
            printf("We are working with STRN now, Enjoy!\n");
            x = '2';
            ChoosenAlgo= &x;
            return ChoosenAlgo;
            break;
        case 3:
            printf("We are working with SJF now, Enjoy!\n");
            x = '3';
            ChoosenAlgo= &x;
            return ChoosenAlgo;
            break;

        default:
            printf("Invalid Choice :(\n");
            continue;
        }
    }
}

void ForkingClock()
{
    int pid;
    pid = fork();
    if (pid == 0)
    {
        printf("Clock is forkeddddd :) \n");
        char *args[] = {"./clk.out", NULL};
        execv(args[0], args);
        printf("Clock is not working :( ");
    }
    else
    {
        printf("Clock Ended\n");
    }
}

int ForkingSchedule(char * ChoosenAlgo)
{
    int pid;
    pid = fork();

    if (pid == 0)
    {
        char *args[] = {"./scheduler.out", ChoosenAlgo, NULL};
        execl("./scheduler.out", "scheduler.out", ChoosenAlgo, NULL);
        perror("Error in execv'ing to scheduler");
        exit(EXIT_FAILURE);
    }
    else
    {
        return pid;
    }
}

int main(int argc, char *argv[])
{
    key_t key_up = 292002;
    UPqueue = msgget(key_up, IPC_CREAT | 0644);
    signal(SIGINT, ClearAll);

    heap_t *list = (heap_t *)calloc(1, sizeof(heap_t));
    list = ReadInputFile("processes.txt");

    ChoosenAlgo= AlgorithmNeeded();

    ForkingClock();
    initClk();
    ForkingSchedule(ChoosenAlgo);

    OurMessage Message;
    while (list->len > 0)
    {
        if (top(list)->ArrTime <= getClk())
        {
            process *proc;
            proc = pop(list);
            Message.mtype = 1;
            Message.ArrTime = proc->ArrTime;
            Message.ID = proc->ID;
            Message.Priority = proc->Priority;
            Message.RunTime = proc->RunTime;

            int send_val = msgsnd(UPqueue, &Message, sizeof(Message) - sizeof(Message.mtype), !IPC_NOWAIT);
            if (send_val == -1)
            {
                perror("Error in receiving");
            }
        }
    }
    Message.ID = 0;
    int send_val = msgsnd(UPqueue, &Message, sizeof(Message) - sizeof(Message.mtype), !IPC_NOWAIT);

    while (!ended)
    {
        signal(SIGINT, IsEnded);
    }

    destroyClk(true);
}

