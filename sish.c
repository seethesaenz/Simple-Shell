#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

size_t MAX =  1024; // max length
int flag = 1;



void piper(char *args[], int size) {
    int i;
    int j = 0;
    int num_cmds = 1;
    int cmd_start = 0;
    int fd[2];
    int in_fd = 0;
    //empty line
    if(args[0] == NULL){
        return;
    }
    //cd
    if (strcmp(args[0], "exit") == 0){
        flag = 0;
    }else if(strcmp(args[0], "cd")==0){
        if(chdir(args[i]) != 0){
            perror("Error changing directory");
        }
    }

    // Count the number of commands/pipes
    for (i = 0; i < size; i++) {
        if (args[i] == NULL) {
            num_cmds++;
        }
    }

    for (i = 0; i < num_cmds; i++) {
        // Find the start and end of the current command
        cmd_start = j;
        while (args[j] != NULL) {
            j++;
        }

        // Set up a pipe for all but the last command
        if (i < num_cmds - 1) {
            pipe(fd);
        }

        // Fork a child process to execute the current command
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork Failed");
        } else if (pid == 0) {
            // In the child process

            // Set up input redirection for all but the first command
            if (i > 0) {
                dup2(in_fd, 0);
                close(in_fd);
            }

            // Set up output redirection for all but the last command
            if (i < num_cmds - 1) {
                dup2(fd[1], 1);
                close(fd[1]);
                close(fd[0]);
            }

            // Execute the current command
            execvp(args[cmd_start], &args[cmd_start]);
            exit(0);
        } else {
            // In the parent process

            // Close the input file descriptor from the previous iteration
            if (i > 0) {
                close(in_fd);
            }

            // Set up the input file descriptor for the next iteration
            if (i < num_cmds - 1) {
                in_fd = fd[0];
                close(fd[1]);
            }

            // Wait for the child process to complete
            waitpid(pid, NULL, 0);

            j++;
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

    while (flag) {
        memset(args, 0, sizeof(args));

        printf("sish> ");

        char *input = malloc(MAX * sizeof(char));
        getline(&input, &MAX, stdin);

        char *tokens;
        tokens = tokenize(input);

        char *arg = strtok(tokens, " \n");
        int i = 0;
        while (arg) {
            if (*arg == '|') {
                args[i] = NULL;
                i++;
            } else {
                args[i] = arg;
                i++;
            }
            arg = strtok(NULL, " \n");
        }
        args[i] = NULL;

        piper(args, i);

        free(input);
    }
    return 0;
}
