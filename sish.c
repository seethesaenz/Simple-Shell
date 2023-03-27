#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_HISTORY 100
size_t MAX =  1024; // max length
int flag = 1;
char *history[MAX_HISTORY];
int history_start = 0;
int history_end = 0;
int history_count = 0;

//add entry to history array
void add(char *cmd){
    //removing oldest entry if full
    if (history_count == MAX_HISTORY){
        free(history[history_start]);
        history_start = (history_start + 1) % MAX_HISTORY;
        history_count--;
    }
    //adding entry
    history[history_end] = strdup(cmd);
    history_end = (history_end + 1) % MAX_HISTORY;
    history_count++;
}
//prints history
void print_history(){
    int i;
    int j = history_start;
    for (i = 0; i < history_count; i++){
        printf("%d %s\n", i, history[j]);
        j = (j + 1) % MAX_HISTORY;
    }
}
//clears history
void clear_history(){
    int i;
    int j = history_start;
    for (i = 0; i < history_count; i++){
        free(history[j]);
        j = (j + 1) % MAX_HISTORY;
    }
    history_start = 0;
    history_count = 0;
    history_end = 0;
}
//history handler
void history_handler(char *args[]){
    if (args[1] == NULL){
        print_history();
    }else if (strcmp(args[1], "-c") == 0){
        clear_history();
    }else {
        int offset = atoi(args[1]);
        if(offset < 0 || offset >= history_count) {
            printf("invalid offset\n");
            return;
        }
        int index = (history_start+offset) % MAX_HISTORY;
        run_history(history[index]);
    }
}


void piper(char *args[], int size) {
    int i;
    int j = 0;
    int count = 1;
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
        if(chdir(args[1]) != 0){
            perror("Error changing directory");
        }
    }else if(strcmp(args[0], "history") == 0){
        history_handler(args);
        break;
    }
    //#number of commands
    for (i = 0; i < size; i++) {
        if (args[i] == NULL) {
            count++;
        }
    }

    for (i = 0; i < count; i++) {
        // start and end , j is end of command
        cmd_start = j;
        while (args[j] != NULL) {
            j++;
        }

        // pipe for every command except last
        if (i < count - 1) {
            pipe(fd);
        }

        //forking 
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork Failed");
        } else if (pid == 0) {
            //redirecting input of all except first
            if (i > 0) {
                dup2(in_fd, 0);
                close(in_fd);
            }

            //output all except last
            if (i < count - 1) {
                dup2(fd[1], 1);
                close(fd[1]);
                close(fd[0]);
            }

            //run cmd
            execvp(args[cmd_start], &args[cmd_start]);
            exit(0);
        } else {
            //close input from prior 
            if (i > 0) {
                close(in_fd);
            }
            //setup input for next
            if (i < count - 1) {
                in_fd = fd[0];
                close(fd[1]);
            }

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
//reruns history command at index
run_history(char *input){
    char *args[MAX];
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
