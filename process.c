#include "headers.h"

/* Modify this file as needed*/
int main(int agrc, char **argv)
{
    initClk();
    int remainingtime = atoi(argv[1]);
    int currentC = getClk();
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
