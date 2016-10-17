#include <stdio.h>
#include <signal.h>
#include "main.c"

static void sigCatcherUSR1(int theSignal, int nrOfCommands, int forks[]) {
    if (theSignal == SIGUSR1) {
        for(int i = 0; i < nrOfCommands; i++){

        }
    }
}