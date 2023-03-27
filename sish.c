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
    }
}

char **tokenize(char *input, int *num_cmds) {
    int i, j = 0;
    char *cleaned = (char *) malloc((MAX * 2) * sizeof(char));
    char **cmds = (char **) malloc(MAX * sizeof(char *));

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

    // tokenize individual commands
    char *cmd = strtok(cleaned, "|");
    i = 0;
    while (cmd) {
        cmds[i] = cmd;
        i++;
        cmd = strtok(NULL, "|");
    }
    cmds[i] = NULL;
    *num_cmds = i;

    free(cleaned);
    return cmds;
}

int main(void) {
    char *args[MAX];
    int num_cmds;

    while (flag) {
        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        char **cmds = tokenize(input, &num_cmds);
        int i = 0;
        int is_piping = 0;

        while (i < num_cmds) {
            char *tokens;
            tokens = strtok(cmds[i], " ");
            int j = 0;

            while (tokens != NULL) {
                if (*tokens == '|') {
                    is_piping = 1;
                    args[j] = NULL;
                    piper(args, j);
                    j = 0;
                } else {
                    args[j] = tokens;
                    j++;
                }
                tokens = strtok(NULL, " ");
            }
            args[j] = NULL;

            if (is_piping == 0) {
                run(args);
            }

            i++;
        }
        free(cmds);
        free(input);
    }
    return 0;
}
