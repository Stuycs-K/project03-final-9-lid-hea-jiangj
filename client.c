#include "networking.h"
#define MAX_LINE_LENGTH 1024

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

// clientLogic() function that takes in the server_socket and a filtered status
int clientLogic(int server_socket, int filtered){

    // Prompts the user for a string.

    // gets the client PID and sends it to the server
    char pid_str[BUFFER_SIZE];
    int pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);
    write(server_socket, pid_str, sizeof(pid_str));
    
    // semaphore
    int semd;
    semd = semget(KEY, 1, 0);
    if(semd == -1){
        printf("error %d: %s\n", errno, strerror(errno));
        printf("Semaphore Does Not Yet Exist\n");
        exit(1);
    }

    // reads the forum from the server
    char input[BUFFER_SIZE];
    read(server_socket, input, sizeof(input));
    
    // prints the forum if the filtered status is false
    if (filtered == 0){
        printf("MOST RECENT POSTS:\n===================================================\n");
        printf("%s===================================================\n", input);
    }

    // prompts the user for a command (post, view, edit, delete, search, sort) and writes it to the server
    printf("Input a command (post, view, edit, delete, search, sort): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
    write(server_socket,input,sizeof(input));
    
    // semaphore
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    sb.sem_op = -1;
    // semop(semd, &sb, 1);

    // check if the inputted command is [post]
    if (strcmp(input,"post")==0) {
        semop(semd, &sb, 1);
        printf("===================================================\n");

        // prompts the user for the title of the new post
        printf("Enter the title of your post: ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");

        // prompts the user for the content of the new post
        char content[BUFFER_SIZE];
        printf("Enter the content of your post: ");
        fgets(content,sizeof(content),stdin);
        printf("===================================================\n");

        // sends the post title, post content, and client pid to the server
        write(server_socket, input, sizeof(input));
        write(server_socket, content, sizeof(content));
        write(server_socket, pid_str, sizeof(pid_str));

        // reads and prints the new post sent from the server
        char post_content[BUFFER_SIZE*3];
        read(server_socket, post_content, sizeof(input));
        int *data;
        int shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        data = shmat(shmid, 0, 0); //attach
        clear();
        printf("===================================================\nCurrent content of p%d: \n%s\n===================================================\n", *data, post_content);
        sb.sem_op = 1;
        semop(semd, &sb, 1);
        shmdt(data);
        sleep(2);
    }
    // check if the inputted command is [view]
    else if (strcmp(input, "view") == 0) {
        printf("===================================================\n");

        // prompts the user for which post to view and sends it to the server
        printf("Which post would you like to view? (# only): ");
        fgets(input, sizeof(input), stdin);
        int num;
        sscanf(input, "%d", &num);
        char post_name[BUFFER_SIZE] = "";
        sprintf(post_name, "p%d", num);
        write(server_socket, post_name, sizeof(post_name));

        // reads the post content from server and prints it
        char content[BUFFER_SIZE] = "";
        read(server_socket, content, sizeof(content));
        clear();
        printf("===================================================\nCurrent content of %s: \n%s\n===================================================\n", post_name, content);

        // prompts the user to either [reply] or go [back] and sends it to the server
        printf("Input a command (reply, back): ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");
        input[strcspn(input, "\n")] = '\0';  // Remove newline character
        write(server_socket, input, sizeof(input));

        // check if the inputted command is [reply]
        if (strcmp(input, "reply") == 0) {
            semop(semd, &sb, 1);
            // prompts the user to reply
            printf("Input a reply: ");
            fgets(input, sizeof(input), stdin);
            printf("===================================================\n");
            input[strcspn(input, "\n")] = '\0';  // Remove newline character

            // sends the reply to the server
            write(server_socket, input, sizeof(input));
            sb.sem_op = 1;
            semop(semd, &sb, 1);
        }
    }
    // check if the inputted command is [edit]
    else if(strcmp(input, "edit") == 0){
        char pid[BUFFER_SIZE];
        printf("===================================================\n");

        // promopts the user for a post to edit
        printf("Which post would you like to edit?(# only): ");
        fgets(input, sizeof(input), stdin);
        char post_num[BUFFER_SIZE];
        strcpy(post_num, input);
        post_num[strlen(post_num)-1] = '\0';
        printf("===================================================\n");
        // sends the post number to the server
        write(server_socket, input, sizeof(input));

        // sends the client PID to the server
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));

        // reads from the server if the user has permission to edit the post
        read(server_socket, input, sizeof(input));

        // the client does have permission to edit the post
        if(strcmp(input, "NO") != 0) {
            // reads the post content from the server and prints it
            semop(semd, &sb, 1);
            char content[BUFFER_SIZE] = "";
            read(server_socket, content, sizeof(content));
            printf("Current content of p%s: \n%s", post_num, content);
            printf("===================================================\n");

            // prompts the user for either [title] or [content] to edit and sends it to the server
            char choice[BUFFER_SIZE] = "";
            char replacement[BUFFER_SIZE] = "";
            printf("Would you like to edit the title or content of this post (title, content): ");
            fgets(choice,sizeof(choice),stdin);
            printf("===================================================\n");
            write(server_socket, choice, sizeof(choice));

            // checks if the user has permission the change the title/content
            read(server_socket, input, sizeof(input));
            // the user does have permission to edit the title/content
            if(strcmp(input, "NO") != 0) {
                // prompts the user for a replacement for the title/content and sends it to the server
                printf("What would you like to replace it with: ");
                fgets(replacement,sizeof(replacement),stdin);
                write(server_socket, replacement, sizeof(replacement));
                printf("===================================================\n");
                sb.sem_op = 1;
                semop(semd, &sb, 1);
            }
            // the user doesn't have permission to edit the title/content 
            else{
                read(server_socket, input, sizeof(input));
                printf("%s", input);
                sb.sem_op = 1;
                semop(semd, &sb, 1);
                sleep(1);
            }
        }
        // the client doesn't have permission to edit the post
        else{
            read(server_socket, input, sizeof(input));
            printf("%s", input);
            sleep(1);
        }
    }
    // check if the inputted command is [delete]
    else if(strcmp(input, "delete") == 0){
        semop(semd, &sb, 1);
        // make sure user has permissions
        
        // delete post file
        // delete title from forum.txt
        // down data by 1
        char pid[BUFFER_SIZE];
        printf("===================================================\n");

        // asks the user for which post they would like to delete and sends it to the server
        printf("Which post would you like to delete?(# only): ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");
        write(server_socket, input, sizeof(input));

        // gets the client pid and sends it to the server
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));

        // reads from the server if the user has permission to delete the post
        read(server_socket, input, sizeof(input));

        // the client does have permission to delete the file 
        if(strcmp(input, "NO") != 0) {
            printf("\t\tFILE DELETED\n===================================================\n"); 
            sb.sem_op = 1;
            semop(semd, &sb, 1);
            sleep(1);        
        }
        // tells the client they don't have permission delete the post
        else{
            read(server_socket, input, sizeof(input));
            printf("%s", input);
            sb.sem_op = 1;
            semop(semd, &sb, 1);
            sleep(1);
        }
    }    
    // check if the inputted command is [search]
    else if(strcmp(input, "search") == 0){
        // prompts the user for a keyword to send to the server
        char keyword[BUFFER_SIZE];
        printf("===================================================\n");
        printf("What keyword would you like to search: ");
        fgets(keyword, sizeof(keyword), stdin);
        keyword[strlen(keyword)-1] = '\0';
        write(server_socket, keyword, sizeof(keyword));

        // reads the forum filtered with the keyword from the server and prints it
        char filtered[BUFFER_SIZE] = "";
        read(server_socket, filtered, sizeof(filtered));
        clear();
        printf("===================================================\nPosts Containing [%s]:\n %s\n===================================================\n", keyword, filtered);

        // returning 1 to turn filtered status true
        return 1;
    }
    // check if the inputted command is [sort]
    else if(strcmp(input, "sort") == 0){
        printf("===================================================\n");

        // asks the user for which form of sort (alphabetical, recency) for the list and sends it to the server
        printf("How would you like your post sorted?(alphabetical, recency): ");
        fgets(input, sizeof(input), stdin);
        printf("===================================================\n");
        input[strlen(input)-1] = '\0';
        write(server_socket, input, sizeof(input));

        // reads the server to see if the sort inserted is valid
        char reply[BUFFER_SIZE] = "";
        read(server_socket, reply, sizeof(reply));
        if(strcmp(reply, "NO") != 0){
            // reads the sorted forum from the server and prints it
            char content[BUFFER_SIZE*3];
            read(server_socket, content, sizeof(content));
            clear();
            for(int i = 0; i < strlen(input); i++) input[i] = toupper(input[i]);
            printf("%s SORTED: \n===================================================\n%s===================================================\n", input, content);
            return 1;
        }
        else{
            // tells the client the inputed choice is invalid
            read(server_socket, reply, sizeof(reply));
            printf("\t\tINVALID CHOICE\n===================================================\n");
            sleep(1);
        }
    }
    else {
        // tells the client the inputed choice is invalid
        printf("===================================================\n");
        printf("NOT A VALID COMMAND\n");
        printf("===================================================\n");
    }
    //upping semaphore
    // sb.sem_op = 1;
    // semop(semd, &sb, 1);
    return 0;

}


int main(int argc, char *argv[] ) {
    printf("client online \n");

    // checks for the IP of the server the client should connect to
    clear();
    char* IP = NULL;
    if(argc>1){
        IP=argv[1];
    }

    // sets the forum display filtered status to false
    int filtered = 0;

    while(1){

        // connect to the server through IP
        int server_socket = client_tcp_handshake(IP);

        // //creates shared memory
        // int shmid = shmget(KEY, sizeof(int), 0640);
        // int* data = shmat(shmid, 0, 0);
        
        // char line[BUFFER_SIZE];
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
        // 
        
        // sets the forum filter display status to the the previous iteration of clientLogic()
        filtered = clientLogic(server_socket, filtered);

        // clears the terminal if the filter display status is false
        if (filtered == 0){
            clear();
        }
    }
}

