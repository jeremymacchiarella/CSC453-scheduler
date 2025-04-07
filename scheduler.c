#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <errno.h>

#define MAX_PROCESSES 128 
#define MAX_ARGS 10 
#define MIN_ARGS 3

//global variables for using in signal handlers
pid_t current_pid = -1; 
bool child_finished = false;
bool timer_flag = false;

void startTimer(int time_ms, struct itimerval *timer){
    timer->it_value.tv_sec = time_ms / 1000;
    timer->it_value.tv_usec = (time_ms % 1000) * 1000; //remainder in microseconds 

    //makes timer not trigger again after first trigger
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, timer, NULL);
}

void stopTimer(struct itimerval *timer){
    timer->it_value.tv_sec = 0;
    timer->it_value.tv_usec = 0;
  
    timer->it_interval.tv_sec = 0;
    timer->it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, timer, NULL);
}

void handle_SIGALRM(int sig) {
    //printf("SIGALRM received\n");
    timer_flag = true;
    if (!child_finished){
        kill(current_pid, SIGSTOP);
    }
    
    
}
void handle_SIGCHLD(int sig){
    
    pid_t pid;
    int status;

    pid = waitpid(current_pid, &status, WNOHANG);

    if (pid > 0){ //child that sent SIGCHLD sent it because they have FINISHED execution
        //printf("termination\n");
        child_finished = true;
    }

    
    


   
}



int main(int argc, char **argv){

    if (argc < MIN_ARGS){ //need at least quantum and name of program 
        printf("need at least quantum and program as args\n");
        exit(1);
    }
    int quantum;
    if ((quantum = atoi(argv[1])) == 0){ //user entered non number or 0
        printf("enter valid number for quantum\n");
        exit(1);
    }

    
    struct sigaction sa_SIGALRM;
    sa_SIGALRM.sa_handler = handle_SIGALRM;
    sigemptyset(&sa_SIGALRM.sa_mask);
    sa_SIGALRM.sa_flags = 0;


    if (sigaction(SIGALRM, &sa_SIGALRM, NULL) == -1) {
        perror("Error setting SIGALRM handler");
        return 1;
    }

    struct sigaction sa_SIGCHLD;
    sa_SIGCHLD.sa_handler = handle_SIGCHLD;
    sigemptyset(&sa_SIGCHLD.sa_mask);
    sa_SIGCHLD.sa_flags = 0;

    if (sigaction(SIGCHLD, &sa_SIGCHLD, NULL) == -1) {
        perror("Error setting SIGALRM handler");
        return 1;
    }


    char* processes[MAX_PROCESSES][MAX_ARGS + 2]; //extra 2 for pgrm name and null terminator

    int process_num = 0;
    int arg_idx = 0;
    for (int i = 2; i < argc; i++){ //skip program name and quantum
        if (strcmp(argv[i], ":") == 0){

            //if there is a ":" then there is a new command, so need to terminate the argument list with NULL
            processes[process_num][arg_idx] = NULL; 
            process_num++; //now on to parsing next process 
            arg_idx = 0; //with new process, restart arg_idx
        }
        else {
            if (arg_idx >= MAX_ARGS + 1){ //plus one to account for first arg being program name
                printf("process %d has too many args\n", process_num);
                exit(EXIT_FAILURE);
            }
            processes[process_num][arg_idx] = argv[i];
            arg_idx++;
            
        }
    }

    processes[process_num][arg_idx] = NULL; //null terminate final command
    process_num++; //makes process_num the correct number of process to run 
    pid_t child_pids[process_num]; //array to store child pids for scheduling

    

    

    // for (int i = 0; i < process_num; i++){
    //     int j = 0;
    //     do {
    //         if (processes[i][j] == NULL){
    //             printf("NULL");
    //         }
    //         else{
    //             printf("%s ", processes[i][j]);
    //         }
    //         j++;
    //     } while(processes[i][j-1] != NULL);
    //     printf("\n");
    // }

    
    for (int i = 0; i < process_num; i++){
        
        pid_t pid; 
        if ((pid = fork()) < 0){
            printf("error forking process %d\n", process_num);
            exit(1);
        }
        if (pid == 0){ //child
            int exec_result;
            //processes[i][0] is program name, processes[i] is command line args array
            
            
            if ((exec_result = execvp(processes[i][0], processes[i])) < 0){ 
                printf("error executing process %d\n", i);
                perror("failed exec");
                exit(1);

            } 
        }
        else{ //parent
            child_pids[i] = pid; //save child pid
            kill(pid, SIGSTOP); //pause child 

        }


    }

    //IMPLEMENT ROUND ROBIN SCHEDULING HERE
    int active_processes = process_num;

    bool processIsActive[process_num];
    //init isActive array with all trues
    for (int i = 0; i < process_num; i++){
        processIsActive[i] = true;
    }

    int i = 0;
    int status;
    struct itimerval process_timer;
    while (active_processes > 0){
        if (processIsActive[i]){

            current_pid = child_pids[i];
            
            startTimer(quantum, &process_timer);
            
            kill(child_pids[i], SIGCONT); //continue process i
            

            
            while (timer_flag == false && child_finished == false){
                
                pause(); //waits for signal, if it is a timer signal or chld finishing, while loop breaks
            }
            timer_flag = false;
            stopTimer(&process_timer);
            
            
            if (child_finished){
                 
                child_finished = false;
                //this waitpid is already done inside the signal handler for sig child
                //waitpid(child_pids[i], &status, 0); 
                active_processes--;
                processIsActive[i] = false;
            }
            

        }
        i++;
        if (i >= process_num){
            i = 0;
        }
        
    }


    return 0;
}