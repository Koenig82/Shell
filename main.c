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

    char *line;
    command parsed[102];
    int commands;
    size_t line_size;

    do{
        printf("\nmish%%>");
        getline(&line, &line_size, stdin);
        if(!strncmp(line, "exit", 4)){
            break;
        }
        commands = parse(line, parsed);
        //print_command(parsed[1]);
        //printf("%d", commands);
        for(int i = 0; i < commands; i++){
            //printf("\n%d", i);
            pid_t newProcess = fork();
            printf("\nProcess %d exists", getpid());
            int pipeChild[2];
            if(i != 0){

                pipe(pipeChild);
            }
            if(newProcess < 0){
                perror("\nforkfail");
                exit(EXIT_FAILURE);
            }
            if(newProcess == 0) {
                printf("\nChild is starting exec\n");
                dupPipe(pipeChild, pipeChild[1], STDOUT_FILENO);
                execvp(parsed[i].argv[0], parsed[i].argv);
                exit(0);
            }
            printf("\n%d", i);

        }
        //parent process
        wait(0);

    }while(1);

    return 0;
}
//int main(void) {
//    //redirect_example();
//    //multiple_commands_example();
//    char input [100];
//    command parsed [101];
//    command kuk;
//    command kuk2;
//
//    printf("\nCpshell:");
//    char* input2 = "ls | grep main";
//    parse(input2, parsed);
//    kuk = parsed[0];
//    kuk2 = parsed[1];
//    pid_t pid;
//    int status;
//    int fd[2];
//    pipe(fd);           
//    if ((pid = fork()) != -1) {
//        if(pid == 0){         /*Child executing */
//            close(fd[0]);  // input pipe
//            close(1); //stdout
//            dup(fd[1]); // output pipe
//            close(fd[1]);
//            execvp(kuk.argv[0], kuk.argv); /* Here's stored the first instruction */
//
//        } else{             /* Parent executing */
//            wait(&status);
//            close(fd[1]); // close out
//            close(0); // close stdin
//            dup(fd[0]); //
//            close(fd[0]);
//            execvp(kuk2.argv[0], kuk2.argv); /* Here's stored the second instruction */
//        }
//    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
//
////    while(fgets(input, 100, stdin) != "exit"){
////        printf("\nCpshell:");
////        parse(input, parsed);
////        kuk = parsed[0];
////        kuk2 = parsed[1];
////        //print_command(kuk);
////        pid_t pid = fork();
////        if(pid == 0){
////            close(0)
////            execvp(kuk.argv[0], kuk.argv);
////        }
////        else{
////
////        }
////
////        //print_command(kuk2);
////        printf("\nCpshell:");
////    }
//
//    return 0;
//}
