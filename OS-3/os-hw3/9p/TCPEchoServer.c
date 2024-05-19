#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>
#include <time.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/mman.h>

#include <stdbool.h>

#define MAXPENDING 25    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 50

#define SHM_ID      0x1111    // ключ разделяемой памяти
#define PERMS       0666      // права доступа

void DieWithError(char *errorMessage);  /* Error handling function */
int HandleTCPClient(int clntSocket, int clntSockets[], void *msg_p);   /* TCP client handling function */

pid_t pid1 = 0, pid2 = 0, pid3 = 0;
sem_t *sem_p;

int servSock;                    /* Socket descriptor for server */
int clntSock1;                   /* Socket descriptor for client 1 */
int clntSock2;                   /* Socket descriptor for client 2 */
int clntSock3;                   /* Socket descriptor for client 3 */
int dsplSock;                    /* Socket descriptor for displayer */

int sockToClose = 0;

int shmid = 0;
void *first_p, *msg_p; 

int dspl_pid_cnt = 0;
pid_t dspl_pids[100];

int dsplSocks[100];


void sighandle(int nsig) {
  if (nsig == SIGINT) {
    int pid = 0;
    if (sockToClose != 0 /*pid == pid1 || pid == pid2 || pid == pid3 */) return;
    printf("SIGINT\n");
    
    char exitStr[RCVBUFSIZE] = {(char)10, (char)10};
    
    if (send(clntSock1, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock2, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    if (send(clntSock3, exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    
    printf("Socks %d, %d, %d\n", clntSock1, clntSock2, clntSock3);
    for (int i = 0; i < dspl_pid_cnt; ++i) {
      printf("Ending dspl %d\n", dsplSocks[i]);
      if (send(dsplSocks[i],  exitStr, RCVBUFSIZE, 0) != RCVBUFSIZE)
            DieWithError("send() failed");
    }
            
            
   
    
    kill(pid1, SIGTERM);
    kill(pid2, SIGTERM);
    kill(pid3, SIGTERM);
    for (int i = 0; i < dspl_pid_cnt; ++i) {
      kill(dspl_pids[i], SIGTERM);
    }
    
    //close(sockToClose);
    for (int i = 0; i < dspl_pid_cnt; ++i) {
      close(dsplSocks[i]);
    }
    
    if (shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
      DieWithError("eraser: shared memory remove error");
    }
    
    fflush(stdout);
    exit(0);
  }
  if (nsig == SIGTERM) {
    printf("SIGTERM\n");
    close(sockToClose);
    //for (int i = 0; i < *displays; ++i) close(displays[i + 1]);
    fflush(stdout);
    exit(0);
  }
}

    unsigned int clntLen;            /* Length of client address data structure */

    struct sockaddr_in echoDsplAddresses[20]; /* Displayer address */

void newSocket(int curr_dsp) {
  if ((dsplSock = accept(servSock, (struct sockaddr *) &(echoDsplAddresses[curr_dsp]),
                                 &clntLen)) < 0)
              DieWithError("accept() failed");
}

void my_post() {
  if(sem_post(sem_p) == -1) {
    DieWithError("sem_post: Incorrect post of posix semaphore");
    exit(-1);
  };
}

void my_wait() {
  if(sem_wait(sem_p) == -1) {
    DieWithError("sem_wait: Incorrect wait of posix semaphore");
    exit(-1);
  };
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
  
  int socks[4] = { clntSock1, clntSock2, clntSock3, -1 };
  
          
  //printf("DEBUG: page size %d\n", getpagesize());
  if ((shmid = shmget (SHM_ID, getpagesize() * 16, PERMS | IPC_CREAT)) < 0)
    DieWithError("writer: can not get shared memory segment");

  // получение адреса сегмента
  if ((first_p = shmat (shmid, 0, 0)) == NULL) {
    DieWithError("writer: shared memory attach error");
  }
  
  sem_p = (sem_t*)first_p; //mmap(0, sizeof (sem_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);
  msg_p = (char*)first_p + sizeof (sem_t);
  
  *(int*)msg_p = 0;
  //dspl_msgs = (char*)(msg_p) + 4;
  
  int iters = 0;
  
  my_post();
  
  pid1 = fork();
  if (pid1 == 0) {
    // child 1
    srand(time(0) + 0);
    sockToClose = socks[0];
    while(iters++ < 50) HandleTCPClient(clntSock1, socks, msg_p);
  } else {
    
    pid2 = fork();
    if (pid2 == 0) {
      // child 2
      srand(time(0) + 1000);
      sockToClose = socks[1];
      while(iters++ < 50) HandleTCPClient(clntSock2, socks, msg_p);
    } else {
      pid3 = fork();
      if (pid3 == 0) {
        // child 3
        srand(time(0) + 2000);
        sockToClose = socks[2];
        while(iters++ < 50) HandleTCPClient(clntSock3, socks, msg_p);
      } else {
        
        
        
        
        // parent
        
        while(true) {
          
          /* Wait for displayer to connect */
          if ((dsplSock = accept(servSock, (struct sockaddr *) &(echoDsplAddresses[dspl_pid_cnt]),
                                 &clntLen)) < 0)
              DieWithError("accept() failed");
            
          printf("Handling displayer %s\n", inet_ntoa(echoDsplAddresses[dspl_pid_cnt].sin_addr));
          
          dsplSocks[dspl_pid_cnt] = dsplSock;
          //(*displays)++;
          
          int cur_pid = dspl_pids[dspl_pid_cnt] = fork();
          
          ++dspl_pid_cnt;
          
        
          if (cur_pid == 0) {
            // display children
            
            sockToClose = dsplSock;
          
            
            int *dspl_msg_cnt = (int*)msg_p;
            char *dspl_msgs = (char*)(msg_p) + 4;
            
            int curr_sent = 0;
            for (;;) {
              if (curr_sent < *dspl_msg_cnt) {
                
                  /*printf("I SENT: ");
                for (int i = 0; i < RCVBUFSIZE; ++i) {
                  char c = (dspl_msgs + curr_sent)[i];
                  if (c < '0') printf("%d; ", (int)c);
                  else printf("%c; ", c);
                }
                printf("\n");*/
                
                if (send(dsplSock/*dspl socket*/, dspl_msgs + curr_sent, RCVBUFSIZE, 0) != RCVBUFSIZE) {
                  printf("Socket %d disconnected\n", dsplSock);
                  for (;;) { }
                }
                    
                curr_sent += RCVBUFSIZE;
              }
            }
            
          
          }
          
          
          
        }
        
      }
      
    }
      
    
  }
    
    
    printf("should not be here.\n");
    /* NOT REACHED */
}
