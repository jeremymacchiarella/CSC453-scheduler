#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>

void startTimer(int time_ms, struct itimerval *timer){
    timer->it_value.tv_sec = time_ms/1000;
    timer->it_value.tv_usec = 0; //first trigger in time_ms 

    //makes timer not trigger again after first trigger
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, timer, NULL);

}
void handle_SIGALRM(int sig){
    printf("received signal\n");
}
int main(){
    struct sigaction sa;
    sa.sa_handler = handle_SIGALRM;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;


    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Error setting SIGALRM handler");
        return 1;
    }
    
    struct itimerval timer;
    startTimer(2000, &timer);
    while (1){
        printf("waiting\n");
        sleep(1);
    }


}