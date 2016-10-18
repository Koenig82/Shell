#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include "parser.h"
#include "execute.h"
#include "sighant.h"
#include "mish.h"

pid_t forks[MAXCOMMANDS + 1];
void signalCatcher(int);
/*
 * The mainfunction of the program "mish".
 * This program creates a small shell with a limited amount of internal
 * commands and the ability to execute any external command with pipes
 * and redirects.
 */
int main(void){

    char* inputLine = NULL;

    command comLine[MAXCOMMANDS + 1];

    size_t line_size = 0;

    int fd[2];

    int nrOfCommands;
    int in = STDIN_FILENO;
    int index = 0;
    pid_t pid = 0;
    int pidstatus;
    ssize_t getlinestatus;

    //initialize signal handler
    if(signalHandler(SIGINT, signalCatcher) == SIG_ERR){
        fprintf(stderr, "Error setting signalhandler\n");
        fflush(stderr);
        perror("signal");
        exit(EXIT_FAILURE);
    }

    while(1){
        //set signalhandler to ignore during prompt
        signalHandler(SIGINT, SIG_IGN);
        //print prompt
        fprintf(stderr, "mish%% ");
        fflush(stderr);

        //get inpuline
        getlinestatus = getline(&inputLine, &line_size, stdin);
        if(getlinestatus == -1){
            break;
        }
        //-----execute internal commands-----
        //enter command
        if(!strcmp(inputLine, "\n")){
            free(inputLine);
            line_size = 0;
            continue;
        }
        //exit command
        if(!strncmp(inputLine, "exit", 4)){
            free(inputLine);
            line_size = 0;
            break;
        }
        //parse the inputline into an array of commands
        nrOfCommands = parse(inputLine, comLine);
        //cd command
        if(strcmp(comLine[0].argv[0],"cd") == 0){
            if(chdir(comLine[0].argv[1])!=0){
                fprintf(stderr, "cd: %s :", comLine[0].argv[1]);
                perror("");
            }
            free(inputLine);
            line_size = 0;
            continue;
        }
        //echo command
        if(strcmp(comLine[0].argv[0],"echo") == 0) {
            for (index = 1; index < comLine[0].argc; index++) {
                fprintf(stdout, "%s ", comLine[0].argv[index]);
            }
            fprintf(stdout,"\n");
            fflush(stdout);
            free(inputLine);
            line_size = 0;
            continue;
        }
        //-------execute external commands------
        //set signalhandler to check for ctrl+c
        if(signalHandler(SIGINT, signalCatcher) == SIG_ERR){
            fprintf(stderr, "Error setting signalhandler\n");
            fflush(stderr);
            perror("signal");
            exit(EXIT_FAILURE);
        }
        //for each command exept the last or only one:
        for(index = 0; index < nrOfCommands-1; index++){
            //handle redirect on the first command
            if(index == 0 && comLine[0].infile != NULL){
                in = redirect(comLine[0].infile, 0, STDIN_FILENO);
            }

            //create a pipe with 2 filedescriptors and fork
            pipe(fd);
            //the first one will take input from stdin and the rest from the
            //read-end of the pipe
            pid = forkProcess(fd, in, fd[1], &comLine[index]);
            if(pid == 0){
                free(inputLine);
                line_size = 0;
            }else{
                //add the pid to the array of pids
                forks[index] = pid;
            }
            if(in != 0){
                if(close(in) < 0){
                    perror("close error:");
                    exit(EXIT_FAILURE);
                }
            }

            if(close(fd[1]) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            //set in to the read-end on the latest pipe for the next child
            in = fd[0];

        }
        //the last or only command
        if((pid = fork()) < 0){
            perror("fork error:");
            exit(EXIT_FAILURE);
        }
        if(pid != 0){
            //add the pidnr to the pid-array
            forks[nrOfCommands - 1] = pid;
        }
        if (pid==0){
            free(inputLine);
            line_size = 0;
            //handle redirect on the only command
            if(nrOfCommands == 1 && comLine[0].infile != NULL) {

                in = redirect(comLine[0].infile, 0, STDIN_FILENO);
            }
            //handle redirect on the only command
            else if(comLine[index].outfile != NULL) {

                fd[1] = redirect(comLine[index].outfile, 1, STDOUT_FILENO);

                //duplicate fd[1] to stdout and close fd[1]
                dup2 (fd[1], STDOUT_FILENO);
                close(fd[1]);

            }

            //if in is not stdin
            if (in != STDIN_FILENO){
                //duplicate in to stdin and close in
                dup2 (in, STDIN_FILENO);
            }
            //close last in- filedescriptor if not the only command
            if(nrOfCommands != 1 && close(in) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            execvp(comLine[index].argv[0], comLine[index].argv);
            exit(EXIT_FAILURE);
        }
        //parent process
        //wait until all childprocesses has finished executing and remove them
        //from pidarray
        for(index = 0; index < nrOfCommands; index++){
            waitpid(forks[index], &pidstatus, WUNTRACED);
            forks[index] = 0;
        }

        free(inputLine);
        line_size = 0;
    }
    return 0;
}
/*
 * Function used to fork processes and connect between pipes inside a loop
 *
 * arguments: a piped filedescriptor array(not used), 2 filedescriptors
 * representing the in and out -end of a pipe and a commandstruct
 *
 * returnvalue: the new pid id
 */
pid_t forkProcess (int fd[2], int in, int out, command *cmd) {
    pid_t pid;
    if((pid = fork()) < 0){
        perror("fork error:");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        //if in is not stdin. ( at the first child it is)
        if (in != STDIN_FILENO) {
            //duplicate stdin to in and close in
            dupPipe(fd, in, STDIN_FILENO);
        }
        //if out is not stdout
        if (out != STDOUT_FILENO) {
            //duplicate stdout to out and close out
            dupPipe(fd, out, STDOUT_FILENO);
        }

        //execute the command
        execvp(cmd->argv[0], cmd->argv);
        exit(EXIT_FAILURE);
    }

    return pid;
}
