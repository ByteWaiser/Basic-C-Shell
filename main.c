//Needed headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

//The one and only error message
char error_message[30] = "An error has occurred\n";

void builtin_error(){
    write(STDERR_FILENO, error_message, strlen(error_message));
}

//Global variable for paths
char *paths[100] = {"/bin"};

//To trim whispaces
char *trim(char *s){
    //Works as changing back and front of string's address
    char* back = s + strlen(s);
    while(isspace(*s)) {
        s++;
    }
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
    // last char will be strlen(s)-1
}

//To clear unused tokens
char **trim_tokens(char **t){
    int i = 0;
    int j = 0;
    while(*(t+i) != NULL){
        //If string is not blank move its position to first not blank tokens position and trim again in case
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

//To seperate the tokens
char **sep(char *s){
    char **temp = malloc(sizeof(char*) * 300);
    char **full = malloc(sizeof(char*) * 300);
    int i = 0;
    //Seperate tokens using space delimeter and add them to a temp array
    while(s != NULL){
        *(temp+i) = malloc(sizeof(char*) * strlen(s));
        *(temp+i) = strsep(&s, " ");
        i++;
    }
    i = 0;
    int k = 0;
    //To clean the tabs from tokens clear and seperate every token again using delimeter \t
    //And add the to real tokens array which I call full
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

//To run the process within the path
void run(char **args){
    pid_t pid, wpid;
    int status;
    char *full_path = malloc(sizeof(char)*100);
    char *final_path = malloc(sizeof(char)*100);


    int out = 0;
    int j = 0;
    char **new_args = malloc(sizeof(char*) * 300);
    char *file_arg = malloc(sizeof(char) * 300);
    //Checkes every arg if one has > then make out = 1
    while(*(args+j) != NULL){
        //If an arg itself is equals to > copy next arg as a file_path if next arg is valid
        //Ex: ls .. > x or ls > x
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
        //If char > is in one of the args catch its position
        else if(strchr(*(args+j), '>') != NULL){
            char *pos = strchr(*(args+j), '>');
            //If it is at the start just take the string comes after > as file_arg
            //Ex: ls .. >x or ls >x
            if(pos == *(args+j)){
                strcpy(file_arg, pos+1);
            }
            //If it is at the last char just take the string comes before > as file_arg
            //Ex: ls ..> x or ls> x
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
            //If it is anywhere in the string just take the string comes after > as file_arg, comes before > as the last token
            //Ex: ls ..>x or ls>x
            else{
                new_args[j] = malloc(sizeof(char*) * strlen(*(args+j)));
                strcpy(new_args[j], strsep((args+j), ">"));
                strcpy(file_arg, *(args+j));
            }
            out = 1;
            break;
        }
        //If the input does not have > just go with the flow
        new_args[j] = malloc(sizeof(char*) * strlen(*(args+j)));
        new_args[j] = *(args+j);
        j++;
    }

    //This loop checks all path variables and see if it can access them
    //If it access copy the new path and first arg to final_path variable
    int i = 0;
    while(*(paths+i) != NULL){
        strcat(strcat(strcpy(full_path, paths[i]),"/"), new_args[0]);
        if(access(full_path, F_OK) == 0){
            strcpy(final_path, full_path);
            break;
        }
        i++;
    }
     
    
    //Fork - Wait - Exec functions
    pid = fork();
    if (pid == 0) {
        //Child process
        //To use redirect and write to a file
        if(out == 1){
            //Used 0600 for permissions and others to only create and write to file
            int fd = open(file_arg, O_WRONLY | O_CREAT, 0600);
            dup2(fd, 1);   //make stdout go to file
            dup2(fd, 2);   //make stderr go to file
            out = 0;
            close(fd); 
        }

        if (execv(final_path, new_args) == -1) {
            builtin_error();
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        //Error forking
        builtin_error();
    } else {
        //Parent process
        do {
        wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

//This checks and runs the args
void process(char **args){
    //Empty input
    if(args[0] == NULL){
        return;
    }
    
    //Exit builtin
    if(strcmp(args[0], "exit") == 0){
        exit(0);
    }
    //Cd builtin: prints error if entered more than one argument besides "cd"
    else if(strcmp(args[0], "cd") == 0){
        if(args[1] == NULL || args[2] != NULL){
            builtin_error();
        }else{
            if(chdir(args[1]) !=0) builtin_error();
        }
    }
    //Path builtin: If no argument pass than empties the path
    //Else copies args after path args to paths global variable
    else if(strcmp(args[0], "path") == 0){
        printf("Path: ");
        if(args[1] == NULL){
            int j = 0;
            while(*(paths+j) != NULL){
                *(paths+j) = NULL;
                j++;
            }
        }
        int i = 0;
        while(*(args+i+1) != NULL){
            paths[i] = malloc(sizeof(char*) * strlen(*(args + i + 1)));
            strcpy(paths[i], *(args + i + 1));
            printf("%s;", paths[i]);
            i++;
        }
        printf("\n");
    }
    else if(paths[0] == NULL){
        builtin_error();
    }
    else{
        run(args);
    }
}

//This is the shell loop
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
