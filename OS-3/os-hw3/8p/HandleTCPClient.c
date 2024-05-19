#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>     /* for strcmp() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */

#define RCVBUFSIZE 50   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */
//void newDspInfo(int curr_dsp, int *toServSock, unsigned int **toLen, struct sockaddr_in** toDspAddress);
void newSocket(int curr_dsp);

int *displayers = 0;

int curr_dsp = 0;

void displayInfo(char id, int arg1, int arg2) {
  char str[RCVBUFSIZE];
  str[0] = (char)3;
  str[1] = (char)20;
  str[2] = id;
  *((int*)(str + 3)) = arg1;
  *((int*)(str + 3) + 1) = arg2;
  
  while (curr_dsp < *displayers) {
    newSocket(curr_dsp);
    ++curr_dsp;
  }
  
  for (int i = 0; i < *displayers; ++i) {
    printf("DEBUG: displaySocket = %d\n", displayers[i + 1]);
    if (send(displayers[i + 1], str, RCVBUFSIZE, 0) != RCVBUFSIZE)
        DieWithError("send() failed [display]");
  }
}

int HandleTCPClient(int clntSocket, int clntSockets[], int *displays)
{
    displayers = displays;
    
    printf("Server handling socket %d...\n", clntSocket);
    displayInfo(1, clntSocket, 0);
    
    int rcvrId = -1;
    char echoBuffer[RCVBUFSIZE + 1];    /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    if (recvMsgSize > 0 && rcvrId == -1) rcvrId = *echoBuffer;
    int totalSize = recvMsgSize;
    
    int ret = 1;
    
    /* Send received string and receive again until end of transmission */
    for (;;)      /* ... indicates end of transmission */
    {
        printf("Server received %d bytes from socket %d\n", recvMsgSize, clntSocket);
        displayInfo(2, recvMsgSize, clntSocket);
        
        printf("DEBUG: rcvId = %d, sendSocket = %d\n", rcvrId, clntSockets[rcvrId]);
        
        if (*echoBuffer == '#') ret = 0;
        /* Echo message back to client */
        if (rcvrId == 3) {
          for (int i = 0; i < *displayers; ++i)
            if (send(displayers[i + 1], echoBuffer, recvMsgSize, 0) != recvMsgSize)
                DieWithError("send() failed [regular send to display]");
        }
        else if (rcvrId != -1 && send(clntSockets[rcvrId], echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed [regular send]");
        printf("Server sent %d bytes to socket %d\n", recvMsgSize, clntSockets[rcvrId]);
        displayInfo(3, recvMsgSize, clntSockets[rcvrId]);
        
        if (totalSize >= RCVBUFSIZE) break;

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
        if (recvMsgSize > 0 && rcvrId == -1) rcvrId = *echoBuffer;
        
        totalSize += recvMsgSize;
    }

    //close(clntSocket);    /* Close client socket */
    
    printf("Server handled socket %d\n", clntSocket);
    displayInfo(4, clntSocket, 0);
    return ret;
}
