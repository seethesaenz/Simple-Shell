#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>



size_t MAX = 1024; // max length
int flag = 1;

void run(char *args[])
{
    pid_t pid;
    if (strcmp(args[0], "exit") ==0){
        flag = 0;
    }else if(strcmp(args[0], "cd") == 0)
    {
        if (chdir(args[1]) != 0){
            printf("Directory: %s, not found.", args[1]);
        }
    }else{
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "Fork Failed");
        }
        else if (pid == 0){
            execvp(args[0], args);
        }
    }


}

void piper(char *args[])
{
    int fd[2];
    pipe(fd);

    dup2(fd[1], 1);
    close(fd[1]);
    printf("args = %s\n", *args);

    run(args);

    dup2(fd[0], 0);
    close(fd[0]);
}

char * tokenize(char *input)
{
    int i;
    int j = 0;
    char *cleaned = (char* )malloc((MAX *2) * sizeof(char));

    //clean input to allow for easy reading
    for (i = 0; i< strlen(input); i++){
        if (input[i] == '|'){
            cleaned[j++] = ' ';
            cleaned[j++] = input[i];
            cleaned[j++] = ' ';
        }
        else{
            cleaned[j++] = input[i];
        }
    }
    cleaned[j++] = '\0';

    char *end;
    end = cleaned + strlen(cleaned)-1;
    end--;
    *(end+1) = '\0';

    return cleaned;
}

int main(void)
{
    char *args[MAX];

    while (flag){
            printf("sish> ");

            char *input = malloc(MAX * sizeof(char));
            getline(&input, &MAX, stdin);

            char *tokens;
            tokens = tokenize(input);

            char *arg = strtok(tokens, " ");
            int i = 0;
            while(arg) { 
                if (*arg == '|'){
                    args[i] = NULL;
                    piper(args);
                    i = 0;
                }else{
                    args[i] = arg;
                    i++;
                }
                arg = strtok(NULL, " ");
            }
            args[i] = NULL;

            run(args);
            free(input);
    }
    return 0;
}