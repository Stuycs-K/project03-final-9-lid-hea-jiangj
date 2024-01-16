# Final Project Proposal

## Group Members:
Alvin He;
Jack Jiang;
Derek Li
       
# Intentions:
[A statement of the problem you are solving and/or a high level description of the project.]

A reddit-like question forum where users can post questions, reply to questions, reply to replies and look at previous posts

    
# Intended usage:
[A description as to how the project will be used (describe the user interface).]
  
Once the user runs the program, the terminal will display all the posts that have already been entered. 

The user will then be prompted for an input where they may...
1. choose to reorganize the posts based on either replies or recency
2. search for specific keywords within the posts that have already been posted
    - once the keyword is entered, the terminal will only display posts with the keyword in its question or replies
3. insert the post ID of the post they want to view to access it's replies
    - once the post ID is chosen, the user can view all of the terminal will display all the information of the post including replies
        - the user will be prompted to enter a reply, choose a reply to reply to, or go exit the post
        - if the post was posted by the user, then the user will also have the option to edit or remove the post
4. create a question to post




# Technical Details:

A description of your technical design. This should include:
   
How you will be using the topics covered in class in the project.

How you are breaking down the project and who is responsible for which parts.
  
What data structures you will be using and how.
     
What algorithms and /or data structures you will be using, and how.

1. we will utilize sockets and pipes to allow for networking across our forum from multiple locations via server and client functions
2. we will utilize programs to manage files that contain the contents and replies of each post
3. we will utilize semaphores to prevent issues when multiple users attempt to modify the same posts or shared memory
4. we will utilize programs in <sys/stat.h> to sort the files that contain the content of each post
5. we will utilize signals to allow the user to perform certain actions like closing, deleting, creating, etc.
6. we will utilize processes like forking to create a child process when modifying content in the forum on the server side

Project Breakdown:

1. Displaying the forum in the terminal - Alvin, Jack, Derek
2. Implementing posts + replies - Alvin, Jack, Derek
3. Sort Posts - Derek
4. Delete/Edit posts - Jack
5. Search Posts - Alvin


# Intended pacing:

A timeline with expected completion dates of parts of the project.

Jan 8 - Users are able to post questions and replies. Terminal will display all posts/replies.

Jan 15 - Users will be able to delete/edit posts and replies.
Users will be able to sort posts based on recency and most replies. Users will be able to search posts based on a specific keyword in the post or replies.


Jan 16 - Project's done.