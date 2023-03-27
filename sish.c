#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

size_t MAX =  1024; // max length
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
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);

        // Create a new array to hold the first command for this process
        char *cmd1[size + 1];
        for (int i = 0; i < size; i++) {
            cmd1[i] = args[i];
        }
        cmd1[size] = NULL;

        run(cmd1);
        exit(0);
    } else {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);

        // Create a new array to hold the second command for this process
        char *cmd2[MAX];
        int i;
        for (i = size + 1; args[i] != NULL; i++) {
            cmd2[i - size - 1] = args[i];
        }
        cmd2[i - size - 1] = NULL;

        // Recursively call piper until all commands have been executed
        if (cmd2[0] != NULL) {
            piper(cmd2, i - size - 1);
        }
    }
}


char **tokenize(char *input, int *num_commands) {
    int i, j = 0;
    char *cleaned = (char *) malloc((MAX * 2) * sizeof(char));
    char **commands = (char **) malloc(MAX * sizeof(char *));
    *num_commands = 0;

    // clean input to allow for easy reading
    for (i = 0; i < strlen(input); i++) {
        if (input[i] == '|') {
            cleaned[j++] = ' ';
            cleaned[j++] = input[i];
            cleaned[j++] = ' ';
            (*num_commands)++;
        } else {
            cleaned[j++] = input[i];
        }
    }
    cleaned[j++] = '\0';

    char *token;
    int k = 0;
    token = strtok(cleaned, " \n");
    while (token != NULL) {
        commands[k] = token;
        k++;
        token = strtok(NULL, " \n");
    }
    commands[k] = NULL;

    return commands;
}


int main(void) {
    char **args;
    int num_commands;

    while (flag) {
        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        args = tokenize(input, &num_commands);

        if (num_commands > 0) {
            piper(args, num_commands);
        } else {
            run(args);
        }

        free(input);
    }

    return 0;
}
