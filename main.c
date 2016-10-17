#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"
#include "execute.h"
#include "sighant.h"


int forkProcess (int fd[2], int in, int out, command *cmd);

int forks[MAXCOMMANDS + 1];

int main(void){

    char* inputLine;

    command comLine[MAXCOMMANDS + 1];

    size_t line_size;

    int fd[2];

    int nrOfCommands;
    int in = STDIN_FILENO;
    int index = 0;
    int pid;
    int pidstatus;


    sigCatcherUSR1(SIGUSR1);

    do{
        fflush(stderr);
        fprintf(stderr, "mish%% ");
        fflush(stderr);
        getline(&inputLine, &line_size, stdin);
        //parse the inputline into an array of commands
        nrOfCommands = parse(inputLine, comLine);
        //execute internal commands
        //enter command
        if(!strcmp(inputLine, "\n")){
            continue;
        }
        //exit command
        if(!strncmp(inputLine, "exit", 4)){
            break;
        }
        //cd command
        if(strcmp(comLine[0].argv[0],"cd") == 0){
            if(chdir(comLine[0].argv[1])!=0){
                fprintf(stderr, "cd: %s :", comLine[0].argv[1]);
                perror("");
            }
            continue;
        }
        //echo command
        if(strcmp(comLine[0].argv[0],"echo") == 0) {
            for (index = 1; index < comLine[0].argc; index++) {
                fprintf(stdout, "%s ", comLine[0].argv[index]);
            }
            fprintf(stdout,"\n");
            fflush(stdout);
            continue;
        }
        //execute external commands:
        //for each command exept the last one:
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
            if(in != 0){
                if(close(in) < 0){
                    perror("close error:");
                    exit(EXIT_FAILURE);
                }
            }
            //add the process to an array of processes
            forks[index] = pid;
            /*fprintf(stderr, "Closing fd out %d\n", fd[1]);
            fflush(stderr);*/
            if(close(fd[1]) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            //set in to the read-end on the latest pipe for the next child
            in = fd[0];

        }
        //the last or only command
        pid = fork();
        //add the process to an array of processes
        forks[nrOfCommands - 1] = pid;
        if (pid==0){
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
            /*fprintf(stderr, "Closing fd at end%d\n", in);
            fflush(stderr);*/
            if(nrOfCommands != 1 && close(in) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            execvp(comLine[index].argv[0], comLine[index].argv);
            exit(EXIT_FAILURE);
        }
        //parent process
        //wait until all childprocesses has finished executing
        for(index = 0; index < nrOfCommands; index++){
            waitpid(forks[index], &pidstatus, WUNTRACED);
        }


    }while(1);

    return 0;
}

int forkProcess (int fd[2], int in, int out, command *cmd) {
    pid_t pid;
    if((pid = fork()) == 0) {
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