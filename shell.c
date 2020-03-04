/**
 * Simple shell interface starter kit program.
 * Operating System Concepts
 * Mini Project1
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_LINE		80 /* 80 chars per line, per command */

int main(void)
{
    char *args[MAX_LINE/2 + 1];
	/* command line (of 80) has max of 40 arguments */
    char *prevArgs[MAX_LINE/2 + 1];
    char *secondArgs[MAX_LINE/2 + 1]; /* for the history command */
    int hasPrevArgs = 0;
    int should_run = 1;

    while (should_run){
        printf("mysh:~$ ");
        fflush(stdout);

        /**
          * After reading user input, the steps are:
          * (1) fork a child process
          * (2) the child process will invoke execvp()
          * (3) if command includes &, parent and child will run concurrently
          */

        /* flags start */
        int redirectOutput = 0;
        int redirectInput = 0;
        int usePrevArgs = 0;
        int willPipe = 0;
        int notWaiting = 0;
        /* flags end */

        /* other variables */
        int indexPipe = 0;


        char *command = (char*)malloc(MAX_LINE + 1);
        fgets(command, MAX_LINE, stdin);
        int i = 0;
        char *token = (char*) malloc(MAX_LINE);
        token = strtok(command, " \n\t");
        while (token != NULL) {
            args[i] = (char*) malloc(MAX_LINE);
            strcpy(args[i], token);
            if (strcmp(token, "<") == 0) {
                redirectInput = 1;
            }
            if (strcmp(token, ">") == 0) {
                redirectOutput = 1;
            }
            if (strcmp(token, "|") == 0) {
                willPipe = 1;
                indexPipe = i;
            }
            token = strtok(NULL, " \n\t");
            i = i + 1;
        }
        args[i] = (char*) malloc(MAX_LINE);
        args[i] = NULL;
        
        if (args[0] == 0) {
            continue;
        }

        if (strcmp(args[0], "exit") == 0) {
            should_run = 0;
            continue;
        }

        if (strcmp(args[0], "!!") == 0) {
            usePrevArgs = 1;
            if (!hasPrevArgs) {
                printf("Error: no previous args\n");
                continue;
            }
            
        }

        if (strcmp(args[i-1], "&") == 0) {
            notWaiting = 1;
            args[i-1] = NULL;
        }

        /* redirection output start */
        int outfd;
        int normOutput;
        if (redirectOutput) {
            outfd = open(args[i-1], O_CREAT | O_RDWR | O_APPEND);
            if (outfd < 0) {
                printf("Error: cannot open\n");
                continue;
            }
            normOutput = dup(1);
            int res = dup2(outfd, 1);
            if (res < 0) {
                printf("Error: cannot dup2\n");
                continue;
            }
            args[i-1] = NULL;
            args[i-2] = NULL;
        }
        /* redirection output end */
        /* redirection input start */
        int infd;
        int normInput;
        if (redirectInput) {
            infd = open(args[i-1], O_RDONLY);
            if (infd < 0) {
                printf("Error: cannot open\n");
                continue;
            }
            normInput = dup(0);
            int res = dup2(infd, 0);
            if (res < 0) {
                printf("Error: cannot dup2\n");
                continue;
            }
            args[i-1] = NULL;
            args[i-2] = NULL;
        }
        /* redirection output end */

        /*piping setup starts*/
        int pipefd[2];
        int write_end = 1;
        int read_end = 0;
        if (willPipe) {
            pipe(pipefd);
            normOutput = dup(1);
            normInput = dup(0);
        }
        /* piping setup ends*/

        pid_t pid;
        int status; 
        pid = fork();

        if (pid < 0) {
            printf("Error: fork unsuccessful\n");
            continue;
        }
        /* parent process */
        else if (pid > 0) {
            if (!notWaiting){
                wait(&status);
            }
        }
        /* child process */
        else {
            /* perform some pipe function*/
            if (willPipe) {
                int newstatus;
                pid_t newpid = fork();
                if (newpid < 0) {
                    printf("Error: fork unsuccessful\n");
                    continue;
                }
                /* parent process with child pid executes 2nd commnd */
                else if (newpid > 0 ) {
                    int g;
                    int g1 = 0;
                    for (g = indexPipe + 1; g < i; g = g + 1) {
                        secondArgs[g1] = (char*) malloc(MAX_LINE + 1);
                        strcpy(secondArgs[g1], args[g]);
                        g1 = g1 + 1; 
                    }
                    secondArgs[g1] = (char*) malloc(MAX_LINE + 1);
                    secondArgs[g1] = NULL;
                    close(pipefd[write_end]);
                    dup2(pipefd[read_end], 0);
                    close(pipefd[read_end]);
                    /*wait(&newstatus);*/
                    if (execvp(secondArgs[0], secondArgs) < 0) {
                        printf("Error: cannot execute1\n");
                        continue;
                    }
                    else {
                        execvp(secondArgs[0], secondArgs);
                    }
                }
                /* child process execute first command*/
                else {
                    args[indexPipe] = NULL;
                    close(pipefd[read_end]);
                    dup2(pipefd[write_end], 1);
                    close(pipefd[write_end]);
                    if (execvp(args[0], args) < 0) {
                        printf("Error: cannot execute1\n");
                        continue;
                    }
                    else {
                        execvp(args[0], args);
                    }
                }
            }
            /* non-pipe function */
            else {
                /* check using the second args */
                if (!usePrevArgs) {
                    /* check waiting */
                    if (notWaiting) {
                        setpgid(0,0);
                        if (execvp(args[0],args) < 0) {
                            printf("Error: cannot execute command!\n");
                            exit(0);
                        }
                        execvp(args[0],args);
                    }
                    else {
                        if (execvp(args[0],args) < 0) {
                            printf("Error: cannot execute command!\n");
                            exit(0);
                        }
                        execvp(args[0],args);
                    }
                } 
                else {
                    /* check waiting */
                    if (notWaiting) {
                        setpgid(0,0);
                        if (execvp(prevArgs[0],prevArgs) < 0) {
                            printf("Error: cannot execute command!\n");
                            exit(0);
                        }
                        execvp(prevArgs[0],prevArgs);
                    }
                    else {
                        if (execvp(prevArgs[0],prevArgs) < 0) {
                            printf("Error: cannot execute command!\n");
                            exit(0);
                        }
                        execvp(prevArgs[0],prevArgs);
                    }
                } 
            }
        }
        /* clear out the args... maybe try a flush function?*/
        int k;
        int upperLimit = i;
        if (redirectInput) {
            upperLimit = upperLimit - 2;
            dup2(normInput, 0);
            close(normInput);
        }
        if (redirectOutput) {
            upperLimit = upperLimit - 2;
            dup2(normOutput, 1);
            close(normOutput);
        }
        if (notWaiting) {
            upperLimit = upperLimit - 1;
        }
        if (willPipe) {
            upperLimit = indexPipe + 1;
            dup2(normOutput,1);
            dup2(normInput,0);
        }
        for (k = 0; k < upperLimit; k = k + 1) {
            prevArgs[k] = (char*) malloc(MAX_LINE);
            strcpy(prevArgs[k], args[k]);
            free(args[k]);
        }
        prevArgs[k] = (char*) malloc(MAX_LINE);
        prevArgs[k] = NULL;
        hasPrevArgs = 1;
    }

    return 0;
}

