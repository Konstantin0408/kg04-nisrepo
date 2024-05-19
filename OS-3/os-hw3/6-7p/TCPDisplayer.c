#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <signal.h>

#define RCVBUFSIZE 50   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */

void proger(int num, int sock);

int sock;                        /* Socket descriptor */

void sighandle(int nsig) {
  if (nsig == SIGINT) {
    
    close(sock);
    exit(0);
  }
}


char echoBuffer[RCVBUFSIZE];
char *echoPointer = echoBuffer;

void resetBuffer() {
  echoPointer = echoBuffer;
}

void bufferToEcho(void *str, int len) {
  char *c_str = (char*)str;
  while (len--) {
    *(echoPointer++) = *(c_str++);
  }
}

void sendToServer(int sock) {
  if (send(sock, echoBuffer, RCVBUFSIZE, 0) != RCVBUFSIZE)
      DieWithError("send() sent a different number of bytes than expected");
  echoPointer = echoBuffer;
}

void recvFromServer(int sock) {
  int recvMsgSize;
  while (echoPointer < echoBuffer + RCVBUFSIZE) {
    if ((recvMsgSize = recv(sock, echoPointer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    //for (char *i = echoPointer; i < echoPointer + recvMsgSize; ++i) printf("%d; ", (int)(*i));
    //if (recvMsgSize > 0) printf("\n");
    echoPointer += recvMsgSize;
  }
  echoPointer = echoBuffer;
}

void readFromBuffer(void *str, int len) {
  char *c_str = (char*)str;
  while (len--) {
    *(c_str++) = *(echoPointer++);
  }
}


int main(int argc, char *argv[])
{
    (void) signal(SIGINT, sighandle);
    
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char echoStringAndTerm[60];    
    char *echoString = echoStringAndTerm + 1;    /* String to send to echo server */ 
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */
    
    const char endstr[] = "The End";

    if ((argc < 2) || (argc > 3))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n",
               argv[0]);
       exit(1);
    }
    
    servIP = argv[1];             /* First arg: server IP address (dotted quad) */

    if (argc == 3)
        echoServPort = atoi(argv[2]); /* Use given port, if any */
    else
        echoServPort = 7;  /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    
    for (;;) {
      resetBuffer();
      recvFromServer(sock);
      char mynum, query_type;
      readFromBuffer(&mynum, 1);
      readFromBuffer(&query_type, 1);
      if (query_type == 10) return 0;
      if (query_type == 20) {
        char id;
        int arg1, arg2;
        readFromBuffer(&id, 1);
        readFromBuffer(&arg1, 4);
        readFromBuffer(&arg2, 4);
        if (id == 1) printf("Server handling socket %d...\n", arg1);
        if (id == 2) printf("Server received %d bytes from socket %d\n", arg1, arg2);
        if (id == 3) printf("Server sent %d bytes to socket %d\n", arg1, arg2);
        if (id == 4) printf("Server handled socket %d\n", arg1);
      }
      if (query_type == 30) {
        char id;
        int arg1, arg3;
        char arg2[11];
        readFromBuffer(&id, 1);
        readFromBuffer(&arg1, 4);
        readFromBuffer(&arg2, 10);
        readFromBuffer(&arg3, 4);
        if (id == 1) printf("Proger %d has program %s, sends to %d\n", arg1, arg2, arg3);
        if (id == 2) printf("Proger %d checking program %s\n", arg1, arg2);
        if (id == 3) printf("Proger %d declared program %s VALID\n", arg1, arg2);
        if (id == 4) printf("Proger %d declared program %s INVALID\n", arg1, arg2);
        if (id == 5) printf("Proger %d saw that his program %s was checked\n", arg1, arg2);
        if (id == 6) printf("Proger %d has VALID program\n", arg1);
        if (id == 7) printf("Proger %d has INVALID program\n", arg1);
      }
      
    }
    
    
    //exit(0);
}
