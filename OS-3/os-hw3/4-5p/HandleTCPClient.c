#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>     /* for strcmp() */

#define RCVBUFSIZE 50   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */

int HandleTCPClient(int clntSocket, int clntSockets[])
{
    printf("Server handling socket %d...\n", clntSocket);
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
        if (*echoBuffer == '#') ret = 0;
        /* Echo message back to client */
        if (rcvrId != -1 && send(clntSockets[rcvrId], echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
        printf("Server sent %d bytes to socket %d\n", recvMsgSize, clntSockets[rcvrId]);
        
        if (totalSize >= RCVBUFSIZE) break;

        /* See if there is more data to receive */
        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
        if (recvMsgSize > 0 && rcvrId == -1) rcvrId = *echoBuffer;
        
        totalSize += recvMsgSize;
    }

    //close(clntSocket);    /* Close client socket */
    
    printf("Server handled socket %d\n", clntSocket);
    return ret;
}
