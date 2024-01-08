#include "networking.h"

void clientLogic(int server_socket){
//    while(1){
    // Prompts the user for a string.
    char input[BUFFER_SIZE];
    read(server_socket, input, sizeof(input));
    printf("%s", input);
        
    printf("Input a command (post, read, reply, edit): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
    // printf("About to write\n");
    write(server_socket,input,sizeof(input));
    // printf("If statement about to run\n");
    if (strcmp(input,"post")==0) {
        printf("Enter your post: ");
        fgets(input, sizeof(input), stdin);
        // printf("fgets: %s\n",input);
        // Send the user input to the client.
        write(server_socket, input, sizeof(input));

    // Read the modified string from the server
    // read(server_socket, input, sizeof(input));


    // Prints the modified string
    //        printf("%s", input);
    }
    else if (strcmp(input,"reply")==0) {
        printf("Reply isn't working rn!\n");
        // printf("Insert a post to view (p#): ");
        // fgets(input, sizeof(input), stdin);
        // write(server_socket, input, sizeof(input));
    }
    else if(strcmp(input, "read") == 0){
        printf("Which post would you like to edit?(# only): ");
        fgets(input, sizeof(input), stdin);
        char post_name[BUFFER_SIZE];
        int num;
        sscanf(input, "%d", &num);
        sprintf(post_name, "p%d", num);
        int post = open(post_name, O_RDONLY, 0);
        char* content = file_to_string(post_name);
        printf("Current content %s: \n%s", post_name, content);
        close(post);
    }
    else if(strcmp(input, "edit") == 0){
        printf("Which post would you like to edit?(# only): ");
        fgets(input, sizeof(input), stdin);
        char post_name[BUFFER_SIZE];
        int num;
        sscanf(input, "%d", &num);
        sprintf(post_name, "p%d", num);
        int post = open(post_name, O_RDONLY, 0);
        char* content = file_to_string(post_name);
        printf("Current content %s: \n%s", post_name, content);
        close(post);

        printf("New content: ");
        fgets(input, sizeof(input), stdin);
        printf("%s", input);
        post = open(post_name, O_WRONLY | O_TRUNC, 0666);
        write(post, input, sizeof(input));
        close(post);
    }
    else {
        printf("Not a valid command!\n");
    }
//    }
}


int main(int argc, char *argv[] ) {
    char* IP = NULL;
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
    // struct sembuf sb;
    // sb.sem_num = 0;
    // sb.sem_flg = SEM_UNDO;
    // sb.sem_op = -1;
    // semop(semd, &sb, 1);

    //displaying the forum
    int server_socket = client_tcp_handshake(IP);
    //    printf("client connected.\n");
    clientLogic(server_socket);

    int *posts;
    int shmid02;
    shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    posts = shmat(shmid02, 0, 0);

    //downing semaphore
    // sb.sem_op = 1;
    // semop(semd, &sb, 1);
    // while(1){
    //     clientLogic(server_socket);
    // }
}

