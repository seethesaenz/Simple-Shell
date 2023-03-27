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

        // Create a copy of args before modifying it
        char **child_args = malloc((size+1) * sizeof(char *));
        for (int i = 0; i < size; i++) {
            child_args[i] = malloc(strlen(args[i]) + 1);
            strcpy(child_args[i], args[i]);
        }
        child_args[size] = NULL;

        waitpid(pid, NULL, 0);

        // Free the copy of args
        for (int i = 0; i < size; i++) {
            free(child_args[i]);
        }
        free(child_args);
    }
    return;
}



char **tokenize(char *input) {
    int i = 0;
    char **tokens = malloc(MAX * sizeof(char *));
    char *arg = strtok(input, " \n");
    while (arg) {
        tokens[i] = arg;
        i++;
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
        fflush(stdin);
        getline(&input, &MAX, stdin);

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        printf("Input: %s\n", input);

        args = tokenize(input);

        int next_arg_index = 0;
        while (args[next_arg_index]) {
            if (strcmp(args[next_arg_index], "|") == 0) {
                args[next_arg_index] = NULL;
                piper(args, next_arg_index);
                next_arg_index++;
            } else {
                next_arg_index++;
            }
        }


        run(args);
        free(input);
    }
    return 0;
}


