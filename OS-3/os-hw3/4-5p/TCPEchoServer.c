#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>
#include <time.h>

#include <stdbool.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 50

void DieWithError(char *errorMessage);  /* Error handling function */
int HandleTCPClient(int clntSocket, int clntSockets[]);   /* TCP client handling function */

pid_t pid1 = 0, pid2 = 0;

int servSock;                    /* Socket descriptor for server */
int clntSock1;                   /* Socket descriptor for client 1 */
int clntSock2;                   /* Socket descriptor for client 2 */
int clntSock3;                   /* Socket descriptor for client 3 */

int sockToClose = 0;


void sighandle(int nsig) {
  if (nsig == SIGINT) {
    int pid = 0;
    if (pid == pid1 || pid == pid2) return;
    printf("SIGINT\n");
    
    char exitStr[RCVBUFSIZE] = {(char)10, (char)10};
    
    if (send(clntSock1, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock2, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock3, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
            
            
   
    
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    
    close(sockToClose);
    fflush(stdout);
    exit(0);
  }
  if (nsig == SIGTERM) {
    printf("SIGTERM\n");
    close(sockToClose);
    fflush(stdout);
    exit(0);
  }
}

int main(int argc, char *argv[])
{
    (void) signal(SIGINT, sighandle);
    (void) signal(SIGTERM, sighandle);
    
    
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr1;/* Client 1 address */
    struct sockaddr_in echoClntAddr2;/* Client 2 address */
    struct sockaddr_in echoClntAddr3;/* Client 3 address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    printf("Server IP address = %s. Wait...\n", inet_ntoa(echoClntAddr1.sin_addr));

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr1);

        /* Wait for client 1 to connect */
        if ((clntSock1 = accept(servSock, (struct sockaddr *) &echoClntAddr1,
                               &clntLen)) < 0)
            DieWithError("accept() failed");
            
        printf("Handling client %s\n", inet_ntoa(echoClntAddr1.sin_addr));
        
        /* Wait for client 2 to connect */
        if ((clntSock2 = accept(servSock, (struct sockaddr *) &echoClntAddr2,
                               &clntLen)) < 0)
            DieWithError("accept() failed");
            
        printf("Handling client %s\n", inet_ntoa(echoClntAddr2.sin_addr));
        
        /* Wait for client 3 to connect */
        if ((clntSock3 = accept(servSock, (struct sockaddr *) &echoClntAddr3,
                               &clntLen)) < 0)
            DieWithError("accept() failed");
            
        printf("Handling client %s\n", inet_ntoa(echoClntAddr2.sin_addr));

        /* clntSock is connected to a client! */

        
        //if (HandleTCPClient(clntSock1, clntSock2) == 0) return 0;
        
    // Forking
  
  int socks[3] = { clntSock1, clntSock2, clntSock3 };
  pid1 = fork();
  if (pid1 == 0) {
    // child 1
    srand(time(0) + 0);
    sockToClose = socks[0];
    while(true) HandleTCPClient(clntSock1, socks);
  } else {
    
    pid2 = fork();
    if (pid2 == 0) {
      // child 2
      srand(time(0) + 1000);
      sockToClose = socks[1];
      while(true) HandleTCPClient(clntSock2, socks);
    } else {
      //parent
      srand(time(0) + 2000);
      sockToClose = socks[2];
      while(true) HandleTCPClient(clntSock3, socks);
      
    }
      
    
  }
    
    
    printf("should not be here.\n");
    /* NOT REACHED */
}
