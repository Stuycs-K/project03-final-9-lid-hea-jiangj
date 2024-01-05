#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


#ifndef NETWORKING_H
#define NETWORKING_H
#define PORT "19230"
#define BUFFER_SIZE 1024
void err(int i, char*message);
int server_setup();
int client_tcp_handshake(char*server_address);
int server_tcp_handshake(int listen_socket);
#define KEY 0x000022d7
#endif
