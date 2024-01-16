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

// void sig_handler(int signo) {
//     if (signo == SIGINT) {
//         exit_flag = 1;
//         endwin();
//         cleanup_and_exit(0); // Clean exit
//     }
//     else if (signo == SIGSEGV) {
//         fprintf(stderr, "Caught signal %d\n", signo);
//         endwin();
//         cleanup_and_exit(1); // Clean exit
//     }
// }

void universal_signal_handler(int signo) {
    fprintf(stderr, "Caught signal %d (%s)\n", signo, strsignal(signo));
    endwin();
    exit(1);
}

void display_last_five_lines() {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int line_count = 0, total_lines = 0;

    file = fopen("forum.txt", "r");
    if (file == NULL) {
        perror("Error opening forum file");
        endwin(); // Restore the terminal to its normal state

        exit(1);
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
    printw("==================FORUM==================\n");
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
        exit_flag = 1;
        endwin();
        printw("SEGMENT & SHARED MEMORY REMOVED\n");
        exit(0);
    }
}

// clientLogic() function that takes in the server_socket and a filtered status
int clientLogic(int server_socket, int filtered){

    // Prompts the user for a string.

    // gets the client PID and sends it to the server
    char pid_str[BUFFER_SIZE];
    int ch;
    int input_pos = 0;
    int pid_int = getpid();
    sprintf(pid_str, "%d", pid_int);
    write(server_socket, pid_str, sizeof(pid_str));
    char input[BUFFER_SIZE];
    char forum[BUFFER_SIZE];
    memset(input, 0, sizeof(input));
    input_pos = 0;
    char prompt[] = "Input a command (post, view, edit, delete, search, sort): ";

    prompt_and_input(prompt,input,input_pos);

    if (strcmp(input, "exit") == 0) {
        exit_flag = 1;
        endwin();
        exit(0);
    }
    // semaphore
    int semd;
    semd = semget(KEY, 1, 0);
    if(semd == -1){
        printw("error %d: %s\n", errno, strerror(errno));
        printw("Semaphore Does Not Yet Exist\n");
        endwin();
        exit(1);
    }

    // reads the forum from the server
    read(server_socket, forum, sizeof(forum));
    
    // prints the forum if the filtered status is false
    // if (filtered == 0){
    //     printw("MOST RECENT POSTS:\n===================================================\n");
    //     printw("%s===================================================\n", forum);
    //     printw("\n");
    // }
    refresh();
    // memset(input, 0, sizeof(input));
    // read(server_socket, input, sizeof(input));
    // printw("%s", input);

    // prompts the user for a command (post, view, edit, delete, search, sort) and writes it to the server
    // printw("\nInput a command (post, view, edit, delete, search, sort): ");
    // getstr(input);
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
        clrtoeol(); // Clear the line

        // prompts the user for the title of the new post
        printw("Enter the title of your post: ");
        
        getstr(input);
        strcat(input, "\n");
        printw("===================================================\n");

        // prompts the user for the content of the new post
        char content[BUFFER_SIZE];
        printw("Enter the content of your post: ");
        getstr(content);
        strcat(content, "\n");
        printw("===================================================\n");
        fflush(stdout);
        // sends the post title, post content, and client pid to the server
        write(server_socket, input, sizeof(input));
        refresh();
        write(server_socket, content, sizeof(content));
        refresh();
        write(server_socket, pid_str, sizeof(pid_str));
        refresh();
        printw("Hello!\n");
        // semop(semd, &sb, 1);
        refresh();
        memset(input, 0, sizeof(input));
        memset(content, 0, sizeof(content));
        clrtoeol(); // Clear the line
        refresh();
        fflush(stdout);

        // Check if the socket is still valid
        if (server_socket < 0) {
            printw("Socket error before sending data.\n");
            refresh();
            endwin();
            exit(1);
        }


        printw("Post sent successfully.\n");
        refresh();

        // reads and prints the new post sent from the server
        char post_content[BUFFER_SIZE*3];
        read(server_socket, post_content, sizeof(input));
        int *data;
        int shmid = shmget(KEY, sizeof(int), IPC_CREAT | 0640);
        data = shmat(shmid, 0, 0); //attach
        clear();
        printw("===================================================\nCurrent content of p%d: \n%s\n===================================================\n", *data, post_content);
        refresh();
        sb.sem_op = 1;
        semop(semd, &sb, 1);
        shmdt(data);
        sleep(2);
    }
    // check if the inputted command is [view]
    else if (strcmp(input, "view") == 0) {
        printw("===================================================\n");

        // prompts the user for which post to view and sends it to the server
        char prompt[] = "Which post would you like to view? (# only): ";
        char input[BUFFER_SIZE];
        memset(input, 0, sizeof(input));
        input_pos = 0;
        prompt_and_input(prompt,input,input_pos);
        int num;
        sscanf(input, "%d", &num);
        char post_name[BUFFER_SIZE] = "";
        sprintf(post_name, "p%d", num);
        refresh();
        write(server_socket, post_name, sizeof(post_name));
        refresh();
        // reads the post content from server and prints it
        char content[BUFFER_SIZE];
        memset(content, 0, sizeof(content));
        refresh();
        read(server_socket, content, sizeof(content));
        clear();
        printw("===================================================\nCurrent content of %s: \n===================================================\n", post_name);
        refresh();
        printw("%s\n",content);
        refresh();
        // prompts the user to either [reply] or go [back] and sends it to the server
        printw("Input a command (reply, back): ");
        getstr(input);
        printw("===================================================\n");
        // input[strcspn(input, "\n")] = '\0';  // Remove newline character
        write(server_socket, input, sizeof(input));

        // check if the inputted command is [reply]
        if (strcmp(input, "reply") == 0) {
            semop(semd, &sb, 1);
            // prompts the user to reply
            printw("Input a reply: ");
            getstr(input);
            printw("===================================================\n");
            // input[strcspn(input, "\n")] = '\0';  // Remove newline character

            // sends the reply to the server
            write(server_socket, input, sizeof(input));
            sb.sem_op = 1;
            semop(semd, &sb, 1);
        }
    }
    // check if the inputted command is [edit]
    else if(strcmp(input, "edit") == 0){
        char pid[BUFFER_SIZE];
        printw("===================================================\n");

        // promopts the user for a post to edit
        printw("Which post would you like to edit?(# only): ");
        getstr(input);
        strcat(input, "\n");
        char post_num[BUFFER_SIZE];
        strcpy(post_num, input);
        post_num[strlen(post_num)-1] = '\0';
        printw("===================================================\n");
        // sends the post number to the server
        write(server_socket, input, sizeof(input));

        // sends the client PID to the server
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));

        // reads from the server if the user has permission to edit the post
        read(server_socket, input, sizeof(input));

        // the client does have permission to edit the post
        if(strcmp(input, "YES") == 0) {
            // reads the post content from the server and prints it
            semop(semd, &sb, 1);
            char content[BUFFER_SIZE] = "";
            read(server_socket, content, sizeof(content));
            clear();
            printw("Current content of p%s: \n%s", post_num, content);
            printw("===================================================\n");
            refresh();
            // prompts the user for either [title] or [content] to edit and sends it to the server
            char choice[BUFFER_SIZE];
            char replacement[BUFFER_SIZE];
            printw("Would you like to edit the title or content of this post (title, content): ");
            refresh();
            getstr(choice);
            strcat(choice, "\n");
            printw("===================================================\n");
            write(server_socket, choice, sizeof(choice));

            // checks if the user has permission the change the title/content
            read(server_socket, input, sizeof(input));
            // the user does have permission to edit the title/content
            if(strcmp(input, "YES") == 0) {
                // prompts the user for a replacement for the title/content and sends it to the server
                printw("What would you like to replace it with: ");
                getstr(replacement);
                strcat(replacement, "\n");
                write(server_socket, replacement, sizeof(replacement));
                printw("===================================================\n");
                refresh();
                sb.sem_op = 1;
                semop(semd, &sb, 1);
            }
            // the user doesn't have permission to edit the title/content 
            else{
                read(server_socket, input, sizeof(input));
                printw("%s", input);
                refresh();
                sb.sem_op = 1;
                semop(semd, &sb, 1);
                sleep(1);
            }
        }
        // the client doesn't have permission to edit the post
        else{
            read(server_socket, input, sizeof(input));
            printw("%s", input);
            refresh();
            sleep(1);
        }
    }
    else if(strcmp(input, "delete") == 0){
        semop(semd, &sb, 1);
        char post_name[BUFFER_SIZE];
        int num;
        // make sure user has permissions
        
        // delete post file
        // delete title from forum.txt
        // down data by 1
        char pid[BUFFER_SIZE];
        printw("===================================================\n");

        // asks the user for which post they would like to delete and sends it to the server
        printw("Which post would you like to delete?(# only): ");
        getstr(input);
        strcat(input, "\n");
        printw("===================================================\n");
        write(server_socket, input, sizeof(input));

        // gets the client pid and sends it to the server
        sprintf(pid, "%d", getpid());
        write(server_socket, pid, sizeof(pid));

        // reads from the server if the user has permission to delete the post
        read(server_socket, input, sizeof(input));

        // the client does have permission to delete the file 
        if(strcmp(input, "NO") != 0) {
            printw("\t\tFILE DELETED\n===================================================\n"); 
            refresh();
            sb.sem_op = 1;
            semop(semd, &sb, 1);
            sleep(1);        
        }
        // tells the client they don't have permission delete the post
        else{
            read(server_socket, input, sizeof(input));
            printw("%s", input);
            refresh();
            sb.sem_op = 1;
            semop(semd, &sb, 1);
            sleep(1);
        }
    }    
    // check if the inputted command is [search]
    else if(strcmp(input, "search") == 0){
        // prompts the user for a keyword to send to the server
        char keyword[BUFFER_SIZE];
        printw("===================================================\n");
        printw("What keyword would you like to search: ");
        getstr(keyword);
        strcat(keyword, "\n");
        keyword[strlen(keyword)-1] = '\0';
        write(server_socket, keyword, sizeof(keyword));

        // reads the forum filtered with the keyword from the server and prints it
        char filtered[BUFFER_SIZE] = "";
        read(server_socket, filtered, sizeof(filtered));
        clear();
        printw("===================================================\nPosts Containing [%s]:\n %s\n===================================================\n", keyword, filtered);
        refresh();
        char trash[BUFFER_SIZE];
        getstr(trash);
        // returning 1 to turn filtered status true
        return 1;
    }
    // check if the inputted command is [sort]
    else if(strcmp(input, "sort") == 0){
        printw("===================================================\n");

        // asks the user for which form of sort (alphabetical, recency) for the list and sends it to the server
        printw("How would you like your post sorted?(alphabetical, recency): ");
        getstr(input);
        strcat(input, "\n");
        printw("===================================================\n");
        refresh();
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
            printw("%s SORTED: \n===================================================\n%s===================================================\n", input, content);
            refresh();
            char trash[BUFFER_SIZE];
            getstr(trash);
            return 1;
        }
        else{
            // tells the client the inputed choice is invalid
            read(server_socket, reply, sizeof(reply));
            printw("\t\tINVALID CHOICE\n===================================================\n");
            refresh();
            sleep(1);
        }
        }
    else {
        // tells the client the inputed choice is invalid
        printw("===================================================\n");
        printw("NOT A VALID COMMAND\n");
        printw("===================================================\n");
    }
    //downing semaphore
    // sb.sem_op = 1;
    // semop(semd, &sb, 1);
    printw("\n");

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
    printw("client online \n");
    for (int i = 1; i < NSIG; i++) {
        signal(i, universal_signal_handler);
    }
    signal(SIGSEGV, sighandler);  // Catch segmentation fault

    // checks for the IP of the server the client should connect to
    clear();
    char* IP = "127.0.0.1";
    initscr();
    cbreak();
    // noecho();
    keypad(stdscr, TRUE);
    signal(SIGINT, sighandler);

    if (argc > 1) {
        IP = argv[1];
    }

    // sets the forum display filtered status to false
    int filtered = 0;

    while(1){

        // connect to the server through IP
        int server_socket = client_tcp_handshake(IP);

        // // //creates shared memory
        // // int shmid = shmget(KEY, sizeof(int), 0640);
        // // int* data = shmat(shmid, 0, 0);
        
        // // char line[BUFFER_SIZE];
        // char lines[5][BUFFER_SIZE];
        // // int NUM_LINES = 5;
        // // int line_nums[NUM_LINES];
        // // char buffer[MAX_LINE_LENGTH];
        // // long filePos;
        // // int lineCount = 0, targetLine = 5;

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
        

    while (!exit_flag) {
        int server_socket = client_tcp_handshake(IP);
        display_last_five_lines();
        // sets the forum filter display status to the the previous iteration of clientLogic()
        filtered = clientLogic(server_socket, filtered);

        // clears the terminal if the filter display status is false
        if (filtered == 0){
            clear();
        }
    }
    printw("closed client\n");
    refresh();
    endwin();
    return 0;
    }
}