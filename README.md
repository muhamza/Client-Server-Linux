# Client-Server-Linux
The client and server processes are developed using system apis. The client requests the server to perform some operations, which are then performed on the server and the results are returned. 

Follow these steps to compile and run the Client and server processes.   
1. To compile the server process type this command in the terminal:   
$~ gcc server.c -o server  

2. To compile the client process type this command in another terminal window:  
$~ gcc client.c -o client -lpthread  

3. To run the server process, type:  
./server  

4. To run the client process, type:  
./client  

**Note:** For help with commands please read the Commands section in the Architecture file.

