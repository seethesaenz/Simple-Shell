#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

size_t MAX = 1024; // max length
int flag = 1;

void run(char *args[]) {

    // Debug statement
    for (int i = 0; args[i] != NULL; i++) {
        printf("args[%d]: %s\n", i, args[i]);
    }

    pid_t pid;
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
            if (execvp(args[0], args) < 0) {
                perror("Error executing command");
            }
            exit(0);
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
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);
        run(args);
        exit(0);
    } else {
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        waitpid(pid, NULL, 0);
        int i;
        for(i = 0; i< size; i++){
            args[i] = NULL;
        }
    }
}

char **tokenize(char *input) {
    int i = 0;
    char **tokens = malloc(MAX * sizeof(char *));
    char *arg = strtok(input, " \n");
    while (arg) {
        if (*arg == '|') {
            tokens[i] = NULL;
            i++;
            tokens[i] = arg;
            i++;
        } else {
            tokens[i] = arg;
            i++;
        }
        arg = strtok(NULL, " \n");
    }
    tokens[i] = NULL;
    return tokens;
}


int main(void) {
    char **args;

    while (flag) {
        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        args = tokenize(input);

        int i = 0;
        while (args[i]) {
            if (*args[i] == '|') {
                args[i] = NULL;
                piper(args, i);
                i++;
            } else {
                i++;
            }
        }

        run(args);
        free(input);
        free(args);
    }
    return 0;
}

