#include "networking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int server_socket = client_tcp_handshake(IP);
    printf("client connected.\n");
    clientLogic(server_socket);
    // while(1){
    //     clientLogic(server_socket);
    // }
}
