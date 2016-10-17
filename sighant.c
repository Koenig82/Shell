#define _GNU_SOURCE
#include <signal.h>
#include "parser.h"
#include "sighant.h"

extern pid_t forks[MAXCOMMANDS+1];

void signalCatcher(int theSignal){

    if (theSignal == SIGINT) {
        for(int currentPid = 0; currentPid < MAXCOMMANDS; currentPid++){

            if (forks[currentPid]!=0){

                kill(forks[currentPid], SIGINT);
            }
        }
    }
    return;
}


Sigfunc * mySignal(int signo, Sigfunc *func) {
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef	SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return(SIG_ERR);
    return(oact.sa_handler);
}