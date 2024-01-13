#include "networking.h"
#define MAX_LINE_LENGTH 1024

// int filtered = 0;

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
int clientLogic(int server_socket, int filtered){
//    printf("clientLogic reached\n");
//    while(1){
    // Prompts the user for a string.
    char pid_str[BUFFER_SIZE];
    int pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);
    write(server_socket, pid_str, sizeof(pid_str));
    
    int semd;
    semd = semget(KEY, 1, 0);
    if(semd == -1){
        printf("error %d: %s\n", errno, strerror(errno));
        printf("Semaphore Does Not Yet Exist\n");
        exit(1);
    }

    char input[BUFFER_SIZE];
    read(server_socket, input, sizeof(input));
    if (filtered == 0){
        printf("MOST RECENT POSTS:\n===================================================\n");
        printf("%s===================================================\n", input);
    }

    printf("Input a command (post, view, edit, delete, search, sort): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
//    printf("About to write\n");
    write(server_socket,input,sizeof(input));
    // printf("If statement about to run\n");
    // uping semaphore
//    printf("Connecting to Server... This may take a moment.\n");
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    sb.sem_op = -1;
    semop(semd, &sb, 1);
//    printf("Checking command\n");
    if (strcmp(input,"post")==0) {
        char content[BUFFER_SIZE];
        // char pid_str[BUFFER_SIZE];
        // int pid_int = getpid();
        // printf("pid: %d\n", pid_int);
        // sprintf(pid_str, "%d", pid_int);
        printf("===================================================\n");
        printf("Enter the title of your post: ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");
        printf("Enter the content of your post: ");
        fgets(content,sizeof(content),stdin);
        printf("===================================================\n");
        // printf("fgets: %s\n",input);
        // Send the user input to the client.
        write(server_socket, input, sizeof(input));
        write(server_socket, content, sizeof(content));
        write(server_socket, pid_str, sizeof(pid_str));
    // Read the modified string from the server
        char post_content[BUFFER_SIZE*3];
        read(server_socket, post_content, sizeof(input));
        int *data;
        int shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        data = shmat(shmid, 0, 0); //attach
        clear();
        printf("===================================================\nCurrent content of p%d: \n%s\n===================================================\n", *data, post_content);
        sleep(3);
        shmdt(data);

    // Prints the modified string
    //        printf("%s", input);
    }
    else if (strcmp(input, "view") == 0) {
        printf("===================================================\n");
        printf("Which post would you like to view? (# only): ");
        fgets(input, sizeof(input), stdin);
        int num;
        sscanf(input, "%d", &num);
        char post_name[BUFFER_SIZE] = "";
        sprintf(post_name, "p%d", num);
        write(server_socket, post_name, sizeof(post_name));

        char content[BUFFER_SIZE] = "";
        read(server_socket, content, sizeof(content));
        clear();
        printf("===================================================\nCurrent content of %s: \n%s\n===================================================\n", post_name, content);

        // Prompt for reply
        printf("Input a command (reply, back): ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");
        input[strcspn(input, "\n")] = '\0';  // Remove newline character
        write(server_socket, input, sizeof(input));

        if (strcmp(input, "reply") == 0) {
            printf("Input a reply: ");
            fgets(input, sizeof(input), stdin);
            printf("===================================================\n");
            input[strcspn(input, "\n")] = '\0';  // Remove newline character
            write(server_socket, input, sizeof(input));
            // read(server_socket, input, sizeof(input));
            // printf("===================================================\nCurrent content of %s: \n%s\n===================================================\n", post_name, content);
        }
    }
    else if(strcmp(input, "edit") == 0){
        char pid[BUFFER_SIZE];
        printf("===================================================\n");
        printf("Which post would you like to edit?(# only): ");
        fgets(input, sizeof(input), stdin);
        char post_num[BUFFER_SIZE];
        strcpy(post_num, input);
        post_num[strlen(post_num)-1] = '\0';
        printf("===================================================\n");
        write(server_socket, input, sizeof(input));
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));
        read(server_socket, input, sizeof(input));
        if(strcmp(input, "NO") != 0) {
            char content[BUFFER_SIZE] = "";
            read(server_socket, content, sizeof(content));
            printf("Current content of p%s: \n%s", post_num, content);
            printf("===================================================\n");
            char choice[BUFFER_SIZE] = "";
            char replacement[BUFFER_SIZE] = "";
            printf("Would you like to edit the title or content of this post (title, content): ");
            fgets(choice,sizeof(choice),stdin);
            printf("===================================================\n");
            write(server_socket, choice, sizeof(choice));

            read(server_socket, input, sizeof(input));
            if(strcmp(input, "NO") != 0) {
                printf("What would you like to replace it with: ");
                fgets(replacement,sizeof(replacement),stdin);
                write(server_socket, replacement, sizeof(replacement));
                printf("===================================================\n");
            }
            else{
                read(server_socket, input, sizeof(input));
                printf("%s", input);
                sleep(1);
            }
        }
        else{
            read(server_socket, input, sizeof(input));
            printf("%s", input);
            sleep(1);
        }
    }
    else if(strcmp(input, "delete") == 0){
        //make sure user has permissions
        
        //delete post file
        //delete title from forum.txt
        //down data by 1
        char pid[BUFFER_SIZE];
        printf("Which post would you like to delete?(# only): ");
        fgets(input, sizeof(input), stdin);
        write(server_socket, input, sizeof(input));
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));
        read(server_socket, input, sizeof(input));
        if(strcmp(input, "NO") != 0) {
            printf("File deleted\n");            
        }
        else{
            read(server_socket, input, sizeof(input));
            printf("%s", input);
        }
    }    
    else if(strcmp(input, "search") == 0){
        char keyword[BUFFER_SIZE];
        printf("what keyword would you like to search: ");
        fgets(keyword, sizeof(keyword), stdin);
        keyword[strlen(keyword)-1] = '\0';
        write(server_socket, keyword, sizeof(keyword));
        char filtered[BUFFER_SIZE] = "";
        read(server_socket, filtered, sizeof(filtered));
        printf("===================================================\nPosts Containing [%s]:\n %s\n===================================================\n", keyword, filtered);

//        printf("results with [%s]: \n%s\n", keyword, filtered);
        sb.sem_op = 1;
        semop(semd, &sb, 1);
//        printf("\n");
        return 1;
    }
    else if(strcmp(input, "sort") == 0){
        printf("how would you like your post sorted: ");
        fgets(input, sizeof(input), stdin);
        input[strlen(input)-1] = '\0';
        write(server_socket, input, sizeof(input));
        char content[BUFFER_SIZE*3];
        read(server_socket, content, sizeof(content));
        clear();
        for(int i = 0; i < strlen(input); i++) input[i] = toupper(input[i]);
        printf("%s SORTED: \n===================================================\n%s===================================================\n", input, content);
        sb.sem_op = 1;
        semop(semd, &sb, 1);
        return 1;
    }
    else {
        printf("Not a valid command!\n");
    }
    //upping semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);
//    printf("\n");
    return 0;

}


int main(int argc, char *argv[] ) {
//    printf("client online \n");
    // checks for the IP of the server the client should connect to
    clear();
    char* IP = NULL;
    if(argc>1){
        IP=argv[1];
    }
    int filtered = 0;
    while(1){
        char line[BUFFER_SIZE];
        // connect to the server through IP
        int server_socket = client_tcp_handshake(IP);

        // //creates shared memory
        // int shmid = shmget(KEY, sizeof(int), 0640);
        // int* data = shmat(shmid, 0, 0);

        // char lines[5][BUFFER_SIZE];
        // int NUM_LINES = 5;
        // int line_nums[NUM_LINES];
        // char buffer[MAX_LINE_LENGTH];
        // long filePos;
        // int lineCount = 0, targetLine = 5;

        // opens file
        // FILE* forum1 = fopen("forum.txt","r");
        // if (forum1 == NULL) {
        //     perror("Error opening file");
        //     return 1;
        // }

        // // Seek to the end of the file
        // fseek(forum1, 0, SEEK_END);
        // filePos = ftell(forum1);

        // // Move backwards through the file to find the 5th last newline
        // while (lineCount < targetLine && filePos >= 0) {
        //     fseek(forum1, --filePos, SEEK_SET);
        //     if (fgetc(forum1) == '\n') {
        //         lineCount++;
        //     }
        // }

        // // Read and print the last 5 lines
        // if (lineCount < targetLine) {
        //     // The file has less than 5 lines, so go to the start
        //     fseek(forum1, 0, SEEK_SET);
        // } else {
        //     // Go to the start of the line
        //     fseek(forum1, filePos + 1, SEEK_SET);
        // }
        // for (int i = 0;i<lineCount;i++) {
        //     if (fgets(lines[i], MAX_LINE_LENGTH, forum1) != NULL) {
        //         printf("%s",lines[i]);
        //     }
        // }
        // printf("===================================================\n");
        // fclose(forum1);
        
        filtered = clientLogic(server_socket, filtered);
        if (filtered == 0){
            clear();
        }
    }
}

