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

        int shmid02;
        shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        shmctl(shmid02, IPC_RMID, 0);

        printf("SEGMENT & SHARED MEMORY REMOVED\n");
        exit(0);
    }
}

// void sort_forum(int forum){
//     int alphabetical = open("alphabetical.txt",O_WRONLY | O_APPEND);
//     // Listens for a string (use the buffer size)
//     char line[BUFFER_SIZE];
//     fgets(line, sizeof(line), forum*);
//     printf("%s",line);
// }


void subserver_logic(int client_socket){
    int forum = open("forum.txt",O_WRONLY | O_APPEND);
    //Sends array of 3 most recent posts to client
    FILE* forum2 = fopen("forum.txt","r");
//     char accum[BUFFER_SIZE] = "";
//     char line[BUFFER_SIZE];
//     while (fgets(line,BUFFER_SIZE,forum2)) {
//         strcat(accum,line);
// //        strcat(accum,"---------------------------------\n");
//     }
//     accum[strlen(accum)] = '\0';
    char* accum = file_to_string("forum.txt");
    write(client_socket, accum, strlen(accum));
//    printf("Accum: %s\n",accum);

    // Gets the client's command
    char input[BUFFER_SIZE];
    read(client_socket, input, sizeof(input));

    if (strcmp(input,"post")==0) {
        read(client_socket, input, sizeof(input));
        printf("Input received: %s\n",input);

        //shared data
        int *data;
        int shmid;
        shmid = shmget(KEY, sizeof(int), 0640);
        data = shmat(shmid, 0, 0); //attach
        int i = *data + 1;
        *data = *data + 1;

        int *posts;
        int shmid02;
        shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        posts = shmat(shmid02, 0, 0);

        char new_input[BUFFER_SIZE+10];
        sprintf(new_input, "p%d: %s", i ,input);
        // printf("%ld\n",strlen(new_input));
        write(forum, new_input, strlen(new_input));
        printf("%s", new_input);
        char post_name[BUFFER_SIZE];
        sprintf(post_name, "p%d", i);
        printf("Post %s created\n", post_name);
        int post = open(post_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
        write(post, new_input, strlen(new_input));
        posts[i-1] = i;

        close(forum);
        close(post);
        shmdt(data); //detach
        shmdt(posts); //detach
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
    printf("server online\n");
    int forum = open("forum.txt",O_RDONLY);
//    printf("%s", file_to_string("forum.txt"));
    FILE* forum1 = fopen("forum.txt","r");
    int listen_socket = server_setup();
    int numStrings = 0;

//    semaphore
    int semd;
    int set;
    semd = semget(KEY, 1, IPC_EXCL | 0644 | IPC_CREAT  );
    if (semd == -1) {
        printf("errno %d: %s\n", errno, strerror(errno));
        semd = semget(KEY, 1, 0);
        set = semctl(semd, 0, GETVAL, 0);
//        printf("Semctl Returned: %d\n", set);
        exit(1);
    }
    else{
        union semun file;
        file.val = 1;
        set = semctl(semd, 0, SETVAL, file);
//        printf("Semctl Returned: %d\n", set);
    }


//    shared memory
    int *data;
    int shmid;
    shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
    data = shmat(shmid, 0, 0); //attach
//    printf("*data: %d\n", *data);
    char line[BUFFER_SIZE];
    
    int *posts;
    int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    posts = (int *)shmat(shmid02, 0, 0); //attaching

    while (fgets(line,sizeof(line),forum1)) {
        if (line[0]=='p') *data = *data + 1;
    }
//    printf("*data: %d\n", *data);
    shmdt(data); //detach
    shmdt(posts); //detach
    signal(SIGINT,sighandler);

    while(1){
        int client_socket = server_tcp_handshake(listen_socket);
        numStrings++;
        pid_t f = fork();
        if(f < 0) {
            perror("fork fail");
            exit(1);
        }
        else if (f == 0){  // child process
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
