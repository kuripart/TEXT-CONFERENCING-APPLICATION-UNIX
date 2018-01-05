PROJECT: CLIENT-SERVER TEXT CONFERENCING<br />
DATE OF SUBMISSION: 30/Nov/2017<br />
AUTHORS: Partha Sarathi Kuri, Nithin Ganesh Prasad<br />

PROJECT OBJECTIVES:
--------------------
- This project allows a user to simulate text conferencing, by allowing them to connect a number of clients to a single server, and setting up conference sessions between the clients.

- The core functions revolve around authentication (login), conference handling (create conference sessions, join sessions, text messages to other users in the same session, and leave sessions), and exiting the server (without causing disconnect-issues for the server). 
NOTE: this is real-time messaging.

- The additional functionality added to this program was to be able to conference with multiple users over multiple sessions simultaneously (ie. user can be involved in 2 or more conference sessions at the same time). 


KEY FUNCTIONALITY:
-------------------
- Login: this project allows you to login to a server through multiple clients (successful logins/failed logins are acknowledge by the server).
- Create: able to create conference sessions capable of holding a number of users.
- Join: able to multiple conference sessions.
- List: able to list sessions and list client sessions.
- Leave: able to leave a conference session at any time.
- Query: automatically done every time a client connects.
- Exit: able to exit from server at any point (without affecting server operation).

EXTRA FUNCTIONALITY/FEATURES:
For the bonus segment, we added the ability for the user to join multiple conference sessions! Below are the functionality associated with this specific feature!
- Swap: allows the user to swap conference sessions.
- Curr: prints the current conference session.


HOW TO RUN:
---------------

First, open up a terminal and run 'gcc -o server server.c' followed by ./server. Then open up to 5 other terminals (1 for each client, given that the maximum number of clients is set to 5) and run 'gcc -o client client.c'. Run each client by typing ./client <username> <password>, where the username and password are valid if they exist as a key-value pair in client_database.txt. NOTE: all files should be in the same directory when running this command (and should have navigated to that directory in each terminal).

Next, the server is waiting for commands from each client. 

The first command from a client is to create a conference session (eg. /create c1). Once a conference session is created, any client can type a command (eg. /join c1) to join that session.
Note, that multiple sessions of different names can be created (by the same or different sessions) and multiple sessions can be 'joined' by the same session.

When a conference joins a session, they immediately set that as their current session. You can type a message (eg. /current) to display the current session that the client is in. In addition, a client can swap to any other sessions they are in by issuing another command (eg. /swap c2). In order to determine what sessions you can join, you can list sessions (eg. /listsessions) to list all the sessions in the current program, as well as list all the clients in each session (eg. 
/listclients c1).

In addition, upon joining a session, the user can type any message into the corresponding client's terminal, and this will be relayed to all client terminals who are in the same session (even if that session is not another client's 'active' session (RECAP: a session can be made 'active' by typing /swap session_name).

If you want to leave a session at any time (for any client), type a corresponding message in that client terminal (eg. /leave c1). If you want to terminate a client connection, you can type to logout (eg. logout) or exit (eg. /exit); the difference is that exiting removes the client from the client_list while the logging out means that the client can log back in from any terminal again (and it will retain its conference sessions etc). However, it will lose all the messages exchanged in the session between the logout and the re-login (to be improved on at a future date).


PROGRAM STRUCTURE:
-------------------
The 2 key files used were server.c and client.c.

- server.c: contained all the functions required to host a server (a program looping infinitely that waits for incoming connections). It acts as a medium to transfer messages between clients in multiple conferences. It also performs all actions relating to conference management (creating conferences, assigning clients to conferences, etc). The key function in server.c was handle_connection() which took any incoming message, parsed the message, determined whether the message was a 'command' or a simple text between users, and acted accordingly. NOTE: the client-server TCP connection was done for multiple clients using 'SELECT' which was implemented in 
main(). Also note that though we listened for new connections in main(), we accepted the said connection in handle_connection(). NEW ADDITIONS: We added the ability to list sessions and list client sessions.

- client.c connected to the server, and accepted user data and sent this data to the server (server.c). It also waits indefinitely for user input, as the user can send multiple commands/messages to interact with the server/other clients. This program terminates upon receiving 
'/exit'.

Additional Key Files that were used in supporting the above files/programs:
- declarations.h - contained major constants/data-structures used in the programs.
- client_database.txt - contained a fixed set of usernames and passwords used by users to authenticate into the program.
- conference_handler.h - contained all the major functions for handling various commands and performing conference actions.
- communication_handler.h - responsible for sending/receiving data to peers.
- message_handler.h - contained all the major functions for handling various networking calls.

NOTE: in all the client files (eg. a.txt, b.txt ...) and session files (eg. c1.txt, c2.txt ...), names of sessions/clients respectively were separated by ':', and the last field was also terminated by a ':'.


PROGRAM LIMITATIONS/NEXT-STEPS:
--------------------------------

While substantial functionality was implemented for the program in the time given, it falls short of a durable, real-time, messaging system for many ways (and can thus be improved in the future). Here are some shortcomings, and some potential improvements:
- Lack of GUI => many functions implemented as ('/' commands) could be streamlined via 'buttons' if a GUI were to be used.
- Single terminal => having access to multiple terminals, would mean that it would be possible to create a unique terminal per conference session!
- Encryption => allow additional security by implementing end-to-end encryption between users for additional security in data/text transfer.

REFERENCES:
---------------
- Beej's Guide to Networking
- Course Notes/Textbook
- Stackoverflow


THANK YOU :)







