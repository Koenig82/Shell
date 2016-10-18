#include <sys/types.h>
#include <signal.h>

typedef	void Sigfunc(int);

void signalCatcher(int theSignal);

Sigfunc* signalHandler(int signo, Sigfunc *func);