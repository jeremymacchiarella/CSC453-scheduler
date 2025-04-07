#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>

int main(int argc, char **argv){
    if (argc < 2){
        printf("need size arg\n");
        exit(1);
    }
    int num = atoi(argv[1]);

    for (int i = 0; i <= num; i+=2){
        printf("%d\n", i);
        sleep(1);
    }
    printf("finished test1\n");
}
