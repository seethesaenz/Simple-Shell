#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

size_t MAX = 1024; // max length
int flag = 1;

void run(char *args[]) {
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
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

void piper(char *args[]) {
    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork Failed");
    } else if (pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        run(args);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Fork Failed");
    } else if (pid2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        run(args+2);
        exit(0);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


char **tokenize(char *input) {
    int i, j;
    char **commands = (char **)malloc(MAX * sizeof(char *));
    for (i = 0; i < MAX; i++) {
        commands[i] = (char *)malloc(MAX * sizeof(char));
    }
    char *token = strtok(input, "|");
    i = 0;
    while (token != NULL) {
        j = 0;
        char *arg = strtok(token, " ");
        while (arg != NULL) {
            commands[i][j++] = *arg;
            arg = strtok(NULL, " ");
        }
        commands[i][j] = '\0';
        token = strtok(NULL, "|");
        i++;
    }
    commands[i] = NULL;
    return commands;
}

int main(void) {
    char **commands;

    while (flag) {
        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        commands = tokenize(input);

        int i = 0;
        while (commands[i] != NULL) {
            char **args = malloc(MAX * sizeof(char *));
            int j = 0;
            char *arg = strtok(commands[i], " ");
            while (arg != NULL) {
                args[j++] = arg;
                arg = strtok(NULL, " ");
            }
            args[j] = NULL;

            if (args[0] != NULL) {
                if (*args[0] == '|') {
                    piper(args);
                } else {
                    run(args);
                }
            }

            free(args);
            i++;
        }

        free(commands);
        free(input);
    }
    return 0;
}
