#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <poll.h>
#define TRUE 1

//-----------------------------------Interfaces--------------------------------------------------------

void add(char *str, int fd);
void sub(char *str, int fd);
void mult(char *str, int fd);
void divi(char *str, int fd);
void run(char *str, int fd);
void showlist(int fd);
void showlistall(int fd);
void help();
void killprocess(char *str, int fd);
void killall();
void print(int result, int nr, int fd);
void printstring(char *buff, int size, int fd);
void changeStatus(int pid);
void sigint_handler(int signo);
void sigint_handler2(int signo);
void endconnection(int pid);
void listconnections();
void listprocesses();
void listprocesses2(char *ip);

//-----------------------------------Struct Definition------------------------------------------------

struct list{
        int pid;
        char name[20];
        int isActive;
    time_t start;
    time_t end;
};

struct connection{
    unsigned short port;     
        char ip[15]; 
    int fsend;
    int frec;
    int pid;
};

//-----------------------------------Arrays Creation--------------------------------------------------

static struct connection conns[30];
static int countconnection = 0;
static struct list runlist[30];
static int countlist = 0;

//-----------------------------------Main Method------------------------------------------------------

int main(void) {
    int sock, length, length2;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int csock;
    char buf[1024];
    int rval;
    int i;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }
    /* Name socket using wildcards */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 0;
    if (bind(sock, (struct sockaddr *) &server, sizeof(server))) {
        perror("binding stream socket");
        exit(1);
    }
    /* Find out assigned port number and print it out */
    length = sizeof(server);
    if (getsockname(sock, (struct sockaddr *) &server, (socklen_t*) &length)) {
        perror("getting socket name");
        exit(1);
    }
    char server_port[25];
    int sp = sprintf(server_port,"Socket has port #%d\n", ntohs(server.sin_port));
    int pwr = write(STDOUT_FILENO,server_port,sp);
    if(pwr == -1){
        perror("Write Socket Port - Error");
    }

    /* Start accepting connections */
    listen(sock, 5);
    length2 = sizeof(client);
    char ipad[15];
    int poad;

    char *arr[10];
    int key = 0;
    char buffer[30];
    char *command;
    char *ip;

    //int pollret;
    struct pollfd fds[2];

    if (signal(SIGCHLD,sigint_handler2) == SIG_ERR){
        fprintf(stderr, "Cannot handle SIGCHLD!\n");
    }
    do {
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = sock;
        fds[1].events = POLLIN;
        poll(fds, 2, -1);
        //if(pollret == -1){
        //    perror("Main Server Poll - Error");
        //}
        if (fds[0].revents & POLLIN) { //STDIN_FILENO Contains Data
            int retread = read(STDIN_FILENO,buffer,30);
            if(retread == -1){
                perror("Read 1st Poll - Error");
            }
            buffer[retread -1] = '\0';

            command = strtok(buffer," ");
            ip = strtok(NULL," ");

            if(strcmp(command,"list-connections") == 0){
                listconnections();
            }
            else if(strcmp(command,"list-processes") == 0 && ip == NULL){
                listprocesses();
            }
            else if(strcmp(command,"list-processes") == 0 && ip != NULL){
                listprocesses2(ip);
            }
            else{
                write(1,"Invalid Command!\n",17);
            }
        }
        if (fds[1].revents & POLLIN) {
            csock = accept(sock, (struct sockaddr *)&client,&length2);
            strcpy(ipad,inet_ntoa(client.sin_addr));
            poad = ntohs(client.sin_port);

            if (csock == -1)
                perror("accept");
            else {
                strcpy(conns[countconnection].ip,ipad); 
                conns[countconnection].port = poad;
            
                int fsend[2];
                int frec[2];
                pipe(fsend);
                pipe(frec);
                conns[countconnection].fsend = fsend[1];
                conns[countconnection].frec = frec[0];
                //countconnection++;            

                int pid = fork(); //fork
                if(pid == -1){
                    perror("Server-Child Fork - Error");
                }
                else if(pid > 0){
                    conns[countconnection].pid = pid;
                    countconnection++;
                }
                else if(pid == 0){
                    char buff[500];
                    char *tok;
                    char *string;
                    int check = 1;
                    int writeret;
                    int killret;
                    struct pollfd pfds[2]; 
                    //int pollret2;

                    while(check == 1){
                        pfds[0].fd = csock;
                        pfds[0].events = POLLIN;
                        pfds[1].fd = fsend[0];
                        pfds[1].events = POLLIN;
                        poll(pfds, 2, -1);
                        //if(pollret2 == -1){
                        //    perror("Server Child Poll - Error");
                        //}
                        if (pfds[0].revents & POLLIN) {
                            int pread = read(csock,buff,500);
                            if(pread == -1){
                                perror("Server Read");
                                exit(1);
                            }
                            key=0;
                            tok = strtok(buff,";");
                            while(tok != NULL){
                                if(key <10){
                                    arr[key++] = tok;
                                    tok = strtok(NULL, ";");
                                }
                                else{
                                    break;
                                }
                            }
                            for(int i=0; i<key; i++){
                                string = arr[i];
                                tok = strtok(string," ");
                                if(strcmp(tok,"add") == 0){
                                    add(NULL,csock);
                                }
                                else if(strcmp(tok,"sub") == 0){
                                    sub(NULL,csock);
                                }
                                else if(strcmp(tok,"mul") == 0){
                                    mult(NULL,csock);
                                }
                                else if(strcmp(tok,"div") == 0){
                                    divi(NULL,csock);
                                }
                                else if(strcmp(tok,"run") == 0){
                                    run(NULL,csock);
                                }
                                else if(strcmp(tok,"help") == 0){
                                    help(csock);
                                }
                                else if(strcmp(tok,"list") == 0){
                                    tok = strtok(NULL," ");
                                    if(tok == NULL){
                                        showlist(csock);
                                    }
                                    else if(tok != NULL && strcmp(tok,"all") == 0){
                                        showlistall(csock);
                                    }
                                    else{
                                        write(csock,"Invalid List Command!\n",22);
                                    }
                                }
                                else if(strcmp(tok,"kill") == 0){
                                    killprocess(NULL, csock);
                                }
                                else if(strcmp(tok,"exit") == 0){
                                    killall();
                                    writeret = write(csock,"Exit\0",5);
                                    if(writeret == -1){
                                        perror("CServer Write on Socket - Error");
                                    }
                                    close(csock);
                                    exit(EXIT_SUCCESS);
                                }
                                else if(strcmp(tok,"disconnect") == 0){
                                    killall();
                                    writeret = write(csock,"Disconnect\0",11);
                                    if(writeret == -1){
                                        perror("CServer Write on Socket - Error");
                                    }
                                    close(csock);
                                    exit(EXIT_SUCCESS);
                                }
                                else{
                                    writeret = write(csock,"Invalid Command!\n",17);
                                    if(writeret == -1){
                                        perror("CServer Write on Socket - Error");
                                    }
                                }

                            }
                        }
                        if (pfds[1].revents & POLLIN) {
                            close(fsend[1]);
                            close(frec[0]);
                            int pread = read(fsend[0],buff,100);
                            if(pread == -1){
                                perror("Server-Child Read from Parent Pipe - Error");
                            }
                            if(strcmp(buff,"listprocesses") == 0){
                                showlistall(frec[1]);
                            }
                        }
                    }
                }
            }
        }
    } while (TRUE);
}

//-----------------------------------Main Ends-----------------------------------------------------------


//-----------------------------------Helper Methods------------------------------------------------------

void add(char *str, int fd){
    char *tok;
    int sum = 0;
    tok = strtok(str, " ");
    if(tok == NULL){
        printstring("Error: Not enough arguments!\n",29,fd);
    }
    else{
        while(tok != NULL) {
            sum = sum + atoi(tok);
            tok = strtok(NULL, " ");
        }
        print(sum,1,fd);
    }
}

void sub(char *str, int fd){
    char *tok;
    int sum = 0;
    tok = strtok(str, " ");
    if(tok == NULL){
        printstring("Error: Not enough arguments!\n",29,fd);
    }
    else{
        while( tok != NULL ) {
            if(sum == 0){
                sum = sum + atoi(tok);
            }
            else{
                sum = sum - atoi(tok);
            }
            tok = strtok(NULL, " ");
        }
        print(sum,2,fd);
    }
}

void mult(char *str, int fd){
    char *tok;
    int prod = 1;
    tok = strtok(str, " ");
    if(tok == NULL){
        printstring("Error: Not enough arguments!\n",29,fd);
    }
    else{
        while(tok != NULL) {
            prod = prod * atoi(tok);
            tok = strtok(str, " ");
        }
        print(prod,3,fd);
    }
}

void divi(char *str, int fd){
    char *tok;
    int zero = 0;
    tok = strtok(str, " ");
    if(tok == NULL){
        printstring("Error: Not enough arguments!\n",29,fd);
    }
    else{
        int div = atoi(tok);
        tok = strtok(str, " ");
        while( tok != NULL ) {
            if(atoi(tok) != 0){
                div = div / atoi(tok);
                tok = strtok(str, " ");
            }
            else if(atoi(tok) == 0){
                zero = 1;
                break;
            }
        }
        if(zero == 0){
            print(div,4,fd);
        }
        else{
            write(fd,"Error: Division by 0!\n",22);
        }
    }
}

void run(char *str, int fd){
    char *tok;
    char *arr[30];
    int i = 0;
    tok = strtok(str, " ");
    while(tok != NULL) {
        arr[i++] = tok;
        tok = strtok(str, " ");
    }
    arr[i] = NULL;
    if (i==0){
        write(fd,"Error: Not Enough Arguments!\n",29);
    }
    else{
        int pcc[2];
        pipe(pcc);
        fcntl(pcc[1], F_SETFD, FD_CLOEXEC);
        int pid = fork();

        if(pid == -1){
            perror("Run Fork - Error");
        }
        else if (pid > 0){
            if (signal(SIGCHLD,sigint_handler) == SIG_ERR){
                fprintf(stderr, "Cannot handle SIGCHLD!\n");
            }

            close(pcc[1]);
            int sprintfret;
            int sprintfret2;
            char buff[30];
            char buff2[30];
            char buff3[30];
            int readret = read(pcc[0],buff,9);
            if(readret == -1){
                perror("PCC Server Read Error");
            }
            else if(strcmp(buff,"ExecFail")==0){
                sprintfret2 = sprintf(buff3,"Exec Failed! - %s\n",arr[0]);
                if(sprintfret2 == -1){
                    perror("Sprintf Run");
                }
                int writeret = write(fd,buff3,sprintfret2);
                if(writeret == -1){
                    perror("Run Socket Exec Detail Write - Error");
                }
            }
            else if(readret == 0){
                runlist[countlist].pid = pid;
                strcpy(runlist[countlist].name,arr[0]);
                runlist[countlist].isActive = 1;
                runlist[countlist].start = time(0); 
                runlist[countlist].end = 0; 
                countlist++;
                sprintfret = sprintf(buff2,"Exec Successful! - %s\n",arr[0]);
                if(sprintfret == -1){
                    perror("Run Sprintf");
                }
                int writeret = write(fd,buff2,sprintfret);
                if(writeret == -1){
                    perror("Socket Exec Detail Write - Error");
                }
            }
        }
        else if(pid == 0){
            int execret = execvp(arr[0], arr);
            if(execret == -1){
                close(pcc[0]);
                int writeret2 = write(pcc[1],"ExecFail\0",9);
                if(writeret2 == -1){
                    perror("PCC Write Error");
                }
                exit(1);
            }
        }
    }
}


void showlistall(int fd){
    struct tm * ts;
    time_t elapsed;
    char buff[1500];
    int var = 0;
    int sno = 0;
    int hrs, hrs2;
    int min, min2;
    int sec, sec2;
    int diff_t;
    int sprintfret;
    int status;
    int waitret;
    
    sprintfret = sprintf(buff + var,"  |  Sno.  |   PID    |  isActive  |    Start     |     End      |   Elapsed   |  Name  \n");       
    var = var + sprintfret;    
    for(int i=0; i<countlist; i++){
        ts = localtime(&runlist[i].start);
        hrs = ts->tm_hour;
        min = ts->tm_min;
        sec = ts->tm_sec;

        if(runlist[i].end != 0){
            ts = localtime(&runlist[i].end);
            hrs2 = ts->tm_hour;
            min2 = ts->tm_min;
            sec2 = ts->tm_sec;
            diff_t = difftime(runlist[i].end, runlist[i].start);

            sprintfret = sprintf(buff + var,"  |   %d    |  %d   |      %d     |   %u:%u:%u   |   %u:%u:%u   |      %d     |  %s\n", ++sno, runlist[i].pid, runlist[i].isActive, hrs, min, sec, hrs2, min2, sec2, diff_t, runlist[i].name);
            var = var + sprintfret;
        }
        else{
            sprintfret = sprintf(buff + var,"  |   %d    |  %d   |      %d     |   %u:%u:%u   |   running    |   running   |  %s\n", ++sno, runlist[i].pid, runlist[i].isActive, hrs, min, sec, runlist[i].name);
            var = var + sprintfret;
        }
    }

    if(var == 88){
        write(fd,"List Empty!\n",12);
    }
    else{
        write(fd,buff,var);
    }
}

void showlist(int fd){
        struct tm * ts;
        time_t elapsed;
        char buff[1500];
        int var = 0;
        int sno = 0;
        int hrs;
        int min;
        int sec;
        int sprintfret;
        int status;
        int waitret;
    
        sprintfret = sprintf(buff + var,"  |  Sno.  |  PID    |  isActive  |    Start     |     End      |   Elapsed   |  Name  \n");       
        var = var + sprintfret;    
        for(int i=0; i<countlist; i++){
            if(runlist[i].isActive == 1){
                ts = localtime(&runlist[i].start);
                hrs = ts->tm_hour;
                min = ts->tm_min;
                sec = ts->tm_sec;
                sprintfret = sprintf(buff + var,"  |   %d    |  %d   |      %d     |   %u:%u:%u   |   running    |   running   |  %s\n", ++sno, runlist[i].pid, runlist[i].isActive, hrs, min, sec, runlist[i].name);
                    var = var + sprintfret;

            }
        }
        if(var == 88){
            write(fd,"List Empty!\n",12);
        }
        else{
            write(fd,buff,var);
        }
}


void help(int fd){
    int writeret = write(fd,"HELP\n 1. add: can be used to ADD numbers. Example: add 1 2 3 4 5\n 2. sub: can be used to SUBTRACT numbers. Example: sub 1 2 3 4 5\n 3. mul: can be used to MULTIPLY numbers. Example: mul 1 2 3 4 5\n 4. div: can be used to DIVIDE numbers. Example div: 100 10 2\n 5. run: you can OPEN programs using this command. Example: run gedit\n 6. list: lists down all programs that are active. Example: list\n 7. list all: lists down all programs that were opened using the run command (active and inactive). Example: list all\n 8. kill pid: kills process with specified pid. Example: kill pid 25365\n 9. kill name: kills process with specified name. Example: kill name gedit\n 10. kill all: kills all processes that are active. Example: kill all\n 11. disconnect: Disconnects the connection of the client with the server. Example: disconnect\n 12. exit: exits the program. Example: exit\n",867);
    //writeret = write(fd,"Hamza\n",6);
    if(writeret == -1){
        perror("Help Write Error");
    }
}

void killprocess(char *str, int fd){
    char *tok;
    int pid;
    int killret;
    int found = 0;

    tok = strtok(str, " ");
    if (tok == NULL){
        write(fd,"Invalid Kill Command! Please refer to help.\n",44);
    }
    else if(strcmp(tok,"all") == 0){
        for(int i=0;i<countlist;i++){
            if(runlist[i].isActive == 1){
                pid = runlist[i].pid;
                killret = kill(pid, SIGTERM);
                if(killret == -1){
                    write(fd,"Kill All - Error\n",17);
                }
            }
        }
        write(fd,"kill all is successfull!\n",25);
    }
    else if(strcmp(tok,"name") == 0){
        tok = strtok(NULL," ");
        if(tok == NULL){
            write(fd,"Please provide the process id!\n",31);
        }
        else{
            for(int i=0;i<countlist;i++){
                if(runlist[i].isActive == 1 && strcmp(runlist[i].name,tok) == 0){
                    pid = runlist[i].pid;
                    killret = kill(pid, SIGTERM);
                    if(killret == -1){
                        write(fd,"Kill Name - Error\n",18);
                    }
                    found = 1;
                    break;
                }
            }
            if(found == 0){
                write(fd,"Process Name does not exist or it is not active!\n",49);
            }
            else{
                write(fd,"kill name is successfull!\n",26);
            }
        }
    }
    else if(strcmp(tok,"pid") == 0){
        tok = strtok(NULL," ");
        if(tok == NULL){
            write(fd,"Please provide the process id!\n",31);
        }
        else{
            pid = atoi(tok);
            for(int i=0;i<countlist;i++){
                if(runlist[i].isActive == 1 && runlist[i].pid == pid){
                    killret = kill(pid, SIGTERM);
                    if(killret == -1){
                        write(fd,"Kill Pid - Error\n",17);
                    }
                    found = 1;
                    break;
                }
            }
            if(found == 0){
                write(fd,"Process ID does not exist or it is not active!\n",47);
            }
            else{
                write(fd,"kill pid is successfull!\n",25);
            }
        }
    }
    else if (strcmp(tok,"all") != 0 || strcmp(tok,"pid") != 0 || strcmp(tok,"name") != 0){
        write(fd,"Invalid Kill Command! Please refer to help.\n",44);
    }
}

void killall(){
    int pid;
    int killret;
    for(int i=0;i<countlist;i++){
        if(runlist[i].isActive == 1){
            pid = runlist[i].pid;
            killret = kill(pid, SIGTERM);
            if(killret == -1){
                perror("Exit Call Kill All - Error");
            }
        }
    }
}


void print(int result, int nr, int fd){
    char buff[50];
    int sprintfret;
    if(nr == 1){
        sprintfret = sprintf(buff,"Addition Result = %d\n",result);
        if(sprintfret == -1){
            perror("Sprintf print method");
        }
    }
    else if(nr == 2){
        sprintfret = sprintf(buff,"Subtraction Result = %d\n",result);
        if(sprintfret == -1){
            perror("Sprintf print method");
        }
    }
    else if(nr == 3){
        sprintfret = sprintf(buff,"Multiplication Result = %d\n",result);
        if(sprintfret == -1){
            perror("Sprintf print method");
        }
    }
    else if(nr == 4){
        sprintfret = sprintf(buff,"Division Result = %d\n",result);
        if(sprintfret == -1){
            perror("Sprintf print method");
        }
    }
    int writeret = write(fd,buff,sprintfret);
    if(writeret == -1){
        perror("Write");
    }
}

void printstring(char *buff,int size,int fd){
    int writeret = write(fd,buff,size);
    if(writeret == -1){
        perror("Write");
    }
}

void changeStatus(int pid){
    for (int i = 0; i<countlist; i++){
        if (runlist[i].pid == pid){
             runlist[i].isActive = 0;
        runlist[i].end = time(0);
            break;
        }
    }
}

//-----------------------------------Signal Handler---------------------------------------------

void sigint_handler(int signo){
    int status; int waitret;
    for(int i=0;i<countlist;i++){
        waitret = waitpid(runlist[i].pid,&status,WNOHANG);
        if(waitret == -1){
            perror("Wait Error");
        }
        else if(waitret == 0){
            continue;
        }
        else{
            if(WIFEXITED(status)){
                changeStatus(waitret);                    //change status on normal termination
            }
            if(WIFSIGNALED(status)){
                if(WTERMSIG(status) == 15){                //change status on signal termination
                    changeStatus(waitret);
                }
            }
        }
    }
}

void sigint_handler2(int signo){
    int status;
    int waitret = waitpid(-1,&status,WNOHANG);
    if(waitret == -1){
        perror("Wait Error");
    }
    if(WIFEXITED(status)){
        endconnection(waitret);                    //remove from conns (list)
    }
    return;
}

void listconnections(){    
    char buff[500];
    int var = 0;
    int sno = 0;
    for(int i=0; i<countconnection; i++){
        int sprintfret = sprintf(buff + var,"%d  IP = %s   Port = %d\n", ++sno,conns[i].ip,conns[i].port);
        var = var + sprintfret;
    }
    if(var == 0){
        write(1,"No Connections Available!\n\n",26);
    }
    else{
        write(1,buff,var);
        write(1,"\n",1);
    }
}

void listprocesses(){    
    int fsend;
    int frec;
    int writeret;
    int readret;
    int sprintfret;
    char buffcon[50];
    char buff[1500];
    int k = 0;
    
    if(countconnection > 0){
        for(int i=0; i<countconnection; i++){
            fsend = conns[i].fsend;
            frec = conns[i].frec;
            write(fsend,"listprocesses\0",14);
    
            readret = read(frec,buff,1500);
            sprintfret = sprintf(buffcon,"Connection %d) IP = %s   Port = %d\n",++k,conns[i].ip,conns[i].port);
            write(1,buffcon,sprintfret);
            write(1,buff,readret);
            write(1,"\n",1);
        }
    }
    else{
        write(1,"The connection list is empty.\n",30);
    }
}

void listprocesses2(char *ip){    
    int fsend;
    int frec;
    int writeret;
    int readret;
    int sprintfret;
    char buffcon[50];
    char buff[1500];
    int found = 0;

    for(int i=0; i<countconnection; i++){
        if(strcmp(conns[i].ip,ip) == 0){
            found = 1;
            fsend = conns[i].fsend;
            frec = conns[i].frec;
            write(fsend,"listprocesses\0",14);
    
            readret = read(frec,buff,1500);
            sprintfret = sprintf(buffcon,"Connection IP = %s   Port = %d\n",conns[i].ip,conns[i].port);
            write(1,buffcon,sprintfret);
            write(1,buff,readret);
            write(1,"\n",1);
        }
    }    
    if(found == 0){
        write(1,"IP address not found.\n",22);
    }
}

void endconnection(int pid){
    for(int i=0; i<countconnection; i++){
        if(conns[i].pid == pid){
            for (int k = i; k <  countconnection - 1; k++){
                conns[k] = conns[k + 1];
            }
        }
        countconnection--;
    }
}