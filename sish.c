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
void piper(char *cmd1[], char *cmd2[]) {
    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork Failed");
    } else if (pid1 == 0) {
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);
        execvp(cmd1[0], cmd1);
        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Fork Failed");
    } else if (pid2 == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execvp(cmd2[0], cmd2);
        exit(0);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
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
    char ***args = malloc(MAX * sizeof(char **));
    for (int i = 0; i < MAX; i++) {
        args[i] = malloc(MAX * sizeof(char *));
    }
    char **cmds[MAX];
    int cmd_count = 0;

    while (flag) {
        memset(args, 0, MAX * sizeof(char **));
        memset(cmds, 0, sizeof(cmds));
        cmd_count = 0;

        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        char *tokens;
        tokens = tokenize(input);

        char *arg = strtok(tokens, " \n");
        int i = 0;
        int arg_count = 0;
        while (arg) {
            if (*arg == '|') {
                args[arg_count][i] = NULL;
                cmds[cmd_count] = args[arg_count];
                cmd_count++;
                arg_count++;
                i = 0;
            } else {
                args[arg_count][i] = arg;
                i++;
            }
            arg = strtok(NULL, " \n");
        }
        args[arg_count][i] = NULL;
        cmds[cmd_count] = args[arg_count];
        cmd_count++;

        for (int j = 0; j < cmd_count; j++) {
            if (j < cmd_count - 1) {
                piper(cmds[j], cmds[j + 1]);
                j++;
            } else {
                run(cmds[j]);
            }
        }

        free(input);
    }

    for (int i = 0; i < MAX; i++) {
        free(args[i]);
    }
    free(args);

    return 0;
}
