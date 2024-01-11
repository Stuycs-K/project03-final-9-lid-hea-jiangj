#include "networking.h"
#define MAX_LINE_LENGTH 1024
#define LINES_PER_PAGE 5
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#include <pthread.h> 

volatile sig_atomic_t exit_flag = 0;
int scroll_position = -1; // Global variable for tracking scroll position

void cleanup_and_exit(int exit_code) {
    endwin(); // Restore the terminal to its normal state
    exit(exit_code);
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        exit_flag = 1;
        cleanup_and_exit(0); // Clean exit
    }
    else if (signo == SIGSEGV) {
        fprintf(stderr, "Caught signal %d\n", signo);
        cleanup_and_exit(1); // Clean exit
    }
}

void universal_signal_handler(int signo) {
    fprintf(stderr, "Caught signal %d (%s)\n", signo, strsignal(signo));
    exit(1);
}

void display_last_five_lines() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int line_count = 0, total_lines = 0;

    file = fopen("forum.txt", "r");
    if (file == NULL) {
        perror("Error opening forum file");
        return;
    }

    // Count total number of lines if scroll_position is not set
    if (scroll_position == -1) {
        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
            total_lines++;
        }
        scroll_position = MAX(0, total_lines - LINES_PER_PAGE);
        rewind(file);
    } else {
        // Re-count total lines each time to account for any new lines added
        while (fgets(line, MAX_LINE_LENGTH, file) != NULL) {
            total_lines++;
        }
        rewind(file);

        // Ensure scroll_position does not exceed the limit
        if (scroll_position > total_lines - LINES_PER_PAGE) {
            scroll_position = MAX(0, total_lines - LINES_PER_PAGE);
        }
    }

    clear();

    // Skip lines up to the current scroll position
    while (line_count < scroll_position && fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        line_count++;
    }

    // Read and display the next LINES_PER_PAGE lines
    int displayed_lines = 0;
    while (displayed_lines < LINES_PER_PAGE && fgets(line, MAX_LINE_LENGTH, file) != NULL) {
        printw("%s", line);
        displayed_lines++;
    }

    refresh();
    fclose(file);
}

void prompt_and_input(char prompt[], char input[], int input_pos) {
    int prompt_len = strlen(prompt);
    printw("%s",prompt);
    int prompt_end_pos = getcurx(stdscr); // Store the cursor position at the end of the prompt
    refresh();

    int ch;
    while ((ch = getch()) != '\n') {
        if (ch == KEY_UP || ch == KEY_DOWN) {
            // Handling scrolling
            if (ch == KEY_UP) scroll_position = MAX(0, scroll_position - 1);
            if (ch == KEY_DOWN) scroll_position++; // Add bounds check if needed
            display_last_five_lines();
            printw("%s%s", prompt, input); // Redraw the prompt and input
            move(getcury(stdscr), prompt_end_pos + input_pos); // Correctly position the cursor
            refresh();
        }
        else if ((ch == KEY_BACKSPACE || ch == 127) && input_pos > 0) {
            // Handling backspace
            input_pos--;
            input[input_pos] = '\0';
            // Clear and redraw the line
            // move(getcury(stdscr), 0); // Move cursor to the start of the current line
            clrtoeol(); // Clear the line
            printw("\b"); // Redraw the prompt and the input
            move(getcury(stdscr), prompt_end_pos + input_pos); // Correctly position the cursor
            refresh();
        }
        else if (input_pos < BUFFER_SIZE - 1 && isprint(ch)) {
            // Handling character input
            input[input_pos] = ch;
            input_pos++;
            // // Redraw the input
            // move(getcury(stdscr), 0); // Move cursor to the start of the current line
            // clrtoeol(); // Clear the line
            printw("%c", input[input_pos]); // Redraw the prompt and the input
            // move(getcury(stdscr), prompt_end_pos + input_pos); // Correctly position the cursor
            refresh();
        }
    }
    input[input_pos] = '\0'; // Null-terminate the input string
}

void clientLogic(int server_socket){
    char pid_str[BUFFER_SIZE];
    int ch;
    int input_pos = 0;
    int pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);
    write(server_socket, pid_str, sizeof(pid_str));

    char input[BUFFER_SIZE];
    memset(input, 0, sizeof(input));
    input_pos = 0;
    char prompt[] = "Input a command (post, view, edit, exit): ";

    prompt_and_input(prompt,input,input_pos);

    if (strcmp(input, "exit") == 0) {
        exit_flag = 1;
        return;
    }
    int semd;
    semd = semget(KEY, 1, 0);
    if(semd == -1){
        printw("error %d: %s\n", errno, strerror(errno));
        printw("Semaphore Does Not Yet Exist\n");
        exit(1);
    }

    // memset(input, 0, sizeof(input));
    // read(server_socket, input, sizeof(input));
    // printw("%s", input);

    
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
    if (strcmp(input, "post") == 0) {
        memset(input, 0, sizeof(input));
        char content[BUFFER_SIZE];
        memset(content, 0, sizeof(content));
        clrtoeol(); // Clear the line
        printw("Enter the title of your post: ");
        refresh();
        getstr(input);
        strcat(input, "\n");
        printw("input: %s",input);
        refresh();
        printw("Enter the content of your post: ");
        refresh();
        getstr(content);
        strcat(content, "\n");
        printw("content: %s",content);
        refresh();
        fflush(stdout);

        // Check if the socket is still valid
        if (server_socket < 0) {
            printw("Socket error before sending data.\n");
            refresh();
            return;
        }

        // Attempt to write the data
        write(server_socket, input, sizeof(input)); 
        refresh();
        write(server_socket, content, sizeof(content));
        refresh();

        write(server_socket, pid_str, sizeof(pid_str));
        refresh();

        printw("Post sent successfully.\n");
        refresh();
    }
    // Read the modified string from the server
    // read(server_socket, input, sizeof(input));


    // Prints the modified string
    //        printf("%s", input);
    
    else if (strcmp(input, "view") == 0) {
        char prompt[] = "Which post would you like to view? (# only): ";
        char input[BUFFER_SIZE];
        memset(input, 0, sizeof(input));
        input_pos = 0;
        prompt_and_input(prompt,input,input_pos);
        int num;
        sscanf(input, "%d", &num);
        char post_name[BUFFER_SIZE];
        sprintf(post_name, "p%d", num);
        write(server_socket, post_name, sizeof(post_name));

        char content[BUFFER_SIZE];
        memset(content, 0, sizeof(content));
        read(server_socket, content, sizeof(content));
        printw("content: %s",content);
        refresh();
        fflush(stdout);
        printw("Current content of %s: \n", post_name);
        refresh();
        printw("%s\n",content);
        refresh();
        // Prompt for reply
        printw("Input a command (reply, back): ");
        getstr(input);
        // input[strcspn(input, "\n")] = '\0';  // Remove newline character
        write(server_socket, input, sizeof(input));

        if (strcmp(input, "reply") == 0) {
            printw("Input a reply: ");
            getstr(input);
            // input[strcspn(input, "\n")] = '\0';  // Remove newline character
            write(server_socket, input, sizeof(input));
        }

    }
    else if(strcmp(input, "edit") == 0){
        printw("Which post would you like to edit?(# only): ");
        getstr(input);
        char post_name[BUFFER_SIZE];
        int num;
        sscanf(input, "%d", &num);

        int *posts;
        int shmid02 = shmget(KEY02, MAX_FILES*sizeof(int), IPC_CREAT | 0640);
        posts = (int *)shmat(shmid02, 0, 0);

        if(posts[num-1] != getpid()) {
            printw("You do not have permission to edit this post!\n");
        }
        else{
            sprintf(post_name, "p%d", num);
            int post = open(post_name, O_RDONLY, 0);
            char* content;
            file_to_string(post_name,content);
            printw("Current content of %s: \n%s", post_name, content);
            close(post);
            printw("Would you like to edit the title or content of this post (title, content): ");
            char choice[BUFFER_SIZE];
            getstr(choice);
            printw("What would you like to replace it with: ");
            char replacement[BUFFER_SIZE];
            getstr(replacement);

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
                printw("Replacment: %s",replacement1);


                currentLine = 1;

                while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
                    // If the current line is the line to replace, write the new line to the temp file
                    if (currentLine == 1) {
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
                printw("Not a valid command!\n");
            }
            }
        }
    else if(strcmp(input, "delete") == 0){
        char post_name[BUFFER_SIZE];
        int num;
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
            sprintf(post_name, "p%d", num);
            int post = open(post_name, O_RDONLY, 0);
            char* content;
            file_to_string(post_name,content);
            printw("Current content of %s: \n%s", post_name, content);
            close(post);
            printw("Would you like to edit the title or content of this post (title, content): ");
            char choice[BUFFER_SIZE];
            getstr(choice);
            printw("What would you like to replace it with: ");
            char replacement[BUFFER_SIZE];
            getstr(replacement);

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
                printw("Replacment: %s",replacement1);


                currentLine = 1;

                while (fgets(buffer, BUFFER_SIZE, pFile) != NULL) {
                    // If the current line is the line to replace, write the new line to the temp file
                    if (currentLine == 1) {
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
                printw("Not a valid command!\n");
            }
        }
        }
    else {
        printw("Not a valid command!\n");
    }
    //downing semaphore
    sb.sem_op = 1;
    semop(semd, &sb, 1);
    printf("\n");

}


int current_position = 0;

// Key listener thread function
void* key_listener(void* p) {
    int ch;
    while (1) {
        ch = getch();
        if(ch == KEY_UP) {
            current_position = MAX(0, current_position - 1);
        } else if(ch == KEY_DOWN) {
            current_position++;
        }
        // Trigger display update here if needed
    }
    return NULL;
}

//outdated code for displaying last 5 lines
// FILE* forum1 = fopen("forum.txt","r");
//         if (forum1 == NULL) {
//             perror("Error opening file");
//             return 1;
//         }

//         // Seek to the end of the file
//         fseek(forum1, 0, SEEK_END);
//         filePos = ftell(forum1);

//         // Move backwards through the file to find the 5th last newline
//         while (lineCount < targetLine && filePos >= 0) {
//             fseek(forum1, --filePos, SEEK_SET);
//             if (fgetc(forum1) == '\n') {
//                 lineCount++;
//             }
//         }

//         // Read and print the last 5 lines
//         if (lineCount < targetLine) {
//             // The file has less than 5 lines, so go to the start
//             fseek(forum1, 0, SEEK_SET);
//         } else {
//             // Go to the start of the line
//             fseek(forum1, filePos + 1, SEEK_SET);
//         }

//         for (int i = 0;i<lineCount;i++) {
//             if (fgets(lines[i], MAX_LINE_LENGTH, forum1) != NULL) {
//                 printf("%s",lines[i]);
//             }
//         }
//         fclose(forum1);


int main(int argc, char *argv[] ) {
    for (int i = 1; i < NSIG; i++) {
        signal(i, universal_signal_handler);
    }
    signal(SIGSEGV, sig_handler);  // Catch segmentation fault

    // checks for the IP of the server the client should connect to
    char* IP = NULL;
    initscr();
    cbreak();
    // noecho();
    keypad(stdscr, TRUE);
    signal(SIGINT, sig_handler);

    if (argc > 1) {
        IP = argv[1];
    }
        // char line[BUFFER_SIZE];
        // // connect to the server through IP
        // int server_socket = client_tcp_handshake(IP);

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

        // printf("MOST RECENT POSTS:\n===================================================\n");
        // for (int i = 0;i<lineCount;i++) {
        //     if (fgets(lines[i], MAX_LINE_LENGTH, forum1) != NULL) {
        //         printf("%s",lines[i]);
        //     }
        // }
        // printf("===================================================\n");
        // fclose(forum1);
        

    while (!exit_flag) {
        int server_socket = client_tcp_handshake(IP);
        display_last_five_lines();
        clientLogic(server_socket);
    }
    printw("closed client\n");
    refresh();
    endwin();
    return 0;
}

