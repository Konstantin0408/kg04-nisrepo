#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>
#include <time.h>
#include <sys/shm.h>

#include <stdbool.h>

#define MAXPENDING 25    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 50

#define SHM_ID      0x1111    // ключ разделяемой памяти
#define PERMS       0666      // права доступа

void DieWithError(char *errorMessage);  /* Error handling function */
int HandleTCPClient(int clntSocket, int clntSockets[], int *displays);   /* TCP client handling function */

pid_t pid1 = 0, pid2 = 0, pid3 = 0;

int servSock;                    /* Socket descriptor for server */
int clntSock1;                   /* Socket descriptor for client 1 */
int clntSock2;                   /* Socket descriptor for client 2 */
int clntSock3;                   /* Socket descriptor for client 3 */
int dsplSock;                   /* Socket descriptor for client 3 */

int sockToClose = 0;

int shmid = 0;
void *msg_p; 
int *displays = 0;




void sighandle(int nsig) {
  if (nsig == SIGINT) {
    int pid = 0;
    if (pid == pid1 || pid == pid2 || pid == pid3) return;
    printf("SIGINT\n");
    
    char exitStr[RCVBUFSIZE] = {(char)10, (char)10};
    
    if (send(clntSock1, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock2, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock3, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
        
    for (int i = 0; i < *displays; ++i)
      if (send(displays[i + 1],  exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
            
            
   
    
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);
    
    close(sockToClose);
    
    if (shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
      DieWithError("eraser: shared memory remove error");
    }
    
    fflush(stdout);
    exit(0);
  }
  if (nsig == SIGTERM) {
    printf("SIGTERM\n");
    close(sockToClose);
    for (int i = 0; i < *displays; ++i) close(displays[i + 1]);
    fflush(stdout);
    exit(0);
  }
}

    unsigned int clntLen;            /* Length of client address data structure */

    struct sockaddr_in echoDsplAddresses[20]; /* Displayer address */
   /* 
void newDspInfo(int curr_dsp, int *toServSock, unsigned int **toLen, struct sockaddr_in** toDspAddress) {
  *toServSock = servSock;
  *toLen = clntLen;
  *toDspAddress = &(echoDsplAddresses[curr_dsp]);
}*/


void newSocket(int curr_dsp) {
  if ((dsplSock = accept(servSock, (struct sockaddr *) &(echoDsplAddresses[curr_dsp]),
                                 &clntLen)) < 0)
              DieWithError("accept() failed");
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

    printf("Server IP address = %s. Wait...\n", inet_ntoa(echoServAddr.sin_addr));

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
            
        printf("Handling client %s\n", inet_ntoa(echoClntAddr3.sin_addr));
        
        

        /* clntSock is connected to a client! */

        
        //if (HandleTCPClient(clntSock1, clntSock2) == 0) return 0;
        
    // Forking
  
  int socks[3] = { clntSock1, clntSock2, clntSock3 };
  
          

  if ((shmid = shmget (SHM_ID, getpagesize(), PERMS | IPC_CREAT)) < 0)
    DieWithError("writer: can not get shared memory segment");

  // получение адреса сегмента
  if ((msg_p = shmat (shmid, 0, 0)) == NULL) {
    DieWithError("writer: shared memory attach error");
  }
  
  displays = (int*)msg_p;
  *displays = 0;
  
  int iters = 0;
  
  pid1 = fork();
  if (pid1 == 0) {
    // child 1
    srand(time(0) + 0);
    sockToClose = socks[0];
    while(iters++ < 50) HandleTCPClient(clntSock1, socks, displays);
  } else {
    
    pid2 = fork();
    if (pid2 == 0) {
      // child 2
      srand(time(0) + 1000);
      sockToClose = socks[1];
      while(iters++ < 50) HandleTCPClient(clntSock2, socks, displays);
    } else {
      pid3 = fork();
      if (pid3 == 0) {
        // child 3
        srand(time(0) + 2000);
        sockToClose = socks[2];
        while(iters++ < 50) HandleTCPClient(clntSock3, socks, displays);
      } else {
        // parent
        
        while(true) {
          
          /* Wait for displayer to connect */
          if ((dsplSock = accept(servSock, (struct sockaddr *) &(echoDsplAddresses[*displays]),
                                 &clntLen)) < 0)
              DieWithError("accept() failed");
            
          printf("Handling displayer %s\n", inet_ntoa(echoDsplAddresses[*displays].sin_addr));
          
          displays[(*displays) + 1] = dsplSock;
          (*displays)++;
          
        }
        
      }
      
    }
      
    
  }
    
    
    printf("should not be here.\n");
    /* NOT REACHED */
}
