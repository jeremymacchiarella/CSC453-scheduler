#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>

int main(int argc, char** argv){

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    execvp(argv[1], &argv[1]);

    // If execvp fails, the following line will execute
    perror("execvp failed");
    return 1;

}