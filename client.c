#include "networking.h"

void clientLogic(int server_socket){
    // Prompts the user for a string.
    char input[BUFFER_SIZE];
    printf("Input a command (post or reply): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
    printf("About to write\n");
    write(server_socket,input,sizeof(input));
    printf("If statement about to run\n");
    if (strcmp(input,"post")==0) {
        printf("Enter your post: ");
        fgets(input, sizeof(input), stdin);
        printf("fgets: %s\n",input);
        // Send the user input to the client.
        write(server_socket, input, sizeof(input));

        // Read the modified string from the server
        read(server_socket, input, sizeof(input));


        // Prints the modified string
        printf("Modified string: %s\n", input);
    }
    else if (strcmp(input,"reply")==0) {
        printf("Reply isn't working rn!\n");
    }
    else {
        printf("Not a valid command!\n");
    }
}


int main(int argc, char *argv[] ) {
    char* IP = "127.0.0.1";
    if(argc>1){
        IP=argv[1];
    }
    //accessing semaphore
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

    //displaying the forum
    int server_socket = client_tcp_handshake(IP);
    char forum[BUFFER_SIZE];
    read(server_socket,forum,sizeof(forum));
    printf("%s\n",forum);
    printf("client connected.\n");
    clientLogic(server_socket);


    //downing semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);
    while(1){
        clientLogic(server_socket);
    }
}

