#include "networking.h"

/*Connect to the server
 *return the to_server socket descriptor
 *blocks until connection is made.*/
int client_tcp_handshake(char * server_address) {

  //getaddrinfo
  
  int serverd;
  struct addrinfo * hints, * results;
  hints = calloc(1,sizeof(struct addrinfo));
  hints->ai_family = AF_INET;
  hints->ai_socktype = SOCK_STREAM; //TCP socket
  // hints->ai_flags = AI_PASSIVE; //only needed on server
  getaddrinfo(server_address, "9845", hints, &results);

  //create socket
  int sd = socket(AF_INET, SOCK_STREAM, 0); 
  // struct addrinfo hints, results; 
  //connect to the server
  connect(sd, results->ai_addr, results->ai_addrlen);
  
  free(hints);
  freeaddrinfo(results);
  serverd = sd;
  return serverd;
}

/*Accept a connection from a client
 *return the to_client socket descriptor
 *blocks until connection is made.
 */
int server_tcp_handshake(int listen_socket){
    //accept the client connection
    
    int client_socket;
    socklen_t sock_size;
    struct sockaddr_storage client_address;
    sock_size = sizeof(client_address);
    if ((client_socket = accept(listen_socket, (struct sockaddr *)&client_address, &sock_size)) < 0) {
        perror("Error accepting connection");
        return -1;  // or handle the error appropriately
    }
    
    //DO STUFF

    // free(hints);
    // freeaddrinfo(results);  
    return client_socket;
}

/*Create and bind a socket.
* Place the socket in a listening state.
*/
int server_setup() {
  //setup structs for getaddrinfo
  
  //create the socket
  struct addrinfo * hints, * results;
  hints = calloc(1,sizeof(struct addrinfo));
  hints->ai_family = AF_INET;
  hints->ai_socktype = SOCK_STREAM; //TCP socket
  hints->ai_flags = AI_PASSIVE; //only needed on server
  getaddrinfo(NULL, "9845", hints, &results);  //Server sets node to NULL
  
  //create socket
  int sd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
  //use bind
  bind(sd, results->ai_addr, results->ai_addrlen);

  int clientd = sd;

  //this code should get around the address in use error
  int yes = 1;
  int sockOpt =  setsockopt(clientd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  err(sockOpt,"sockopt  error");
  
  //bind the socket to address and port
  
  //set socket to listen state
  listen(sd, 10);

  //free the structs used by getaddrinfo
  free(hints);
  freeaddrinfo(results);
  
  return clientd;
}

void err(int i, char*message){
  if(i < 0){
	  printf("Error: %s - %s\n",message, strerror(errno));
  	exit(1);
  }
}

void file_to_string(const char* filename, char *accum) {
    // FILE* file = fopen(filename, "r");
    // if (file == NULL) {
    //     perror("fopen failure");
    //     exit(1);
    // }

    // char* accum = malloc(1); 
    // char line[BUFFER_SIZE]; 

    // while (fgets(line, sizeof(line), file) != NULL) {
    //     char* new_content = realloc(accum, strlen(accum) + strlen(line) + 1); // to increase the length of accum
    //     if (new_content == NULL) {
    //         perror("realloc error");
    //         exit(1);
    //     }

    //     accum = new_content;
    //     strcat(accum, line);
    // }

    // fclose(file);
    // return accum;
    int file = open(filename, O_RDONLY, 0666);
    char buff[BUFFER_SIZE] = "";
    int byte;
    while(byte = read(file, buff, BUFFER_SIZE)) {
        strcat(accum, buff);
    }
    accum[strlen(accum)] = '\0';
    close(file);
}