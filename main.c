#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/stdio.h>
#include "parser.h"
#include "execute.h"

#define STRCMP(a, R, b) strcmp(a, b) R 0

int forkProcess (int fd[2], int in, int out, command *cmd);
static const unsigned int MAX_COMMANDS = 2;


void print_command(command com) {
    int i = 0;

    printf("{\n");
    printf("  Argv: [");

    while (*(com.argv + i) != NULL) {
        /* Print each argument. */
        printf("\"%s\"", (*(com.argv + i)));
        if (i < com.argc - 1) printf(", ");

        i++;
    }
    printf("]\n");

    printf("  Argc: %d\n", com.argc);
    printf("  Infile: %s\n", com.infile);
    printf("  Outfile: %s\n", com.outfile);
    printf("}\n");
}

void redirect_example(void) {
    char* command_line_with_redirect = "foo < bar.txt";

    // You have to leave room for a NULL-terminating character
    // hence the +1
    command comLine[MAX_COMMANDS + 1];

    // The only command in "command_line_with_redirect" is "foo"
    int expected = 1;
    int actual = parse(command_line_with_redirect, comLine);
    assert(actual == expected);

    command foo = comLine[0];

    // The name of the command is "foo"
    assert(STRCMP(foo.argv[0], ==, "foo"));

    // No arguments where supplied to "foo"
    assert(foo.argc == 1);
    assert(foo.argv[1] == NULL);

    // foo < bar.txt sets the stdin of foo to the file descriptor of
    // bar.txt
    assert(STRCMP(foo.infile, ==, "bar.txt"));

    // The parser can only handle either "<" or ">", not both
    // simultaneously, hence "foo.outfile" is nothing
    assert(foo.outfile == NULL);

    printf("The command-line: \"foo < bar.txt\" yields a struct that "
                   "looks like this:\n");
    print_command(foo);
    printf("\n");
}

void multiple_commands_example(void) {
    char* command_line_with_pipe = "echo hi | grep hi";

    // You have to leave room for a NULL-terminating character
    // hence the +1
    command comLine[MAX_COMMANDS + 1];

    // We have two commands "echo" and "grep"
    int expected = 2;
    int actual = parse(command_line_with_pipe, comLine);
    assert(expected == actual);

    command echo = comLine[0];

    // The name of the command is "echo"
    assert(STRCMP(echo.argv[0], ==, "echo"));

    // One argument was supplied to "echo"
    assert(echo.argc == 2);
    assert(STRCMP(echo.argv[1], ==, "hi"));

    // There are no file-redirects,
    assert(echo.infile == NULL);
    assert(echo.outfile == NULL);

    command grep = comLine[1];

    // The name of the command is "grep"
    assert(STRCMP(grep.argv[0], ==, "grep"));

    // One argument was supplied to "grep"
    assert(grep.argc == 2);
    assert(STRCMP(grep.argv[1], ==, "hi"));

    // There are no file-redirects,
    assert(grep.infile == NULL);
    assert(grep.outfile == NULL);

    printf("The command-line: \"echo hi | grep hi\" yields two structs "
                   "that looks like this:\n");
    print_command(echo);
    printf("\n");
    print_command(grep);
}

int main(void){

    char* inputLine;

    command comLine[MAXCOMMANDS + 1];

    size_t line_size;

    int fd[2];

    int nrOfCommands;
    int in = STDIN_FILENO;
    int index;
    int pid;

    do{
        fprintf(stderr, "mish%% ");
        fflush(stderr);
        getline(&inputLine, &line_size, stdin);
        /*if(!strncmp(inputLine, "exit", 4)){
            break;
        }*/
        //parse the inputline into an array of commands
        nrOfCommands = parse(inputLine, comLine);
        //for each command exept the last one:
        for(index = 0; index < nrOfCommands-1; index++){
            //create a pipe with 2 filedescriptors and fork
            pipe(fd);
            //the first one will take input from stdin and the rest from the
            //write-end of the pipe
            printf("%d,%d ", fd[0], fd[1]);
            fflush(stdout);
            pid = forkProcess(fd, in, fd[1], &comLine[index]);
            if(close(fd[1]) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            //set in to the read-end on the latest pipe for the next child
            in = fd[0];

        }
        //the last or only command
        pid = fork();
        if (pid==0){
            //if in is not stdin
            if (in != STDIN_FILENO){
                //duplicate stdin to in and close in
                dup2 (in, STDIN_FILENO);
            }
            if(close(in) < 0){
                perror("close error:");
                exit(EXIT_FAILURE);
            }
            execvp(comLine[index].argv[0], comLine[index].argv);
            fprintf(stderr,"no such command");
            exit(EXIT_FAILURE);
        }

        //parent process
        /*close(fd[0]);
        close(fd[1]);*/
        wait(0);

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
        fprintf(stderr,"no such command");
        exit(EXIT_FAILURE);
    }

    return pid;
}