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

// some global variables:
int StartProcess = 0;
int IsFinished = 0;
int IsLastOne = 0;
char WriteBuff[100];
float averageWTA = 0;
float averageWt = 0;
float cpuilization = 0;
int returning =0;
heap_t *AllProcesses;
heap_t *EndedProceess;
int msgqid1;
process p, RunningNowProcess;
process *ProcessVAR;
int rec_val;
OurMessage data;

//Functions declerations only: 
void GeneratingPerf(float cpu, float ta, float avgwait, float std);
void Waiting(int signum);
void SJF(FILE *logFile);
void SRT(FILE *logFile);
void RR(FILE *logFile);

int main(int argc, char **argv)
{
    FILE *logFile;
    logFile = fopen("Scheduler.log", "w");
    fprintf(logFile, "At\ttime\tx\tprocess y\tstate\t\tarr\tw\ttotal\tz\tremain\ty\twait\tk\n");
    int ChoosenAlg = atoi(argv[1]);

    RunningNowProcess.ID = 0;
    RunningNowProcess.Priority = -1;
    AllProcesses = (heap_t *)calloc(1, sizeof(heap_t));
    EndedProceess = (heap_t *)calloc(1, sizeof(heap_t));
    msgqid1 = msgget((key_t)292002, 0644 | IPC_CREAT);

    initClk();

    // recieve data of process in data
    rec_val= msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, !IPC_NOWAIT);
    while(true)
    {
   
        signal(SIGINT, Waiting);
        while (rec_val != -1)
        {
            if (data.ID == 0)
            {
                IsLastOne = 1;
                break;
            }
            // initialize the data of p to be like the recieved message
            p.ID = data.ID;
            p.Priority = data.Priority;
            p.RunTime = data.RunTime;
            p.ArrTime = data.ArrTime;
            p.PID = 0;
            p.RemaingTime = data.RunTime;
            p.WatingTime = 0;

           
            if (ChoosenAlg == 2 || ChoosenAlg == 3) // SJF or SRTN
            {
                push(AllProcesses, p.RunTime, &p);
            }
            else if (ChoosenAlg == 1) // RR
            {
                push(AllProcesses, p.ArrTime, &p);
            }

            rec_val = msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, IPC_NOWAIT);
        }
    

        if (ChoosenAlg == 1) // RR
        {
            RR(logFile);
            if(returning==-1)
            {
                break;
            }
        }

        else if (ChoosenAlg == 2) // SRTN
        {
           SRT(logFile);
           if(returning==-1)
            {
                break;
            }
          
        }

        else if (ChoosenAlg == 3) // SJF
        {
            SJF(logFile);
            if(returning==-1)
            {
                break;
            }
          
           
        }

        rec_val = msgrcv(msgqid1, &data, sizeof(data) - sizeof(data.mtype), 0, IPC_NOWAIT);
    
    }
    GeneratingPerf(cpuilization, averageWTA, averageWt, averageWt);
    fclose(logFile);
    printf("All Processes Are run, Check Log/Perf Files :) \n");
}

void GeneratingPerf(float cpu, float ta, float avgwait, float std)
{
    FILE *Perf = fopen("scheduler.perf", "w");
    fprintf(Perf, "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.2f\nStd WTA = %.2f", cpu, ta, avgwait, std);
    fclose(Perf);
}

void Waiting(int signum)
{
    int DummyVariable;
    wait(&DummyVariable);
    IsFinished = 1;
}

void SJF(FILE *logFile)
{
   
        if (top(AllProcesses) != NULL && (IsFinished == 1 || RunningNowProcess.ID == 0))
        {
            if (RunningNowProcess.ID != 0)
            {
                RunningNowProcess.RemaingTime = 0;
                RunningNowProcess.Running = 0;
                RunningNowProcess.Stoped = getClk();
                printf("Process ID: %d Has ended :) \n", RunningNowProcess.ID);
                averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                averageWt += (float)RunningNowProcess.WatingTime;
                cpuilization += RunningNowProcess.RunTime;
                sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.Stoped, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                fprintf(logFile,"%s" ,WriteBuff);

                push(EndedProceess, RunningNowProcess.ID, &RunningNowProcess);
            }
            
            ProcessVAR = pop(AllProcesses);
            RunningNowProcess.ID = ProcessVAR->ID;
            RunningNowProcess.PID = ProcessVAR->PID;
            RunningNowProcess.RunTime = ProcessVAR->RunTime;
            RunningNowProcess.ArrTime = ProcessVAR->ArrTime;
            RunningNowProcess.Priority = ProcessVAR->Priority;
            RunningNowProcess.WatingTime = ProcessVAR->WatingTime;
            RunningNowProcess.RemaingTime = ProcessVAR->RemaingTime;
            RunningNowProcess.Stoped = ProcessVAR->Stoped;
            
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
                    RunningNowProcess.startingTime = getClk();
                    RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.ArrTime);
                    RunningNowProcess.Running = 1;
                    printf("Process ID: %d Has Started :) \n", RunningNowProcess.ID);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstarted\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.startingTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile,"%s", WriteBuff);
                }
            }

            IsFinished = 0;
            StartProcess = getClk();
        }
        else if (top(AllProcesses) == NULL && IsLastOne == 1 && IsFinished)
        {
            RunningNowProcess.RemaingTime = 0;
            RunningNowProcess.Running = 0;
            RunningNowProcess.Stoped = getClk();
            push(EndedProceess, RunningNowProcess.ID, &RunningNowProcess);

            averageWt += (float)RunningNowProcess.WatingTime;
            averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
            cpuilization += (float)(RunningNowProcess.RunTime);
            cpuilization = 100 * (cpuilization / (float)getClk());
            averageWt = averageWt / (float)EndedProceess->len;
            averageWTA = averageWTA / (float)EndedProceess->len;
            sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID,  RunningNowProcess.Stoped , RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
            fprintf(logFile,"%s",WriteBuff);
            kill(getppid(), SIGINT);
            returning=-1;
            return;
          
          
        }
 
      
}

void SRT(FILE *logFile)
{
   
         if (top(AllProcesses) != NULL && (RunningNowProcess.Priority == -1 || top(AllProcesses)->RemaingTime < RunningNowProcess.RemaingTime || IsFinished == 1))
            {
                if (IsFinished == 0 && RunningNowProcess.ID != 0)
                {
                    printf("Process ID: %d Has Stopped :) \n", RunningNowProcess.ID);
                    kill(RunningNowProcess.PID, SIGSTOP);
                    RunningNowProcess.RemaingTime -= (getClk() - StartProcess);
                    RunningNowProcess.Stoped = getClk();
                    RunningNowProcess.Running = 0;
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile,"%s", WriteBuff);
                    push(AllProcesses, RunningNowProcess.RemaingTime, &RunningNowProcess);
                }
                else if (IsFinished == 1) 
                {
                    RunningNowProcess.RemaingTime = 0;
                    RunningNowProcess.Stoped = getClk();

                    averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                    averageWt += (float)RunningNowProcess.WatingTime;
                    cpuilization += RunningNowProcess.RunTime;
                    push(EndedProceess, RunningNowProcess.ID, &RunningNowProcess);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tstopped\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile,"%s", WriteBuff);
                    printf("Process ID: %d end\n", RunningNowProcess.ID);
                }
               
                ProcessVAR = pop(AllProcesses);
                RunningNowProcess.ID = ProcessVAR->ID;
                RunningNowProcess.PID = ProcessVAR->PID;
                RunningNowProcess.RunTime = ProcessVAR->RunTime;
                RunningNowProcess.ArrTime = ProcessVAR->ArrTime;
                RunningNowProcess.Priority = ProcessVAR->Priority;
                RunningNowProcess.WatingTime = ProcessVAR->WatingTime;
                RunningNowProcess.RemaingTime = ProcessVAR->RemaingTime;
                RunningNowProcess.Stoped = ProcessVAR->Stoped;
                
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
                        fprintf(logFile,"%s", WriteBuff);
                        printf("Process ID: %d Has Started \n", RunningNowProcess.ID);
                    }
                }
                else 
                {
                    RunningNowProcess.WatingTime += (getClk() - RunningNowProcess.Stoped);
                    kill(RunningNowProcess.PID, SIGCONT);
                    RunningNowProcess.Running = 1;
                    printf("Process ID: %d continue its work :)) \n", RunningNowProcess.ID);
                    sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tresumed\t\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime);
                    fprintf(logFile,"%s", WriteBuff);
                }

                IsFinished = 0;
                StartProcess = getClk();
            }
            else if (top(AllProcesses) == NULL && IsLastOne == 1 && IsFinished)
            {
                RunningNowProcess.RemaingTime = 0;
                RunningNowProcess.Running = 0;
                RunningNowProcess.Stoped = getClk();
                push(EndedProceess, RunningNowProcess.ID, &RunningNowProcess);
                averageWt += (float)RunningNowProcess.WatingTime;
                averageWTA += (float)((getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                cpuilization += (float)(RunningNowProcess.RunTime);
                cpuilization = 100 * (cpuilization / (float)getClk());
                averageWt = averageWt / (float)EndedProceess->len;
                averageWTA = averageWTA / (float)EndedProceess->len;
                sprintf(WriteBuff, "At\ttime\t%d\tprocess\t%d\tfinished\tarr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d\tWTA\t%0.2f\n", getClk(), RunningNowProcess.ID, RunningNowProcess.ArrTime, RunningNowProcess.RunTime, RunningNowProcess.RemaingTime, RunningNowProcess.WatingTime, (getClk() - RunningNowProcess.ArrTime), (float)(getClk() - RunningNowProcess.ArrTime) / RunningNowProcess.RunTime);
                fprintf(logFile,"%s", WriteBuff);
                kill(getppid(), SIGINT);
                returning=-1;
                return;
            }
    
}

void RR(FILE *logFile)
{
   
        
    
}