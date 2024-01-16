[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/SQs7pKlr)
# PROJECT NAME HERE: 
Feddit

### GROUP NAME HERE
[Group Members Here] 

Alvin He, Jack Jiang, Derek Li
       
### Project Description:
[Explain what is this project.] 
  
A reddit-like forum where users can create, edit, view, delete, search, and sort posts. The forum will have colored text and users will be able to scroll through the posts with the up and down key. 

### Libraries:
    - ncurses

### Instructions:

How does the user install/compile/run the program.

Installation:
The user MUST install ncurses on their computer/laptop. 
If you are on windows install via:

    - sudo apt-get install libncurses5-dev libncursesw5-dev (in Linux)

If you are on mac install via:

    - brew install ncurses

Compile:
To compile run make compile. To clean run make clean. 

Run:
The user must first run server by typing ./server in one terminal. In another terminal the user should run ./client. The user will interact with the forum through the terminal with ./client. The user will be prompted for commands such as post, edit, view, etc. Once done, the user will exit the client first with CTRL C (do this when the program asks for a command), then exit the server with CTRL C

How does the user interact with this program?
The user can ask questions and express opinions by creating posts with the "post" command. If they want to reply to other posts and even reply to their own they can do so with the "reply" command. They have full ownership over their posts as ONLY they can delete or edit them. Of course, users can view each other's posts and their replies. This forum serves as a mini real-time community connecting forum. The user can run all these commands until he wants to exit with CTRL+C when it asks for a command.