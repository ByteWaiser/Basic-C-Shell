#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


char error_message[30] = "An error has occurred\n";

char *paths[] = {"/bin/"};

int paths_size(){
    return sizeof(paths) / sizeof(*paths);
}

char *builtin_list[] = {"exit", "cd", "path"};

int exit_cmd(){
    return 0;
}

int (*builtin_func[]) (char **) = {
    &exit_cmd,
};

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

int run(char **args){
    pid_t pid, wpid;
    int status;
    char *full_path = malloc(sizeof(char)*100);

    for(int i = 0; i < paths_size(); i++){
        

        strcat(strcpy(full_path, paths[i]), args[0]);
        printf("%s\n", full_path);
        if(access(full_path, X_OK) == 0) break;
    }
    
    
    
    pid = fork();
    if (pid == 0) {
        // Child process
        
        if (execv(full_path, args) == -1) {
            perror("cen354sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        printf("asdas\n");
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
        run(k);
    }

    return 0;
}
