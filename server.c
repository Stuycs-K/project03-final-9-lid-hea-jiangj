#include "networking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void subserver_logic(int client_socket){
    int forum = open("forum.txt",O_WRONLY | O_APPEND);
    // Listens for a string (use the buffer size)
    char input[BUFFER_SIZE];
    read(client_socket, input, sizeof(input));
    // trim_input(input);
    write(forum, input, strlen(input));
    printf("%s", input);

    // Reply with the rot13 of the string.
    for (int i = 0; i < strlen(input); i ++){
        if (input[i] - 13 < 'a'){
            input[i] += 13;
        }
        else{
            input[i] -= 13;
        }
    }
    write(client_socket, input, sizeof(input));
  
}

int main(int argc, char *argv[] ) { 
    int forum = open("forum.txt",O_RDONLY);
    int listen_socket = server_setup(); 
    int numStrings = 0;
    while(1){
        int client_socket = server_tcp_handshake(listen_socket);
        numStrings++;
        pid_t f = fork();
        if (f == 0){  // child process
            subserver_logic(client_socket);
            exit(1);
        }
        else {
            printf("%d clients connected \n", numStrings);
        }
    }
}
