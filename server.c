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


void search_file(const char* filename, char* string) {
    int file = open(filename, O_RDONLY, 0666);
    char buff[BUFFER_SIZE] = "";
    char new_string[BUFFER_SIZE] = "";
    int byte;
    while((byte = read(file, buff, BUFFER_SIZE))) {
        if (strstr(buff, string) != NULL){
            strcat(new_string, buff);
        }
    }
    new_string[strlen(new_string)] = '\0';
    close(file);
}

void subserver_logic(int client_socket){
    char clientPID[BUFFER_SIZE+12];
    read(client_socket, clientPID, sizeof(clientPID));
    printf("clientPID: %s\n", clientPID);
    int forum = open("forum.txt", O_WRONLY | O_APPEND, 0666);
    FILE* forum2 = fopen("forum.txt","r");
    char accum[BUFFER_SIZE] = "";
    file_to_string("forum.txt", accum);
    write(client_socket, accum, strlen(accum));
//    printf("%s", accum);
    fflush(stdin);

    // Gets the client's command
    char input[BUFFER_SIZE];
    read(client_socket, input, sizeof(input));

    if (strcmp(input,"post")==0) {
        char content[BUFFER_SIZE];
        char pid_str[BUFFER_SIZE];
        int pid_int;
        read(client_socket, input, sizeof(input));
        read(client_socket, content, sizeof(content));
        read(client_socket, pid_str, sizeof(pid_str));
        sscanf(pid_str, "%d", &pid_int);
        printf("Input received: %s\n",input);

        //shared data
        int *data;
        int shmid;
        shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        data = shmat(shmid, 0, 0); //attach
        printf("Data: %d\n",*data);
        int i = *data + 1;
        *data = *data + 1;

        int *posts;
        int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        posts = (int *)shmat(shmid02, 0, 0);

        char new_input[BUFFER_SIZE+10];
        sprintf(new_input, "p%d: %s", i ,input);
        // printf("%ld\n",strlen(new_input));
        write(forum, new_input, strlen(new_input));
        printf("New_input: %s", new_input);
        char post_name[BUFFER_SIZE];
        sprintf(post_name, "p%d", i);
        printf("Post %s created\n", post_name);
        int post = open(post_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
        char post_creator[BUFFER_SIZE*3];
        sprintf(post_creator, "[by user%s]\n", clientPID);
        write(post, post_creator, strlen(post_creator));
        write(post, new_input, strlen(new_input));
        char post_content[BUFFER_SIZE*3];
        sprintf(post_content, "Content: %s\n", content);
        write(post, post_content, strlen(post_content));
        posts[i-1] = pid_int;

        // sends back the updated forum

        close(forum);
        close(post);
        shmdt(data); //detach
        shmdt(posts); //detach
    } 
    else if(strcmp(input, "view") == 0){
        read(client_socket, input, sizeof(input));
        if (strlen(input) <= 3){
            char* post_name = input;
            int post = open(post_name, O_WRONLY | O_APPEND, 0666);
            char post_content[BUFFER_SIZE];
            file_to_string(post_name, post_content);
            write(client_socket, post_content, strlen(post_content));
            read(client_socket, input, sizeof(input));
            if (strcmp(input, "reply") == 0){
                char reply[BUFFER_SIZE*3];
                read(client_socket, input, sizeof(input));
                sprintf(reply, "\t[user%s] %s\n", clientPID, input);
                write(post, reply, strlen(reply));
            }
            else if (strcmp(input, "back") == 0){

            }
            else {
                char invalid[BUFFER_SIZE] = "Invalid Command";
                write(client_socket, invalid, sizeof(invalid));
            }
        }
        else{
            char invalid[BUFFER_SIZE] = "Invalid Post";
            write(client_socket, invalid, sizeof(invalid));
        }
    }
    else if (strcmp(input, "edit")==0) {
        read(client_socket, input, sizeof(input));
        char post_name[BUFFER_SIZE];
        int num;
        sscanf(input, "%d", &num);
        printf("%d\n", num);

        //shared memory
        int *posts;
        int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        posts = (int *)shmat(shmid02, 0, 0);

        //permission
        char pid_str[BUFFER_SIZE];
        int pid_int;
        read(client_socket, pid_str, sizeof(pid_str));
        sscanf(pid_str, "%d", &pid_int);
        if(posts[num-1] != pid_int) {
            char answer[BUFFER_SIZE] = "NO";
            write(client_socket, answer, sizeof(answer));
            char reply[BUFFER_SIZE] = "You do not have permission to edit this post!\n";
            write(client_socket, reply, sizeof(reply));
        }
        else{
            char answer[BUFFER_SIZE] = "YES";
            write(client_socket, answer, sizeof(answer));
            sprintf(post_name, "p%d", num);
            int post = open(post_name, O_RDONLY, 0);
            char content[BUFFER_SIZE] = "";
            file_to_string(post_name, content);
            printf("Current content of %s: \n%s", post_name, content);
            close(post);

            char choice[BUFFER_SIZE];
            char replacement[BUFFER_SIZE];
            read(client_socket, choice, sizeof(choice));
            read(client_socket, replacement, sizeof(replacement));

            FILE *file, *tempFile;
            char buffer[BUFFER_SIZE];
            int lineToReplace = num; // The line number to replace
            char *newLine = replacement; // The new line content
            char replacement1[BUFFER_SIZE+10];
            sprintf(replacement1,"p%d: %s",num,replacement);
            char *newLine1 = replacement1;
            int currentLine = 1;

            if (strcmp(choice,"title\n")==0) {
                file = fopen("forum.txt", "r");
                tempFile = fopen("temp.txt", "w");

                if (file == NULL || tempFile == NULL) {
                    perror("Error opening file!\n");
                }

                // Read from the original file and write to the temporary file
                while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
                    // If the current line is the line to replace, write the new line to the temp file
                    if (currentLine == lineToReplace) {
                        fputs(newLine1, tempFile);
                    } else {
                        // Otherwise, write the original line
                        fputs(buffer, tempFile);
                    }
                    currentLine++;
                }

                // Close the files
                fclose(file);
                fclose(tempFile);

                // Delete the original file and rename the temporary file to the original file name
                remove("forum.txt");
                rename("temp.txt", "forum.txt");
            }
        
            else if (strcmp(choice,"content\n")==0) {
                FILE * pFile = fopen(post_name, "r");
                tempFile = fopen("temp.txt", "w");

                memset(buffer,0,sizeof(buffer));
                memset(replacement1,0,sizeof(replacement1));
                sprintf(replacement1,"p%d: %s",num,replacement);
                printf("Replacment: %s",replacement1);

                currentLine = 1;

                while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
                    // If the current line is the line to replace, write the new line to the temp file
                    if (currentLine == 1) {
                        printf("This ran!\n");
                        fputs(replacement1, tempFile);
                    } 
                    else {
                        // Otherwise, write the original line
                        fputs(buffer, tempFile);
                    }
                    currentLine++;
                }

                fclose(pFile);
                fclose(tempFile);

                // Delete the original file and rename the temporary file to the original file name
                remove(post_name);
                rename("temp.txt", post_name);
            }
            else {
                printf("Not a valid command!\n");
            }
        }
    }
    // else if(strcmp(input,"delete")==0) {
    //     read(client_socket, input, sizeof(input));
    //     char post_name[BUFFER_SIZE];
    //     int num;
    //     sscanf(input, "%d", &num);
    //     printf("%d\n", num);

    //     //shared memory
    //     int *posts;
    //     int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    //     posts = (int *)shmat(shmid02, 0, 0);

    //     //permission
    //     char pid_str[BUFFER_SIZE];
    //     int pid_int;
    //     read(client_socket, pid_str, sizeof(pid_str));
    //     sscanf(pid_str, "%d", &pid_int);
    //     if(posts[num-1] != pid_int) {
    //         char answer[BUFFER_SIZE] = "NO";
    //         write(client_socket, answer, sizeof(answer));
    //         char reply[BUFFER_SIZE] = "You do not have permission to edit this post!\n";
    //         write(client_socket, reply, sizeof(reply));
    //     }
    //     else{
    //         char answer[BUFFER_SIZE] = "YES";
    //         write(client_socket, answer, sizeof(answer));
    //         sprintf(post_name, "p%d", num);
    //         int post = open(post_name, O_RDONLY, 0);


            

    //         FILE *file, *tempFile;
    //         char buffer[BUFFER_SIZE];
    //         int lineToReplace = num; // The line number to replace
    //         char *newLine = replacement; // The new line content
    //         char replacement1[BUFFER_SIZE+10];
    //         sprintf(replacement1,"p%d: %s",num,replacement);
    //         char *newLine1 = replacement1;
    //         int currentLine = 1;

    //         if (strcmp(choice,"title\n")==0) {
    //             file = fopen("forum.txt", "r");
    //             tempFile = fopen("temp.txt", "w");

    //             if (file == NULL || tempFile == NULL) {
    //                 perror("Error opening file!\n");
    //             }

    //             // Read from the original file and write to the temporary file
    //             while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
    //                 // If the current line is the line to replace, write the new line to the temp file
    //                 if (currentLine == lineToReplace) {
    //                     fputs(newLine1, tempFile);
    //                 } else {
    //                     // Otherwise, write the original line
    //                     fputs(buffer, tempFile);
    //                 }
    //                 currentLine++;
    //             }

    //             // Close the files
    //             fclose(file);
    //             fclose(tempFile);

    //             // Delete the original file and rename the temporary file to the original file name
    //             remove("forum.txt");
    //             rename("temp.txt", "forum.txt");
    //         }
        
    //         else if (strcmp(choice,"content\n")==0) {
    //             FILE * pFile = fopen(post_name, "r");
    //             tempFile = fopen("temp.txt", "w");

    //             memset(buffer,0,sizeof(buffer));
    //             memset(replacement1,0,sizeof(replacement1));
    //             sprintf(replacement1,"p%d: %s",num,replacement);
    //             printf("Replacment: %s",replacement1);

    //             currentLine = 1;

    //             while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
    //                 // If the current line is the line to replace, write the new line to the temp file
    //                 if (currentLine == 1) {
    //                     printf("This ran!\n");
    //                     fputs(replacement1, tempFile);
    //                 } 
    //                 else {
    //                     // Otherwise, write the original line
    //                     fputs(buffer, tempFile);
    //                 }
    //                 currentLine++;
    //             }

    //             fclose(pFile);
    //             fclose(tempFile);

    //             // Delete the original file and rename the temporary file to the original file name
    //             remove(post_name);
    //             rename("temp.txt", post_name);
    //         }
    //         else {
    //             printf("Not a valid command!\n");
    //         }
    //     }
    // }
    else {
        printf("Not a valid command!\n");
    }
}

// union semun {
//     int val;
//     struct semid_ds *buf;
//     unsigned short *array;  
//     struct seminfo *__buf;  
//  };

int main(int argc, char *argv[] ) {
    printf("SERVER ONLINE\n===================================================\n");
    // int forum = open("forum.txt",O_RDONLY);
    // // printf("%s", file_to_string("forum.txt"));
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
        // printf("Semctl Returned: %d\n", set);
        exit(1);
    }
    else{
        union semun file;
        file.val = 1;
        set = semctl(semd, 0, SETVAL, file);
        // printf("Semctl Returned: %d\n", set);
    }


    // shared memory
    int *data;
    int shmid;
    shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
    data = shmat(shmid, 0, 0); //attach
    // printf("*data: %d\n", *data);
    char line[BUFFER_SIZE];
    
    int *posts;
    int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    posts = (int *)shmat(shmid02, 0, 0); //attaching

    while (fgets(line,sizeof(line),forum1)) {
        if (line[0]=='p') *data = *data + 1;
    }
    printf("*data: %d\n", *data);
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
            printf("%d Clients Connected \n", numStrings);
            close(client_socket);
        }
    }
}
