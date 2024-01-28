#include "headers.h"

/* Modify this file as needed*/
int main(int agrc, char **argv)
{
    bool x = true;

    initClk();
    int remainingtime = atoi(argv[1]);
    int currentC = getClk();
    int sliceTime = 2;
    printf("slice time -> %d\n", sliceTime);
    printf("Current clock is %d\n", currentC);
    if (sliceTime > 0)
    {
        while (x)
        {
            if (currentC < getClk())
            {
                for (int i = 0; i < sliceTime; i++)
                {
                    printf("Remaimng Time -> %d\n", remainingtime);
                    remainingtime--;
                    char ProcessVAR[20];
                    if (remainingtime <= 0)
                    {
                        finished(1);
                        printf("Process Terminated :) \n");
                        kill(getppid(), SIGINT);
                        exit(0);
                        x=false;
                        break;
                    }
                    currentC = getClk();
                    finished(0);

                    finished(0);
                    if(i ==1)
                    {
                        x=false;
                    }
                }
            }
        }

        kill(getppid(), SIGINT);
        exit(0);
        return 1;
    }
    else
    {
        while (true)
        {
            if (currentC < getClk())
            {
                printf("Remaimng Time -> %d\n", remainingtime);
                remainingtime--;
                char ProcessVAR[20];
                if (remainingtime <= 0)
                {
                    finished(1);
                    break;
                }

                currentC = getClk();
                finished(0);
            }
        }
        printf("Process Terminated :) \n");
        kill(getppid(), SIGINT);
        exit(0);
    }
}
