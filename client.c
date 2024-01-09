#include "networking.h"
#define MAX_LINE_LENGTH 1024

void clientLogic(int server_socket){
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

    printf("Input a command (post, view, edit): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
    // printf("About to write\n");
    write(server_socket,input,sizeof(input));
    // printf("If statement about to run\n");
    // uping semaphore
//    printf("Connecting to Server... This may take a moment.\n");
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_flg = SEM_UNDO;
    sb.sem_op = -1;
    semop(semd, &sb, 1);
    if (strcmp(input,"post")==0) {
        char content[BUFFER_SIZE];
        // char pid_str[BUFFER_SIZE];
        // int pid_int = getpid();
        printf("pid: %d\n", pid_int);
        // sprintf(pid_str, "%d", pid_int);
        printf("Enter the title of your post: ");
        fgets(input, sizeof(input), stdin);
        printf("Enter the content of your post: ");
        fgets(content,sizeof(content),stdin);
        // printf("fgets: %s\n",input);
        // Send the user input to the client.
        write(server_socket, input, sizeof(input));
        write(server_socket, content, sizeof(content));
        write(server_socket, pid_str, sizeof(pid_str));
    // Read the modified string from the server
    // read(server_socket, input, sizeof(input));


    // Prints the modified string
    //        printf("%s", input);
    }
    else if (strcmp(input, "view") == 0) {
        printf("Which post would you like to view? (# only): ");
        fgets(input, sizeof(input), stdin);
        int num;
        sscanf(input, "%d", &num);
        char post_name[BUFFER_SIZE];
        sprintf(post_name, "p%d", num);
        write(server_socket, post_name, sizeof(post_name));

        char content[BUFFER_SIZE];
        read(server_socket, content, sizeof(content));
        printf("Current content of %s: \n%s\n", post_name, content);

        // Prompt for reply
        printf("Input a command (reply, back): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';  // Remove newline character
        write(server_socket, input, sizeof(input));

        if (strcmp(input, "reply") == 0) {
            printf("Input a reply: ");
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';  // Remove newline character
            write(server_socket, input, sizeof(input));
        }

    }
    else if(strcmp(input, "edit") == 0){
        printf("Which post would you like to edit?(# only): ");
        fgets(input, sizeof(input), stdin);
        char post_name[BUFFER_SIZE];
        int num;
        sscanf(input, "%d", &num);

        int *posts;
        int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        posts = (int *)shmat(shmid02, 0, 0);

        if(posts[num-1] != getpid()) {
            printf("You do not have permission to edit this post!\n");
        }
        else{
        sprintf(post_name, "p%d", num);
        int post = open(post_name, O_RDONLY, 0);
        char* content = file_to_string(post_name);
        printf("Current content of %s: \n%s", post_name, content);
        close(post);
        printf("Would you like to edit the title or content of this post (title, content): ");
        char choice[BUFFER_SIZE];
        fgets(choice,sizeof(choice),stdin);
        printf("What would you like to replace it with: ");
        char replacement[BUFFER_SIZE];
        fgets(replacement,sizeof(replacement),stdin);

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
                } else {
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
    else {
        printf("Not a valid command!\n");
    }
    //downing semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);
    //    }
}


int main(int argc, char *argv[] ) {
    keypad(stdstr, TRUE);
    int ch = getch();
    if (ch == KEY_UP) {
        printf("You pressed up!\n");
    }
    else if(ch == KEY_DOWN) {
        printf("You pressed down!\n");
    }
    char* IP = NULL;
    if(argc>1){
        IP=argv[1];
    }

    while(1){
        //displaying the forum
        char line[BUFFER_SIZE];
        int server_socket = client_tcp_handshake(IP);
        int shmid = shmget(KEY, sizeof(int), 0640);
        int* data = shmat(shmid, 0, 0); //attach

        char lines[5][BUFFER_SIZE];
        int NUM_LINES = 5;
        int line_nums[NUM_LINES];
        char buffer[MAX_LINE_LENGTH];
        long filePos;
        int lineCount = 0, targetLine = 5;

        FILE* forum1 = fopen("forum.txt","r");
        if (forum1 == NULL) {
            perror("Error opening file");
            return 1;
        }

        // Seek to the end of the file
        fseek(forum1, 0, SEEK_END);
        filePos = ftell(forum1);

        // Move backwards through the file to find the 5th last newline
        while (lineCount < targetLine && filePos >= 0) {
            fseek(forum1, --filePos, SEEK_SET);
            if (fgetc(forum1) == '\n') {
                lineCount++;
            }
        }

        // Read and print the last 5 lines
        if (lineCount < targetLine) {
            // The file has less than 5 lines, so go to the start
            fseek(forum1, 0, SEEK_SET);
        } else {
            // Go to the start of the line
            fseek(forum1, filePos + 1, SEEK_SET);
        }

        for (int i = 0;i<lineCount;i++) {
            if (fgets(lines[i], MAX_LINE_LENGTH, forum1) != NULL) {
                printf("%s",lines[i]);
            }
        }
        fclose(forum1);

        // int *posts;
        // int shmid02;
        // shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        // posts = shmat(shmid02, 0, 0);
        // printf("posts: %d\n",*posts);
        
            clientLogic(server_socket);
    // printf("MOST RECENT POSTS:\n===================================================\n");
    // for (int i = 0;i<lineCount;i++) {
    //     if (fgets(lines[i], MAX_LINE_LENGTH, forum1) != NULL) {
    //         printf("%s",lines[i]);
    //     }
    // }
    // printf("===================================================\n");
    // fclose(forum1);
    // clientLogic(server_socket);
    }
}

