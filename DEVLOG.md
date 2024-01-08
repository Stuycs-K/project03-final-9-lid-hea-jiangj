# Dev Log:

## Player 1: Alvin He

### 2024-01-02 - Brief description
CW/HW: found group members and started thinking of ideas

### 202s4-01-03 - Brief description
CW: brainstormed ideas (battleship, monopoly, reddit)and checked with Mr. K
HW: worked on proposal file with the idea of a reddit-like question forum

### 2024-01-04 - Brief description
CW: discussed logistics of terminal displaying all the posts. also discussed how to keep updating clients with all the most recent posts
HW: made the base server, client, and makefile. established basic communication between client and server. server can now take input from client and place into a file.

### 2024-01-05 - Brief description
CW: sorted out server and client communication stuff. started working on forum reading and sorting alphabetically. 
HW: cleaned up code and got server to send post menu to the client instantly, started working on replies, made a file_to_string method for faster communication from server to client

### 2024-01-08 - Brief description
CW: moved view and post protocols into server, clients only have ask and get a string back nothing inbetween

## Player 2: Derek Li

### 2024-01-02 - Brief description
CW/HW: found group members and brainstormed some ideas to propose

### 2024-01-03 - Brief description
CW: created project repo and brainstormed ideas to propose (decided on reddit-like forum; approval from Mr. K)
HW: worked on technical details in proposal file and discussed work division and intended timeline with group

### 2024-01-04 - Brief description
CW: decided on logistics of the program. discussed whether to use a single large file for data storage or multiple smaller files for each post made (still ongoing). decided on server-client based program using sockets for networking and implemented during class.
HW: established semaphore access. creates shared memory and semaphore on the server side. ups and downs the semaphore on client side to prevent mutliple users from modifying the forum file at the same time. modified sighandler to remove semaphore after kill signal based on a recognizable key.

### 2024-01-05 - Brief description
CW: discussed with group the logistics of how each post and their content would be sorted. decided on creating a seperate text file for each post and their content. started working on shared memory of array that will contain the file descriptors for the posts
HW: continued working on shared memory of int array. successfully created and populated (needs more testing). began working on opening files for each post and implementing an edit feature that would allow client users to edit the content inside each post

### 2024-01-7 - Brief description
WORK: worked on functions that allows the client user to read and edit the individual post files that they create. succesfully got edit and read to function but encountered an issue regarding the file not terminating at null and displaying old data (looking into). planning on implementing user permission using shared memory once debugging is completed.

## Player 3: Jack Jiang

### 2024-01-02 - Brief description
CW/HW: found group members and thought/discussed of project ideas 

### 2024-01-03 - Brief description
CW: brainstormed/discussed project ideas like monopoly, battleship, and eventually decided on reddit. Asked questions/approval from Mr. K
HW: worked on timemline and project breakdown in PROPOSAL.md

### 2024-01-04 Brief Description
CW: brainstormed logistics of project. Decided on multiple file system, where each post + its replies would be its own file. Also decided to do a client-server system, where client can only access its own post (file). All displaying of the forum will be in the client, the server will handle processing data. Added networking.c file from lab 16 as reference. Added empty server.c and client.c files. Made own branch and linked with remote branch.

HW: Worked on adding posts to forum.txt so that each post would have p(n) where n is a number based on the number of that post. So the first post in forum.txt would have p1, the second would have p2 and so on. I used shared memory to store the number of questions in forum.txt by first looping through forum.txt and saving it, and then using that memory to decide what number to put before a post. I used signals to delete the shared memory once you exited the server/session. Now anything the user types in is prefaced by p<post #>. 

### 2024-01-7 - Brief description
HW: Editing feature now allows users to replace specific posts based on number inputted. It replaces the post in forum.txt and the post file created. I did this creating a temp.txt file and renaming it. I will try to work on displaying the forum posts to the client to make it look better.


### 2024-01-8 - Brief description
CW: Changed editing feature so that it you can edit the title and content of a post separately. Forum asks user if they want to change title or content.