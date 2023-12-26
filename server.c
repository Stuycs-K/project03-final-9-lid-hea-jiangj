#include "networking.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#define KEY 142325

// // trim_input trims the input for newlines to be used for parse_args and execute
// // args:        input, the command to be ran
// // returns:     no return value  
// void trim_input(char *input) {
//     for(int x = 0; x < strlen(input); x++){
//         if (strcmp(&input[x], "\n") == 0 || strcmp(&input[x], "\r") == 0){
//             input[x] = 0; // removes new line
//             break;
//         }
//     }
// }

static void sighandler( int signo ) {
    if (signo == SIGINT) {
        int shmid;
        shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        shmctl(shmid, IPC_RMID, 0);
        exit(0);
    }
}

void subserver_logic(int client_socket){
    int forum = open("forum.txt",O_WRONLY | O_APPEND);
    // Listens for a string (use the buffer size)
    char input[BUFFER_SIZE];
    read(client_socket, input, sizeof(input));
    // trim_input(input);
    int *data;
    int shmid;
    shmid = shmget(KEY, sizeof(int), 0640);
    data = shmat(shmid, 0, 0); //attach
    int i = *data + 1;
    *data = *data + 1;
    char new_input[BUFFER_SIZE+10];
    sprintf(new_input, "p%d: %s",i,input);
    printf("%ld\n",strlen(new_input));
    write(forum, new_input, strlen(new_input));
    printf("%s", new_input);
    write(client_socket, new_input, strlen(new_input));
    shmdt(data); //detach
  
}

int main(int argc, char *argv[] ) { 
    int forum = open("forum.txt",O_RDONLY);
    FILE* forum1 = fopen("forum.txt","r");
    int listen_socket = server_setup(); 
    int numStrings = 0;
    int *data;
    int shmid;
    shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
    data = shmat(shmid, 0, 0); //attach
    printf("*data: %d\n", *data);
    char line[BUFFER_SIZE];
    
    while (fgets(line,sizeof(line),forum1)) {
        if (line[0]=='p') *data = *data + 1;
    }
    printf("*data: %d\n", *data);
    shmdt(data); //detach
    signal(SIGINT,sighandler);

    while(1){
        int client_socket = server_tcp_handshake(listen_socket);
        numStrings++;
        pid_t f = fork();
        if (f == 0){  // child process
            subserver_logic(client_socket);
            close(client_socket);
            exit(1);
        }
        else {
            printf("%d clients connected \n", numStrings);
            close(client_socket);
        }
    }
}
