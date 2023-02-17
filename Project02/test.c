#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    char* x = argv[1];

    printf("%s\n", x);

    return 0;
}