#include "networking.h"

void clientLogic(int server_socket){
    // Prompts the user for a string.
    char input[BUFFER_SIZE];
    printf("Post something: ");
    fgets(input, sizeof(input), stdin);
    // Send the user input to the client.
    write(server_socket, input, sizeof(input));


    // Read the modified string from the server
    read(server_socket, input, sizeof(input));


    // Prints the modified string
    printf("Modified string: %s\n", input);
}


int main(int argc, char *argv[] ) {
    char* IP = NULL;
    if(argc>1){
        IP=argv[1];
    }
    // accessing semaphore
    printf("Waiting for server... This may take a moment\n");
    int semd;
    int *data;
    semd = semget(KEY, 1, 0);
    if(semd == -1){
        printf("error %d: %s\n", errno, strerror(errno));
        printf("Semaphore Does Not Yet Exist\n");
        exit(1);
    }
    // uping semaphore
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    sb.sem_op = -1;
    semop(semd, &sb, 1);

    int server_socket = client_tcp_handshake(IP);
    printf("client connected.\n");
    clientLogic(server_socket);

    int *posts;
    int shmid02;
    shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    posts = shmat(shmid02, 0, 0);

    // downing semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);
    // while(1){
    //     clientLogic(server_socket);
    // }
}

