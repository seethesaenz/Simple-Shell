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
    int i = 0, num_tokens = 0;
    char **tokens;

    // Count the number of tokens in the input
    for (char *c = input; *c != '\0'; c++) {
        if (*c == ' ' || *c == '\n' || *c == '|') {
            continue;
        }
        num_tokens++;
        while (*c != ' ' && *c != '\n' && *c != '|' && *c != '\0') {
            c++;
        }
    }

    tokens = malloc((num_tokens + 1) * sizeof(char *));
    char *arg = strtok(input, " \n|");
    while (arg) {
        tokens[i++] = arg;
        arg = strtok(NULL, " \n|");
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

        // Tokenize the input by pipes
        char **pipe_tokens = tokenize(input, "|");
        int num_pipes = 0;
        while (pipe_tokens[num_pipes] != NULL) {
            num_pipes++;
        }

        // Create an array of pipe arrays to hold pipe arguments
        char ***pipe_args = malloc((num_pipes + 1) * sizeof(char **));
        for (int i = 0; i < num_pipes; i++) {
            pipe_args[i] = tokenize(pipe_tokens[i], " ");
        }
        pipe_args[num_pipes] = NULL;

        // Execute commands for each set of pipe arguments
        int i = 0;
        while (pipe_args[i]) {
            if (pipe_args[i+1]) {
                piper(pipe_args[i], count_tokens(pipe_args[i]));
            } else {
                run(pipe_args[i]);
            }
            i++;
        }

        // Free memory
        free(input);
        free_tokens(pipe_tokens);
        free_pipe_args(pipe_args);
    }

    return 0;
}



