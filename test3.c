#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>

int main(int argc, char **argv){
    for (int i = 10; i <= 14; i+=2){
        printf("%d\n", i);
        sleep(1);
    }
    printf("finished test3\n");
}