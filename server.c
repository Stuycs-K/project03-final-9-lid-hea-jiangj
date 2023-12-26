#include "networking.h"

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
        //removing semaphore
        int semd;
        semd = semget(KEY, 1, 0);
        semctl(semd, IPC_RMID, 0);
       
        //removing shared memory
        int shmid;
        shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        shmctl(shmid, IPC_RMID, 0);

        printf("SEGMENT & SHARED MEMORY REMOVED\n");
        exit(0);
    }
}


void subserver_logic(int client_socket){
    printf("this func ran\n");
    int forum = open("forum.txt",O_WRONLY | O_APPEND);
    //Sends array of 3 most recent posts to client
    FILE* forum2 = fopen("forum.txt","r");
    char recent3[3][BUFFER_SIZE];
    fseek(forum2, 0, SEEK_END);
    int counter = 0;
    long int pos = ftell(forum2);
    /* Don't write each char on output.txt, just search for '\n' */
    char accum[BUFFER_SIZE];
    while (counter<3) {
        fseek(forum2, --pos, SEEK_SET); /* seek from begin */
        char letter = fgetc(forum2);
        if (letter == '\n') {
            printf("%c\n",letter);
            counter++;
        }
    }

    // Gets the client's command
    char input[BUFFER_SIZE];
    read(client_socket,input,sizeof(input));

    if (strcmp(input,"post")==0) {
        read(client_socket, input, sizeof(input));
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
    
    else if(strcmp(input,"reply")==0) {
        printf("Still working on this!\n");
    }
    else {
        printf("Not a valid command!\n");
    }
}


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;  
    struct seminfo *__buf;  
 };


int main(int argc, char *argv[] ) {
    int forum = open("forum.txt",O_RDONLY);
    FILE* forum1 = fopen("forum.txt","r");
    int listen_socket = server_setup();
    int numStrings = 0;


    // semaphore
    int semd;
    int set;
    semd = semget(KEY, 1, IPC_EXCL | 0644 | IPC_CREAT  );
    if (semd == -1) {
        printf("errno %d: %s\n", errno, strerror(errno));
        semd = semget(KEY, 1, 0);
        set = semctl(semd, 0, GETVAL, 0);
        printf("Semctl Returned: %d\n", set);
        exit(1);
    }
    else{
        union semun file;
        file.val = 1;
        set = semctl(semd, 0, SETVAL, file);
        printf("Semctl Returned: %d\n", set);
    }


    //shared memory
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
    fflush(stdout);

    while(1){
        printf("Child ran \n");
        int client_socket = server_tcp_handshake(listen_socket);
        printf("Child ran \n");
        numStrings++;
        printf("Child ran \n");
        pid_t f = fork();
        // if(f < 0) {
        //     perror("fork fail");
        //     exit(1);
        // }
        if (f == 0){  // child process
            printf("Child ran \n");
            subserver_logic(client_socket);
            close(client_socket);
            exit(0);
        }
        else {
            printf("%d clients connected \n", numStrings);
            close(client_socket);
        }
    }
}
