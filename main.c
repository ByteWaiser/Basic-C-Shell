#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char *ltrim(char *s){
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s){
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s){
    return rtrim(ltrim(s));
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

int launch(char **args)
{
    pid_t pid, wpid;
    int status;
    char x[100] = "/bin/";
    char *path = strcat(x, args[0]);
    
    pid = fork();
    if (pid == 0) {
        // Child process
        if (execv(path, args) == -1) {
        perror("cen354sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("cen354sh");
    } else {
        // Parent process
        do {
        wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int main(){

    while(1){
        char *s;
        size_t a = 0;
        s = malloc(sizeof(char) * a);
        printf(">>> ");
        int x = getline(&s, &a, stdin);
        char** k;
        s = trim(s);
        k = sep(s);
        launch(k);
    }
    return 0;
}
