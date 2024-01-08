#include "networking.h"

void clientLogic(int server_socket){
//    while(1){
    // Prompts the user for a string.
    char input[BUFFER_SIZE];
    read(server_socket, input, sizeof(input));
    printf("%s", input);
        
    printf("Input a command (post, view, edit): ");
    fgets(input, sizeof(input), stdin);
    *strchr(input, '\n') = 0;
    // printf("About to write\n");
    write(server_socket,input,sizeof(input));
    // printf("If statement about to run\n");
    if (strcmp(input,"post")==0) {
        char content[BUFFER_SIZE];
        printf("Enter the title of your post: ");
        fgets(input, sizeof(input), stdin);
        printf("Enter the content of your post: ");
        fgets(content,sizeof(content),stdin);
        // printf("fgets: %s\n",input);
        // Send the user input to the client.
        write(server_socket, input, sizeof(input));
        write(server_socket, content, sizeof(content));
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
        sprintf(post_name, "p%d", num);
        int post = open(post_name, O_RDONLY, 0);
        char* content = file_to_string(post_name);
        printf("Current content of %s: \n%s", post_name, content);
        close(post);
        printf("Would you like to edit the title or content of the post: ");
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
            printf("Replacment1: %s",replacement1);


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

    int *posts;
    int shmid02;
    shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
    posts = shmat(shmid02, 0, 0);
    printf("posts: %d\n",*posts);

    clientLogic(server_socket);


    //downing semaphore
    // sb.sem_op = 1;
    // semop(semd, &sb, 1);
    // while(1){
    //     clientLogic(server_socket);
    // }
}

