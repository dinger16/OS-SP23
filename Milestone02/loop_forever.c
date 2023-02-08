#include <stdio.h>
#include <unistd.h>

int main() {


    while (1) {
        printf("%d\n", getpid());
    }
    return 0;
}