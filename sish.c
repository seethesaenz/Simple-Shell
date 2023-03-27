#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

size_t MAX = 1024; // max length
int flag = 1;

void run(char *args[], int fd_in) {
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
            if (fd_in != 0) {
                dup2(fd_in, 0);
                close(fd_in);
            }
            if (execvp(args[0], args) < 0) {
                perror("Error executing command");
            }
            exit(0);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}

void piper(char *cmd1[], int fd_in, int fd_out) {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork Failed");
    } else if (pid == 0) {
        dup2(fd_in, 0);
        if (fd_out != 1) {
            dup2(fd[1], 1);
        }
        close(fd[0]);
        close(fd[1]);
        run(cmd1, fd[0]);
        exit(0);
    } else {
        if (fd_out != 1) {
            close(fd[1]);
            piper(NULL, fd[0], fd_out);
        } else {
            close(fd[0]);
            waitpid(pid, NULL, 0);
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
    char *args[MAX];
    int fd_in = 0;
    char *input, *tokens, *arg;
    int i;

    while (flag) {
        printf("sish> ");

        input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        tokens = tokenize(input);
        arg = strtok(tokens, " ");
        i = 0;

        while (arg) {
            if (*arg == '|') {
                args[i] = NULL;
                piper(args, fd_in, STDOUT_FILENO);
                i = 0;
            } else {
                args[i++] = arg;
            }
            arg = strtok(NULL, " ");
        }

        args[i] = NULL;
        if (args[0] != NULL && strcmp(args[0], "cd") != 0 && strcmp(args[0], "exit") != 0) {
            run(args, fd_in);
        }
        free(input);
    }
    return 0;
}



