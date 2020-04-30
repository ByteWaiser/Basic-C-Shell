#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


char error_message[30] = "An error has occurred\n";

void builtin_error(){
    write(STDERR_FILENO, error_message, strlen(error_message));
}

char *paths[100] = {"/bin"};

char *trim(char *s){
    char* back = s + strlen(s);
    while(isspace(*s)) s++;
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
    // last char will be strlen(s)-1
}

char **sep(char *s){
    char **temp = malloc(sizeof(char*) * 100);
    int i = 0;
    while(s != NULL){
        *(temp+i) = strsep(&s, " ");
        i++;
    }
    return temp;
}

void run(char **args){
    pid_t pid, wpid;
    int status;
    char *full_path = malloc(sizeof(char)*100);
    char *final_path = malloc(sizeof(char)*100);
    
    int i = 0;
    while(*(paths+i) != NULL){
        full_path = strcat(strcat(strcpy(full_path, *(paths+i)),"/"), args[0]);
        if(access(full_path, F_OK) == 0){
            strcpy(final_path, full_path);
        }
        i++;
    }

    pid = fork();
    if (pid == 0) {
        // Child process
        
        if (execv(final_path, args) == -1) {
            builtin_error();
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        builtin_error();
    } else {
        // Parent process
        do {
        wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

void process(char **args){
    if(args[0] == NULL){
        return;
    }

    if(strcmp(args[0], "exit") == 0){
        exit(0);
    }
    else if(strcmp(args[0], "cd") == 0){
        if(args[1] == NULL || args[2] != NULL){
            builtin_error();
        }else{
            if(chdir(args[1]) !=0) builtin_error();
        }
    }
    else if(strcmp(args[0], "path") == 0){
        if(args[1] == NULL){
            paths[0] = NULL;
        }
        int i = 0;
        while(*(args+i+1) != NULL){
            *(paths+i) = *(args + i + 1);
            printf("%s\n", paths[i]);
            i++;
        }
    }
    else if(paths[0] == NULL){
        builtin_error();
    }
    else{
        run(args);
    }
}

void shell_loop(){

    char *input;
    char **tokens;
    size_t size = 0;
    while(1){
        printf(">>> ");
        getline(&input, &size, stdin);
        input = trim(input);
        tokens = sep(input);
        process(tokens);
        free(input);
        free(tokens);
    }

}

int main(){

    shell_loop();

    return 0;
}
