#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
    #define chdir _chdir
#else
    #include <unistd.h>
#endif

int main() {
    char cmd[1024];
    char cwd[1024];

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s> ", cwd);
        } else {
            printf("> ");
        }

        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }

        cmd[strcspn(cmd, "\r\n")] = '\0';

        if (strlen(cmd) == 0) {
            continue;
        }

        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        if (strncmp(cmd, "cd ", 3) == 0) {
            char *path = cmd + 3;
            while (*path == ' ') path++; 
            if (chdir(path) != 0) {
                perror("cd");
            }
        } else {
            system(cmd);
        }
    }

    return 0;
}