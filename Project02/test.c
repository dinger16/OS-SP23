#include <stdio.h>
#include <unistd.h>

int main(void) {
    char* args[2];
    args[0] = "not";
    args[1] = NULL;
    // char* ls = "ls";
    // char** command = {ls};
    int code = execvp(args[0], args);
    printf("%d\n", code);

    return code;
}