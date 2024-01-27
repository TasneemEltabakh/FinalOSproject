#include "headers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/msg.h>


//---------------- some initializations --------------
typedef enum scheduling_ChoosenAlgs_ {HPF, SRTN,SJF } scheduling_ChoosenAlgs;
scheduling_ChoosenAlgs chosen_alg;
void clearResources(int);
int uPrioirityQueue;
int ended=0;
void Waiting(int signum) //handeler that change ended form 0 to 1 when the scheduler finish.
{
    ended = 1;
}
//-----------------  read file  --------------------------------

/// @brief function to read inputs 
/// @param file_name 
/// @return a process list containing processes
heap_t* read_ints (const char* file_name)
{
  FILE* file = fopen (file_name, "r");
  process p[10];
  heap_t *AllProcesses = (heap_t *)calloc(1, sizeof(heap_t));

  
  for (int i = 0; i < 100; i++){
    if (!feof (file)){
      fscanf (file, "%d %d %d %d", &p[i].ID, &p[i].ArrTime, &p[i].RunTime, &p[i].Priority);
      push(AllProcesses,p[i].ArrTime,&p[i]);
      printf("list size:%d,ID:%d,ArrTime:%d\n",AllProcesses->len,p[i].ID,p[i].ArrTime);

    }
  }  

  fclose (file);   
  return AllProcesses;     
}
//-------------------  get ChoosenAlg num  ----------------------------------------

/// @brief function to return the ChoosenAlg number to be sent to schduler
/// @return char of the ChoosenAlg num
char* get_algo_num()
{
    if(chosen_alg ==HPF)
        return "1";
    if(chosen_alg ==SRTN)
        return "2";
    if(chosen_alg ==SJF)
        return "3";
}
//----------------  ask about which ChoosenAlg  ------------------------------------------------

/// @brief function to ask the user which ChoosenAlg to use 
/// @return the ChoosenAlg to be used in the enum of ChoosenAlgs
scheduling_ChoosenAlgs ask_for_alg(){
 
       
    int answer;
    scheduling_ChoosenAlgs chosed_alg;
    answer = -1;
    while(!(answer ==1 || answer ==2 | answer ==3))
    {
        printf("Please specifiy an ChoosenAlg for sceduling. \n 1: HPF \n 2: SRTN\n 3: SJF \n ");
        scanf("%i", &answer);

        switch (answer)
        {
        case 1:
            printf("Working with HPF \n ");
            return HPF;
            break;

         case 2:
            printf("Working with SRTN \n");
            return SRTN;
            break;
        case 3:
            printf("Working with SJF \n");
            return SJF;
            break;

        
        default:
            continue;
        }  
    }
}
//----------------  struct message  --------------------------------------------------

/// @brief to store the message
typedef struct
{
    long mtype;
    int ID;
    int ArrTime;
    int RunTime;
    int Priority;
    int memsize;

} OurMessage;
//------------------  fork clk  --------------------------------------------------

/// @brief function to fork the clk
void fork_clk()
{
    int pid;
    pid= fork();
    if (pid ==0)
    {
        printf("clk is runnint \n");
        char *args[]= {"./clk.out",NULL};
        execv(args[0],args);
        printf("didn'twork");
    }
    else{
        printf("out of clk\n");
    }
    
}
//---------------  fork scheduler  ------------------------------------------------------

/// @brief function to fork the scheduler 
/// @return pid
int fork_schedular()
{
    int pid;
    pid= fork();

    printf("check :%d",pid);
    if (pid ==0)
         {
        printf("check :%d",pid);
        char *args[]= {"./scheduler.out",get_algo_num(),NULL};
        execl("./scheduler.out","scheduler.out",get_algo_num(),NULL);
        perror("Error in execv'ing to scheduler");
        exit(EXIT_FAILURE);

         }
    else
    {
        return pid;
    }
}



//---------------  main  -----------------------------------------------------------
int main(int argc, char * argv[])
{
//-----------  some initializations  ----------------------
    key_t key_up =123;
    uPrioirityQueue = msgget(key_up, IPC_CREAT | 0644);
    signal(SIGINT, clearResources);
    char ChoosenAlg_num[20];
//---------------------------------------------------------
    // TODO Initialization
    // 1. Read the input files.
    heap_t *list = (heap_t *)calloc(1, sizeof(heap_t));
    list =read_ints("processes.txt");
//---------------------------------------------------------
    // 2. Ask the user for the chosen scheduling ChoosenAlg and its parameters, if there are any.
    chosen_alg =ask_for_alg();
//---------------------------------------------------------    
    // 3. Initiate and create the scheduler and clock processes.
    fork_clk();
    initClk();      //use this function to initialize the clock
    fork_schedular(ChoosenAlg_num);
//---------------------------------------------------------
    // TODO Generation Main Loop
    // 6. Send the information to the scheduler at the appropriate time.
    OurMessage mss;
    while(list->len>0)
    {
        if(top(list)->ArrTime <= getClk())
        {
        process* proc;
        proc =pop(list);
        mss.mtype = 1;
        mss.ArrTime= proc->ArrTime;
        mss.ID=proc->ID;
        mss.Priority=proc->Priority;
        mss.RunTime=proc->RunTime;
        printf("check if mss is sent %d\n", getClk());
        
        int send_val=msgsnd(uPrioirityQueue, &mss, sizeof(mss) - sizeof(mss.mtype), !IPC_NOWAIT);
        if(send_val == -1)//if no process is sent
        {
            perror("Error in receiving"); 
        } 
        }
    }
    printf("Sending!! Last");
    mss.ID=0;
    int send_val=msgsnd(uPrioirityQueue, &mss, sizeof(mss) - sizeof(mss.mtype), !IPC_NOWAIT);
    
    while(!ended){
    signal(SIGINT, Waiting); 
    }
//-------------------------------------------------------------
    // 7. Clear clock resources
    
    destroyClk(true);


}





void clearResources(int signum)
{
    printf("clearing resources \n");
    //TODO Clears all resources in case of interruption
    msgctl(uPrioirityQueue, IPC_RMID, (struct msqid_ds *) 0);
    exit(0);
}




