#include "networking.h"

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

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;  
    struct seminfo *__buf;  
 };

// a function to search the forum file and return all lines containing the given string 
void search_file(const char* filename, char* keyword, char* filtered_return) {
    FILE *file = fopen("forum.txt", "r");
    char line[BUFFER_SIZE] = "";
    char filtered[BUFFER_SIZE] = "";
    int byte;
    while (fgets(line, BUFFER_SIZE, file) != NULL) {
//        printf("line: %s [%s]", line, strstr(line, keyword));
        if (strstr(line, keyword) != NULL){ // searches if the line has a reference to the keyword
            strcat(filtered, line);
        }
    }
    filtered[strlen(filtered)] = '\0';
//    printf("%s", filtered);
    fclose(file);
    strcpy(filtered_return, filtered);
}


void subserver_logic(int client_socket){

    // gets the connected client's PID to identify the user
    char clientPID[BUFFER_SIZE+12];
    read(client_socket, clientPID, sizeof(clientPID));
    printf("clientPID: %s\n", clientPID);

    // opens the forum and sends it to the client to display
    int forum = open("forum.txt", O_WRONLY | O_APPEND, 0666);
    FILE* forum2 = fopen("forum.txt","r");
    char accum[BUFFER_SIZE] = "";
    file_to_string("forum.txt", accum);
    write(client_socket, accum, strlen(accum));

    
    // gets the client's intial command (post, view, edit, delete, search, sort)
    char input[BUFFER_SIZE];
    read(client_socket, input, sizeof(input));

    if (strcmp(input,"post")==0) {
        char content[BUFFER_SIZE];
        char pid_str[BUFFER_SIZE];
        int pid_int;
        
        // gets the post title from the client
        read(client_socket, input, sizeof(input));
        // gets the post content from the client
        read(client_socket, content, sizeof(content));
        // gets the pid of the connected client and turns it from a string to an int
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

        // creates the post in the format [p#: TITLE], writes it in the forum, and prints it
        char new_input[BUFFER_SIZE+10];
        sprintf(new_input, "p%d: %s", i ,input);
        write(forum, new_input, strlen(new_input));
        printf("New_input: %s", new_input);
        
        char post_name[BUFFER_SIZE];
        sprintf(post_name, "p%d", i);
        printf("Post %s created\n", post_name);
        int post = open(post_name, O_WRONLY | O_APPEND | O_CREAT, 0666);
        char post_creator[BUFFER_SIZE*3];
        sprintf(post_creator, "[by user%s]\n", clientPID);
        write(post, post_creator, strlen(post_creator));
        // write(post, new_input, strlen(new_input));
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
                // file_to_string(post_name, post_content);
                // write(client_socket, post_content, strlen(post_content));
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
            char reply[BUFFER_SIZE] = "===================================================\n\t\tPERMISSION DENIED\n===================================================\n";
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
            sprintf(replacement1,"p%d: %s", num, replacement);
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
                sprintf(replacement1,"content: %s", replacement);
                printf("Replacment: %s",replacement1);

                currentLine = 1;

                while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
                    // If the current line is the line to replace, write the new line to the temp file
                    if (currentLine == 2) {
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
    else if(strcmp(input,"delete")==0) {
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
            char reply[BUFFER_SIZE] = "You do not have permission to delete this post!\n";
            write(client_socket, reply, sizeof(reply));
        }
        else{
            char answer[BUFFER_SIZE] = "YES";
            write(client_socket, answer, sizeof(answer));
            sprintf(post_name, "p%d", num);
            //Deleting post file
            if (remove(post_name) == 0)
                printf("Deleted successfully");
            else
                printf("Unable to delete the file");
            
            //Removing post from forum.txt
            FILE * pFile = fopen("forum.txt", "r");
            FILE * tempFile = fopen("temp.txt", "w");

            char buffer[BUFFER_SIZE];
            memset(buffer,0,sizeof(buffer));

            int *data;
            int shmid;
            shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
            data = shmat(shmid, 0, 0); //attach
            *data = *data - 1;
            shmdt(data); //detach


            int currentLine = 1;

            while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
                // If the current line is the line to replace, write the new line to the temp file
                if (currentLine == num) {

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
            remove("forum.txt");
            rename("temp.txt", "forum.txt");
        }
    }
    else if(strcmp(input, "search") == 0){
        char keyword[BUFFER_SIZE];
        read(client_socket, keyword, sizeof(keyword));
        keyword[strlen(keyword)] = '\0';
        printf("keyword:[%s] length:[%lu]", keyword, strlen(keyword));
        char filtered[BUFFER_SIZE] = ""; 
        search_file("forum.txt", keyword, filtered);
        printf("filtered: %s strlen: %lu", filtered, strlen(filtered));
        
        write(client_socket, filtered, strlen(filtered));
    }
    else if(strcmp(input, "sort") == 0){
        read(client_socket, input, sizeof(input));
        input[strlen(input)-1] = '\0';
        printf("%s\n", input);
        if(strcmp(input, "alphabetical") == 0){
            int *data;
            int shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
            data = shmat(shmid, 0, 0); //attach
            FILE * posts = fopen("forum.txt", "r");
            int byte = open(input, O_WRONLY | O_APPEND | O_CREAT, 0666);
            char list[*data][BUFFER_SIZE];
            char line1[BUFFER_SIZE];
            int lines = 0;
            while(fgets(line1, sizeof(line1), posts)){
                char line2[BUFFER_SIZE] = "";
                int start = 0;
                int end = 0;
                while(line1[start] != ' ') start++;
                while(line1[end] != '\n') end++;
                start++;
                end++;
                printf("start: %d end: %d\n", start, end);
                for(int i = start; i < end; i++) line2[i-start] = line1[i];
                // list[lines] = line2;
                strcpy(list[lines++], line2);
                // printf("%s", list[lines++]);
                // write(byte, list[lines++], end-start);
            }
            char temp[BUFFER_SIZE];
            for(int i = 0; i < *data; i++){
                for(int j = i+1; j < *data; j++){
                    if(strcmp(list[i], list[j]) > 0){
                        strcpy(temp, list[i]);
                        strcpy(list[i], list[j]);
                        strcpy(list[j], temp);
                    }
                }
            }
            // for(int i = 0; i < *data; i++){
            //     printf("%s", list[i]);
            // }
            for(int i = 0; i < *data; i++){
                write(byte, list[i], strlen(list[i]));
            }
            // write(byte, list[1], sizeof(list[1]));
            shmdt(data); //detach
            pclose(posts);
            close(byte);
        }
    }
    else {
        printf("Not a valid command!\n");
    }
    close(forum);


}

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;  
    struct seminfo *__buf;  
 };

int main(int argc, char *argv[] ) {
    printf("SERVER ONLINE\n===================================================\n");

    FILE* forum1 = fopen("forum.txt","r");
    int listen_socket = server_setup();
    int numClients = 0;

    // semaphore
    int semd;
    int set;
    semd = semget(KEY, 1, IPC_EXCL | 0666 | IPC_CREAT  );
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
        // waits waits for client connection
        int client_socket = server_tcp_handshake(listen_socket);
        numClients++;

        // forks a separate process to interact with a client
        pid_t f = fork();
        if(f < 0) {
            perror("fork fail");
            exit(1);
        }
        else if (f == 0){ 
            // subserver deals with the client stuff
            subserver_logic(client_socket);
            close(client_socket);
            exit(0);
        }
        else {
            // server prints the number of clients connected and waits for subserver process to finish
            printf("%d Clients Connected \n", numClients);
            int status;
            waitpid(f,&status,0);
            close(client_socket);
        }
    }
}
