# Client-Server-Linux
The client and server processes are developed using system apis. The client requests the server to perform some operations, which are then performed on the server and the results are returned. 

**Follow these steps to compile and run the Client and server processes.**   
1. To compile the server process type this command in the terminal:   
$~ gcc server.c -o server  

2. To compile the client process type this command in another terminal window:  
$~ gcc client.c -o client -lpthread  

3. To run the server process, type:  
./server  

4. To run the client process, type:  
./client  

**Note:** Read the architecture details for a better understanding of the two process and how they work. The valid commands are listed below.

# Valid Client and Server Commands

**Client Commands:**  

1. add: can be used to ADD numbers. Example: add 1 2 3 4 5  
2. sub: can be used to SUBTRACT numbers. Example: sub 1 2 3 4 5      
3. mul: can be used to MULTIPLY numbers. Example: mul 1 2 3 4 5    
4. div: can be used to DIVIDE numbers. Example div: 100 10 2  
5. run: you can OPEN programs using this command. Example: run gedit  
6. list: lists down all programs that are active. Example: list  
7. list all: lists down all programs that were opened using the run command (active and inactive). Example: list all  
8. help: displays the help text on the screen.  
9. kill pid: kills process with specified pid. Example: kill pid 25365  
10. kill name: kills process with specified name. Example: kill name gedit  
11. kill all: kills all processes that are active. Example: kill all  
12. connect: connect command takes in the IP address and the Port number of the server and form a connection with the 
server if the addresses are correct. Example: connect 172.14.250.144 46525  
13. disconnect: disconnects the connection of the client with the server. Example: disconnect  
14. exit: exits the program. Example: exit  


**Server Commands:**  

1. list-connections: lists down the IP addresses and the Port numbers of all clients that have been connected to the 
server. Example: list-connections  
2. list-processes: lists down all processes that are running on the server as well as those that had been running. The
server parent process sends the request to all server-child process, which in return provide the server with their 
current process lists. Those lists are then printed onto the screen by the server parent. Example: list-processes  
3. list-processes IP: lists down all active and nonactive processes belonging to a client with the specified IP. 
Example: list-processes 172.14.250.152  


