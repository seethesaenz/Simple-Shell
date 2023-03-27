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

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork Failed");
    } else if (pid1 == 0) {
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);
        execvp(args[0], args);
        exit(0);
    } else {
        pid_t pid2 = fork();
        if (pid2 < 0) {
            perror("Fork Failed");
        } else if (pid2 == 0) {
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            execvp(args[size + 1], args + size + 1);
            exit(0);
        } else {
            close(fd[0]);
            close(fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}

char *tokenize(char *input) {
    int i;
    int j = 0;
    char *cleaned = (char *) malloc((MAX * 2) * sizeof(char));

    // clean input to allow for easy reading
    for (i = 0; i < strlen(input); i++) {
        if (input[i] == '|') {
            cleaned[j++] = ' ';
            cleaned[j++] = input[i];
            cleaned[j++] = ' ';
        } else {
            cleaned[j++] = input[i];
        }
    }
    cleaned[j++] = '\0';

    char *end;
    end = cleaned + strlen(cleaned) - 1;
    end--;
    *(end + 1) = '\0';

    return cleaned;
}

int main(void) {
    char *args1[MAX];
    char *args2[MAX];

    while (flag) {
        memset(args1, 0, sizeof(args1));
        memset(args2, 0, sizeof(args2));

        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        char *tokens;
        tokens = tokenize(input);

        char *arg = strtok(tokens, " \n");
        int i = 0;
        int j = 0;
        int pipe_found = 0;
        while (arg) {
            if (*arg == '|') {
                pipe_found = 1;
                i = 0;
            } else if (pipe_found) {
                args2[i] = arg;
                i++;
            } else {
                args1[i] = arg;
                i++;
            }
            arg = strtok(NULL, " \n");
        }
        args1[i] = NULL;
        args2[j] = NULL;

        if (pipe_found) {
            piper(args1, args2);
        } else {
            run(args1);
        }

        free(input);
    }
    return 0;
}
