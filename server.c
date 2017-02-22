#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFSIZE 1024

void sendToAll(int j, int i, int sockFd, int bytesReceived, char *receiveBuffer,fd_set *masterFdSet);
void sendReceive(int i, fd_set *masterFdSet, int sockFd, int fdMax);
void connAccept(fd_set *masterFdSet, int *fdMax, int sockFd, struct sockaddr_in *client_addr);
void connRequest(int *sockFd, struct sockaddr_in *my_addr);

int main() {
  fd_set masterFdSet;
  fd_set readFdSet;
  int fdMax, i;
  int sockFd = 0;
  struct sockaddr_in my_addr, client_addr;

  FD_ZERO(&masterFdSet);
  FD_ZERO(&readFdSet);
  connRequest(&sockFd, &my_addr);
  FD_SET(sockFd, &masterFdSet);

  fdMax = sockFd;
  
  while (1) {
    readFdSet = masterFdSet;
    if (select(fdMax + 1, &readFdSet, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }

    for (i = 0; i <= fdMax; i++) {
      if (FD_ISSET(i, &readFdSet)) {
        if (i == sockFd)
          connAccept(&masterFdSet, &fdMax, sockFd, &client_addr);
        else
          sendReceive(i, &masterFdSet, sockFd, fdMax);
      }
    }
  }
  
  return 0;
}

void sendToAll(int j, int i, int sockFd, int bytesReceived, char *receiveBuffer, fd_set *masterFdSet) {
  if (FD_ISSET(j, masterFdSet)) {
    if (j != sockFd && j != i) {
      if (send(j, receiveBuffer, bytesReceived, 0) == -1) {
        perror("send");
      }
    }
  }
}

void sendReceive(int i, fd_set *masterFdSet, int sockFd, int fdMax) {
  int bytesReceived, j;
  char receiveBuffer[BUFSIZE];

  if ((bytesReceived = recv(i, receiveBuffer, BUFSIZE, 0)) <= 0) {
    if (bytesReceived == 0) {
      printf("socket %d hung up\n", i);
    } else {
      perror("recv");
    }
    
    close(i);
    FD_CLR(i, masterFdSet);
  } else {
    for (j = 0; j <= fdMax; j++) {
      sendToAll(j, i, sockFd, bytesReceived, receiveBuffer, masterFdSet);
    }
  }
}

void connAccept(fd_set *masterFdSet, int *fdMax, int sockFd, struct sockaddr_in *client_addr) {
  socklen_t addrLen;
  int newSocketFd;
  addrLen = sizeof(struct sockaddr_in);
  
  if ((newSocketFd = accept(sockFd, (struct sockaddr *)client_addr, &addrLen)) == -1) {
    perror("accept");
    exit(1);
  } else {
    FD_SET(newSocketFd, masterFdSet);
    if (newSocketFd > *fdMax) {
      *fdMax = newSocketFd;
    }
    
    printf("Connection from %s @ %d \n", inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
  }
}

void connRequest(int *sockFd, struct sockaddr_in *my_addr) {
  int yes = 1;

  if ((*sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }

  my_addr->sin_family = AF_INET;
  my_addr->sin_port = htons(PORT);
  my_addr->sin_addr.s_addr = INADDR_ANY;
  memset(my_addr->sin_zero, '\0', sizeof my_addr->sin_zero);

  if (setsockopt(*sockFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  if (bind(*sockFd, (struct sockaddr *)my_addr, sizeof(struct sockaddr)) == -1) {
    perror("Unable to bind");
    exit(1);
  }
  
  if (listen(*sockFd, 10) == -1) {
    perror("listen");
    exit(1);
  }
  
  printf("\nServer running @ 8080...\n");
  fflush(stdout);
}