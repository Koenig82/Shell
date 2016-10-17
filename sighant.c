#include <signal.h>
#include "parser.h"
#include "mish.c"
#include <sys/types.h>

void sigCatcher(int theSignal){
    if( theSignal == SIGINT){
        for(int i = 0; i < MAXCOMMANDS; i++){
            if(forks[i] != 0){
                kill(forks[i], SIGINT);
            }
        }
    }
}


