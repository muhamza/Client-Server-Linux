#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#define TRUE 1

struct inthread {
    int fd;
    pthread_t tid;
};

void *input(void *test){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    char buff[500];
    int pread;
    int pwrite;
    int *sock = (int *)test;
    while(TRUE){
        //write(STDOUT_FILENO,">> ",3);
        pread = read(0,buff,500);
        if(pread == -1){
            perror("Client Read from User - Error");
        }
        buff[pread-1] = '\0';
        pwrite = write(*sock,buff,pread);
        if(pwrite == -1){
            perror("Client Write on Socket - Error");
        }
    }
}

void *output(void *test){
    char buff[1500];
    int pread;
    int pwrite;
    struct inthread *in;
        in = (struct inthread*)test;
    int fd = (int)in->fd;
    pthread_t tid = in->tid;
    free(in);

    while(TRUE){
        pread = read(fd,buff,1500);
        if(pread == -1){
            perror("Client Read from Socket - Error");
        }
        else if (pread == 0){
            exit(0);
        }
        if(strcmp(buff,"Exit") == 0){ 
            pwrite = write(STDOUT_FILENO,"Client Exiting\n",15);
            if(pwrite == -1){
                perror("Client Print on Screen - Error");
            }
            exit(0);
        }
        else if(strcmp(buff,"Disconnect") == 0){ 
            pwrite = write(STDOUT_FILENO,"Client Disconnecting!\n",27);
            if(pwrite == -1){
                perror("Client Print on Screen - Error");
            }
            pthread_cancel(tid);
            pthread_exit(NULL);
        }
        else{
            pwrite = write(STDOUT_FILENO,buff,pread);
            if(pwrite == -1){
                perror("Client Print on Screen - Error");
            }
            write(STDOUT_FILENO,"\n",1);
        }
    }
}

int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in server;
    struct hostent *hp;
    int ctest;
    char conn[30];
    char *tok;
    char *ip;
    char *port;
    int readrc;
    while(TRUE){
        ctest = 0;
        while(ctest == 0){
            write(STDOUT_FILENO,"Enter Connection Command: ",26);
            readrc = read(STDIN_FILENO,conn,30);
            if(readrc == -1){
                perror("Connect Read - Error");
            }
            else{
                tok = strtok(conn," ");
                if(strcmp(tok,"connect") != 0){
                    write(STDOUT_FILENO,"Invalid Command\n",16);
                }
                else{
                    ip = strtok(NULL," ");
                    port = strtok(NULL," ");
                    sock = socket(AF_INET, SOCK_STREAM, 0);
                    if (sock < 0) {
                        perror("Opening Stream Socket - Error");
                        continue;
                    }
                    server.sin_family = AF_INET;
                    hp = gethostbyname(ip);
                    if (hp == 0) {
                        write(STDOUT_FILENO,"Invalid IP\n",11);
                        continue;
                    }
                    bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
                    server.sin_port = htons(atoi(port));

                    if (connect(sock,(struct sockaddr *) &server,sizeof(server)) == 0) {
                        ctest = 1;
                    }
                    else{
                        perror("Connecting Stream Socket - Error");
                        continue;
                    }
                }
            }
        }
        pthread_t thread_in,  thread_out;
        write(STDOUT_FILENO,"Client Connected to Server\n",27);
        pthread_create(&thread_in,NULL,input,(void*)&sock);
        struct inthread *in = (struct inthread *)malloc(sizeof(struct inthread));
        in->fd = sock;
        in->tid = thread_in;
        pthread_create(&thread_out,NULL,output,(void*)in);

        pthread_join(thread_in, NULL);
        pthread_join(thread_out, NULL);
        close(sock);
    }    
}