#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/socket.h>
#define DBG if (false)
#define CRIT_BEGIN 
#define CRIT_END 

#define RCVBUFSIZE 50   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */

void sys_err (char *msg) {
  puts (msg);
  exit (1);
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


void proger(int num, int sock) {
  srand(time(0) + 4000 + num * 1000);
  printf("Proger %d started working\n", num);
  char my_prog[11] = "\0";
  
  for (int i = 0; i < 10; ++i) {
    my_prog[i] = 'a' + rand() % 26;
  }
  bool my_prog_ok = false;
  char checker = (num + 1 + (rand() & 1)) % 3;
  sleep(rand() % 3);
  printf("Proger %d has program %s, sends to %d\n", num, my_prog, checker);
  
  char tocheck_type = 1, checked_type_ok = 2, checked_type_wa = 3;
  char cnum = num;
  
  CRIT_BEGIN
  resetBuffer();
  bufferToEcho(&checker, 1);
  bufferToEcho(&tocheck_type, 1);
  bufferToEcho(my_prog, 10);
  bufferToEcho(&cnum, 1);
  sendToServer(sock);
  CRIT_END
  
  char checking_prog[11] = "\0";
  
  while (true) {
    resetBuffer();
    
    char query_type = '\0', author = '\0';
    char mynum;
    
    recvFromServer(sock);
    readFromBuffer(&mynum, 1);
    readFromBuffer(&query_type, 1);
    
    if (query_type == tocheck_type) {
      readFromBuffer(checking_prog, 10);
      readFromBuffer(&author, 1);
      printf("Proger %d checking program %s\n", num, checking_prog);
      sleep(rand() % 3);
      if (checking_prog[0] <= 'f' || checking_prog[1] <= 'f') {
        printf("Proger %d declared program %s VALID\n", num, checking_prog);
        CRIT_BEGIN
        resetBuffer();
        bufferToEcho(&author, 1);
        bufferToEcho(&checked_type_ok, 1);
        sendToServer(sock);
        CRIT_END
      } else {
        printf("Proger %d declared program %s INVALID\n", num, checking_prog);
        CRIT_BEGIN
        resetBuffer();
        bufferToEcho(&author, 1);
        bufferToEcho(&checked_type_wa, 1);
        sendToServer(sock);
        CRIT_END
      }
    } else {
      printf("Proger %d saw that his program %s was checked\n", num, my_prog);
      if (query_type == checked_type_ok) {
        printf("Proger %d has VALID program\n", num);
        my_prog_ok = true;
      } else if (query_type == checked_type_wa) {
        printf("Proger %d has INVALID program\n", num);
        
        for (int i = 0; i < 10; ++i) {
          my_prog[i] = 'a' + rand() % 26;
        }
        sleep(rand() % 3);
        printf("Proger %d has program %s, sends to %d\n", num, my_prog, checker);
        CRIT_BEGIN
        resetBuffer();
        bufferToEcho(&checker, 1);
        bufferToEcho(&tocheck_type, 1);
        bufferToEcho(my_prog, 10);
        bufferToEcho(&cnum, 1);
        sendToServer(sock);
        CRIT_END
      } else if (query_type == 10) {
        return;
      }
    }
    
    
  }
}

/*
int main() {
  (void) signal(SIGINT, sighandle);
  (void) signal(SIGTERM, sighandle);
  
  if((p_sem = sem_open(sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create posix semaphore");
    exit(-1);
  };
  safe_post();
  
  char    name0[] = "aaa0.fifo";
  (void)umask(0);
  mknod(name0, S_IFIFO | 0666, 0);
  char    name1[] = "aaa1.fifo";
  (void)umask(0);
  mknod(name1, S_IFIFO | 0666, 0);
  char    name2[] = "aaa2.fifo";
  (void)umask(0);
  mknod(name2, S_IFIFO | 0666, 0);
  


  pid1 = fork();
  if (pid1 == 0) {
    // child 1
    srand(time(0) + 0);
    fd[0] = open(name0, O_RDONLY);
    fd[1] = open(name1, O_WRONLY);
    fd[2] = open(name2, O_WRONLY);
    proger(0);
  } else {
    
    pid2 = fork();
    if (pid2 == 0) {
      // child 2
      srand(time(0) + 1000);
      fd[0] = open(name0, O_WRONLY);
      fd[1] = open(name1, O_RDONLY);
      fd[2] = open(name2, O_WRONLY);
      proger(1);
    } else {
      //parent
      srand(time(0) + 2000);
      fd[0] = open(name0, O_WRONLY);
      fd[1] = open(name1, O_WRONLY);
      fd[2] = open(name2, O_RDONLY);
      proger(2);
      //close_all();
    }
      
    
  }
  // if (semctl(semid, 0, IPC_RMID, 0) < 0) {
  //   printf("Can\'t delete semaphore\n");
  //   return -1;
  // }
  return 0;
}
*/
