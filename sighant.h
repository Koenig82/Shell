#include <sys/types.h>
#include <signal.h>

typedef	void Sigfunc(int);

int kill(pid_t pid, int sig);

void signalCatcher(int theSignal);

Sigfunc* mySignal(int signo, Sigfunc *func);