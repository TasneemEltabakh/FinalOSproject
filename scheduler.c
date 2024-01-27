#include "headers.h"
#include <math.h>

typedef struct
{
    long mtype;
    int ID;
    int ArrTime;
    int RunTime;
    int Priority;
    int memsize;

} OurMessage;

//some global variables:

int StartProject = 0;
int IsFinished = 0;
int IsLastOne = 0;
char WriteBuff[100];
float averageWTA = 0;
float averageWt = 0;
float cpuilization = 0;

void GeneratingPerf(float cpu, float ta, float avgwait, float std)
{
    FILE *Perf= fopen("scheduler.perf", "w");
    fprintf(Perf, "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f", cpu, ta, avgwait, std);
    fclose(Perf);
}

int main(int argc, char **argv)
{
    FILE *logFile;
    logFile = fopen("Scheduler.log", "w");
    fprintf(logFile, "At\ttime\tx\tprocess y\tstate\t\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n");
    int ChoosenAlg = atoi(argv[1]);
    process p, RunningNowProcess;
    RunningNowProcess.ID = 0;
    RunningNowProcess.Priority = -1;
    process *puff;
    heap_t *process_list = (heap_t *)calloc(1, sizeof(heap_t));
    heap_t *ProFinished = (heap_t *)calloc(1, sizeof(heap_t));
    OurMessage data;
    int msgqid1 = msgget((key_t)123, 0644 | IPC_CREAT);
    initClk();
    int rec_val = msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, !IPC_NOWAIT);
    while (true)
    {
        signal(SIGINT, handle);
        // Read the recived data.
        while (rec_val != -1)
        {
            if (data.ID == 0)
            {
                IsLastOne = 1;
                break;
            }
            p.ID = data.ID;
            p.Priority = data.Priority;
            p.RunTime = data.RunTime;
            p.ArrTime = data.ArrTime;
            p.PID = 0;
            p.RemaingTime = data.RunTime;
            p.WatingTime = 0;
            if (ChoosenAlg == 1) // HPF
            {
                push(process_list, p.Priority, &p);
            }
            else if (ChoosenAlg == 2 || ChoosenAlg == 3) // SJF or SRTN
            {
                push(process_list, p.RunTime, &p);
            }

            rec_val = msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, IPC_NOWAIT);
        }

        if (ChoosenAlg == 1) // Preemptive HPF
        {

            if (top(process_list) != NULL && (RunningNowProcess.Priority == -1 || top(process_list)->Priority < RunningNowProcess.Priority || IsFinished == 1))
            {

                // stop of RunningNowProcess processes
                if (IsFinished == 0 && RunningNowProcess.ID != 0)
                {
                    printf("Process ID: %d Stoped\n", RunningNowProcess.ID);
                    kill(RunningNowProcess.PID, SIGSTOP);
                    RunningNowProcess.RemaingTime -= (getClk() - StartProject);
                    RunningNowProcess.Stoped = getClk();
                    RunningNowProcess.Running = 0;
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile, WriteBuff);
                    push(process_list, RunningNowProcess.Priority, &RunningNowProcess);
                }
                else if (IsFinished == 1) // The current process has finished
                {
                    RunningNowProcess.RemaingTime = 0;
                    RunningNowProcess.Stoped = getClk();

                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    fprintf(logFile, WriteBuff);
                    averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    averageWt += (float)RunningNowProcess.WatingTime;
                    cpuilization += RunningNowProcess.RunTime;
                    push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);
                    printf("Process ID: %d end\n", RunningNowProcess.ID);
                }
                // Get the Next process.
                puff = pop(process_list);
                RunningNowProcess.ID = puff->ID;
                RunningNowProcess.PID = puff->PID;
                RunningNowProcess.RunTime = puff->RunTime;
                RunningNowProcess.ArrTime = puff->ArrTime;
                RunningNowProcess.Priority = puff->Priority;
                RunningNowProcess.WatingTime = puff->WatingTime;
                RunningNowProcess.RemaingTime = puff->RemaingTime;
                RunningNowProcess.Stoped = puff->Stoped;
                // fork the process if it new
                if (RunningNowProcess.PID == 0)
                {
                    int pid = fork();
                    if (pid == -1)
                        perror("error in fork");

                    else if (pid == 0)
                    {
                        char BurstTime[5];
                        char arrt[5];
                        sprintf(BurstTime, "%d", RunningNowProcess.RunTime);
                        printf(arrt, "%d", RunningNowProcess.ArrTime);
                        execl("./process.out", "process.out", BurstTime, NULL);
                    }
                    else
                    {
                        RunningNowProcess.PID = pid;
                        RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.ArrTime);
                        RunningNowProcess.Running = 1;
                        printf("Process ID: %d Starts \n", RunningNowProcess.ID);
                        sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                        fprintf(logFile, WriteBuff);
                    }
                }
                else // resume it if it old.
                {
                    RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.Stoped);
                    kill(RunningNowProcess.PID, SIGCONT);
                    RunningNowProcess.Running = 1;
                    printf("Process ID: %d Resume\n", RunningNowProcess.ID);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile, WriteBuff);
                }

                IsFinished = 0;
                StartProject = getClk();
            }
            else if (top(process_list) == NULL && IsLastOne == 1 && IsFinished)
            {
                RunningNowProcess.RemaingTime = 0;
                RunningNowProcess.Running = 0;
                RunningNowProcess.Stoped = getClk();
                push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);
                averageWt += (float)RunningNowProcess.WatingTime;
                averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                cpuilization += (float)(RunningNowProcess.RunTime);
                cpuilization = 100 * (cpuilization / (float)getClk());
                averageWt = averageWt / (float)ProFinished->len;
                averageWTA = averageWTA / (float)ProFinished->len;
                sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                fprintf(logFile, WriteBuff);
                kill(getppid(), SIGINT);
                break;
            }
        }

        else if (ChoosenAlg == 2) // SRTN
        {
            if (top(process_list) != NULL && (RunningNowProcess.Priority == -1 || top(process_list)->RemaingTime < RunningNowProcess.RemaingTime || IsFinished == 1))
            {
                if (IsFinished == 0 && RunningNowProcess.ID != 0)
                {
                    printf("Process ID: %d Stoped\n", RunningNowProcess.ID);
                    kill(RunningNowProcess.PID, SIGSTOP);
                    RunningNowProcess.RemaingTime -= (getClk() - StartProject);
                    RunningNowProcess.Stoped = getClk();
                    RunningNowProcess.Running = 0;
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile, WriteBuff);
                    push(process_list, RunningNowProcess.RemaingTime, &RunningNowProcess);
                }
                else if (IsFinished == 1) // The current process has finished
                {
                    RunningNowProcess.RemaingTime = 0;
                    RunningNowProcess.Stoped = getClk();

                    averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    averageWt += (float)RunningNowProcess.WatingTime;
                    cpuilization += RunningNowProcess.RunTime;
                    push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile, WriteBuff);
                    printf("Process ID: %d end\n", RunningNowProcess.ID);
                }
                // Get the Next process.
                puff = pop(process_list);
                RunningNowProcess.ID = puff->ID;
                RunningNowProcess.PID = puff->PID;
                RunningNowProcess.RunTime = puff->RunTime;
                RunningNowProcess.ArrTime = puff->ArrTime;
                RunningNowProcess.Priority = puff->Priority;
                RunningNowProcess.WatingTime = puff->WatingTime;
                RunningNowProcess.RemaingTime = puff->RemaingTime;
                RunningNowProcess.Stoped = puff->Stoped;
                // fork the process if it new
                if (RunningNowProcess.PID == 0)
                {
                    int pid = fork();
                    if (pid == -1)
                        perror("error in fork");

                    else if (pid == 0)
                    {
                        char BurstTime[5];
                        char arrt[5];
                        sprintf(BurstTime, "%d", RunningNowProcess.RunTime);
                        printf(arrt, "%d", RunningNowProcess.ArrTime);
                        execl("./process.out", "process.out", BurstTime, NULL);
                    }
                    else
                    {
                        RunningNowProcess.PID = pid;
                        RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.ArrTime);
                        RunningNowProcess.Running = 1;
                        sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                        fprintf(logFile, WriteBuff);
                        printf("Process ID: %d Starts \n", RunningNowProcess.ID);
                    }
                }
                else // resume it if it old.
                {
                    RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.Stoped);
                    kill(RunningNowProcess.PID, SIGCONT);
                    RunningNowProcess.Running = 1;
                    printf("Process ID: %d Resume\n", RunningNowProcess.ID);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile, WriteBuff);
                }

                IsFinished = 0;
                StartProject = getClk();
            }
            else if (top(process_list) == NULL && IsLastOne == 1 && IsFinished)
            {
                RunningNowProcess.RemaingTime = 0;
                RunningNowProcess.Running = 0;
                RunningNowProcess.Stoped = getClk();
                push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);
                averageWt += (float)RunningNowProcess.WatingTime;
                averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                cpuilization += (float)(RunningNowProcess.RunTime);
                cpuilization = 100 * (cpuilization / (float)getClk());
                averageWt = averageWt / (float)ProFinished->len;
                averageWTA = averageWTA / (float)ProFinished->len;
                sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                fprintf(logFile, WriteBuff);
                kill(getppid(), SIGINT);
                break;
            }
        }

        else if (ChoosenAlg == 3) // SJF
        {
            if (top(process_list) != NULL && (IsFinished == 1 || RunningNowProcess.ID == 0))
            {
                if (RunningNowProcess.ID != 0)
                {
                    RunningNowProcess.RemaingTime = 0;
                    RunningNowProcess.Running = 0;
                    RunningNowProcess.Stoped = getClk();
                    printf("Process ID: %d Stoped\n", RunningNowProcess.ID);
                    averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    averageWt += (float)RunningNowProcess.WatingTime;
                    cpuilization += RunningNowProcess.RunTime;
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    fprintf(logFile, WriteBuff);

                    push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);
                }
                // Get the Next process.
                puff = pop(process_list);
                RunningNowProcess.ID = puff->ID;
                RunningNowProcess.PID = puff->PID;
                RunningNowProcess.RunTime = puff->RunTime;
                RunningNowProcess.ArrTime = puff->ArrTime;
                RunningNowProcess.Priority = puff->Priority;
                RunningNowProcess.WatingTime = puff->WatingTime;
                RunningNowProcess.RemaingTime = puff->RemaingTime;
                RunningNowProcess.Stoped = puff->Stoped;
                // fork the process if it new
                if (RunningNowProcess.PID == 0)
                {
                    int pid = fork();
                    if (pid == -1)
                        perror("error in fork");

                    else if (pid == 0)
                    {
                        char BurstTime[5];
                        char arrt[5];
                        sprintf(BurstTime, "%d", RunningNowProcess.RunTime);
                        printf(arrt, "%d", RunningNowProcess.ArrTime);
                        execl("./process.out", "process.out", BurstTime, NULL);
                    }
                    else
                    {
                        RunningNowProcess.PID = pid;
                        RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.ArrTime);
                        RunningNowProcess.Running = 1;
                        printf("Process ID: %d Starts \n", RunningNowProcess.ID);
                        sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                        fprintf(logFile, WriteBuff);
                    }
                }

                IsFinished = 0;
                StartProject = getClk();
            }
            else if (top(process_list) == NULL && IsLastOne == 1 && IsFinished)
            {
                RunningNowProcess.RemaingTime = 0;
                RunningNowProcess.Running = 0;
                RunningNowProcess.Stoped = getClk();
                push(ProFinished, RunningNowProcess.ID, &RunningNowProcess);

                averageWt += (float)RunningNowProcess.WatingTime;
                averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                cpuilization += (float)(RunningNowProcess.RunTime);
                cpuilization = 100 * (cpuilization / (float)getClk());
                averageWt = averageWt / (float)ProFinished->len;
                averageWTA = averageWTA / (float)ProFinished->len;
                sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                fprintf(logFile, WriteBuff);
                kill(getppid(), SIGINT);
                break;
            }
        }

        rec_val = msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, IPC_NOWAIT);
    }
    GeneratingPerf(cpuilization, averageWTA, averageWt,averageWt);
    fclose(logFile);
    printf("Sch. Finished\n");
}

void handle(int signum)
{
    int x;
    wait(&x);
    IsFinished = 1;
}