#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


char error_message[30] = "An error has occurred\n";

void builtin_error(){
    write(STDERR_FILENO, error_message, strlen(error_message));
}

char *paths[100] = {"/bin"};

char *trim(char *s){
    
    char* back = s + strlen(s);
    while(isspace(*s)) {
        s++;
    }
    while(isspace(*--back)){
       // printf("%d\n",*back);
    }
    *(back+1) = '\0';
    return s;
    // last char will be strlen(s)-1
}

char **trim_tokens(char **t){
    int i = 0;
    int j = 0;
    while(*(t+i) != NULL){
        if(strcmp(*(t+i), "") != 0){
            *(t+j) = *(t+i);
            *(t+j) = trim(*(t+j));
            j++;
        }
        else{
            *(t+i) = NULL;
        }
        i++;
    }
    return t;
}

char **sep(char *s){
    char **temp = malloc(sizeof(char*) * 100);
    char **full = malloc(sizeof(char*) * 100);
    int i = 0;
    while(s != NULL){
        *(temp+i) = malloc(sizeof(char*) * strlen(s));
        *(temp+i) = strsep(&s, " ");
        i++;
    }
    i = 0;
    int k = 0;
    while(*(temp+i) != NULL){
        while(*(temp+i) != NULL){
            *(full+k) = malloc(sizeof(char*) * strlen(*(temp+i)));
            *(full+k) = strsep((temp+i), "\t");
            k++;
        }
        i++;
    }
    
    return full;
}

void run(char **args){
    pid_t pid, wpid;
    int status;
    char *full_path = malloc(sizeof(char)*100);
    char *final_path = malloc(sizeof(char)*100);


    int out = 0;
    int j = 0;
    char **new_args = malloc(sizeof(char*) * 100);
    char *file_arg = malloc(sizeof(char) * 100);

    while(*(args+j) != NULL){
        if(strcmp(*(args+j), ">") == 0){
            out = 1;
            if(*(args+j+1)){
                strcpy(file_arg, *(args+j+1));
            }
            else{
                builtin_error();
                return;
            }
            break;
        }

        else if(strchr(*(args+j), '>') != NULL){
            char *pos = strchr(*(args+j), '>');
            if(pos == *(args+j)){
                strcpy(file_arg, pos+1);
            }
            else if(pos == (*(args+j)+strlen(*(args+j))-1)){
                new_args[j] = malloc(sizeof(char*) * strlen(*(args+j)));
                strcpy(new_args[j], strsep((args+j), ">"));
                if(*(args+j+1)){
                    strcpy(file_arg, *(args+j+1));
                }
                else{
                    builtin_error();
                    return;
                }
            }
            else{
                new_args[j] = malloc(sizeof(char*) * strlen(*(args+j)));
                strcpy(new_args[j], strsep((args+j), ">"));
                strcpy(file_arg, *(args+j));
            }
            out = 1;
            break;
        }
        new_args[j] = malloc(sizeof(char*) * strlen(*(args+j)));
        new_args[j] = *(args+j);
        j++;
    }

    int i = 0;
    while(*(paths+i) != NULL){
        strcat(strcat(strcpy(full_path, paths[i]),"/"), new_args[0]);
        if(access(full_path, F_OK) == 0){
            strcpy(final_path, full_path);
            break;
        }
        i++;
    }
     
    
    //printf("%saL\n",args[0]);
    pid = fork();
    if (pid == 0) {
        // Child process
        if(out == 1){
            int fd = open(file_arg, O_WRONLY | O_CREAT, 0600);
            dup2(fd, 1);   // make stdout go to file
            dup2(fd, 2);   // make stderr go to file - you may choose to not do this
            out = 0;
            close(fd); 
        }

        if (execv(final_path, new_args) == -1) {
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
            paths[i] = malloc(sizeof(char*) * strlen(*(args + i + 1)));
            strcpy(paths[i], *(args + i + 1));
            printf("Path: %s\n", paths[i]);
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
        printf("cen354sh>> ");
        getline(&input, &size, stdin);
        input = trim(input);
        tokens = trim_tokens(sep(input));
        process(tokens);
    }
    free(input);
    free(tokens);
}

int main(){

    shell_loop();

    return 0;
}
