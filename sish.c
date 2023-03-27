#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

size_t MAX =  1024; // max length
int flag = 1;

void run(char *args[]) {

    // Debug statement
    // for (int i = 0; args[i] != NULL; i++) {
    //     printf("args[%d]: %s\n", i, args[i]);
    // }

    pid_t pid;
    if (args[0] == NULL){
        return;
    }
    if (strcmp(args[0], "exit") == 0) {
        flag = 0;
    } else if (strcmp(args[0], "cd") == 0) {
        if (chdir(args[1]) != 0) {
            perror("Error changing directory");
        }
    } else {
        pid = fork();
        if (pid < 0) {
            perror("Fork Failed");
        } else if (pid == 0) {
            errno = 0;
            if (execvp(args[0], args) < 0) {
                perror("Error executing command");
                exit(1);
            }
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

void piper(char *args[], int size) {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Failed");
    } else if (pid == 0) {
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);
        run(args);
        exit(0);
    } else {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        waitpid(pid, NULL, 0);
        int i;
        for(i = 0; i< size; i++){
            args[i] = NULL;
        }
        args[size] = NULL;
    }
}

char **tokenize(char *input) {
    char **commands = (char **) malloc(MAX * sizeof(char *));
    int i = 0;

    char *command = strtok(input, "|");
    while (command != NULL) {
        commands[i++] = command;
        command = strtok(NULL, "|");
    }
    commands[i] = NULL;

    return commands;
}


int main(void) {
    char *args[MAX];

    while (flag) {
        memset(args, 0, sizeof(args));

        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        char **commands = tokenize(input);

        int i = 0;
        while (commands[i] != NULL) {
            char *arg = strtok(commands[i], " \n");
            int j = 0;
            while (arg) {
                args[j++] = arg;
                arg = strtok(NULL, " \n");
            }

            if (commands[i+1] != NULL) {
                // pipe to next command
                piper(args, j);
            } else {
                // last command, don't pipe
                run(args);
            }

            i++;
        }

        free(input);
        free(commands);
    }

    return 0;
}
