#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 1024
#define PORT 8080

int isUpperCase(const char *inputString);
void sendReceive(int i, int sockFd);
void connRequest(int *sockFd, struct sockaddr_in *server_addr);
char *nickname;

int main(int argc, char *argv[]) {
  int sockFd, fdMax, i;
  struct sockaddr_in server_addr;
  fd_set masterFdSet;
  fd_set readFdSet;
  nickname = argv[1];

  if (argc != 2) {
    printf("Usage: %s <your name>\n", argv[0]);
    return 1;
  }

  connRequest(&sockFd, &server_addr);
  FD_ZERO(&masterFdSet);
  FD_ZERO(&readFdSet);
  FD_SET(0, &masterFdSet);
  FD_SET(sockFd, &masterFdSet);
  fdMax = sockFd;

  while (1) {
    readFdSet = masterFdSet;
    
    if (select(fdMax + 1, &readFdSet, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }

    for (i = 0; i <= fdMax; i++)
      if (FD_ISSET(i, &readFdSet))
        sendReceive(i, sockFd);
  }
  
  close(sockFd);
  
  return 0;
}

int isUpperCase(const char *inputString) {
  int i, len = strlen(inputString);
  
  for (i = 0; i < len; i++) {
    if (inputString[i] >= 'a' && inputString[i] <= 'z') {
      return 0;
    }
  }
  
  return 1;
}

void sendReceive(int i, int sockFd) {
  char sendBuffer[BUFSIZE], 
    receiveBuffer[BUFSIZE];
  int bytesReceived;

  if (i == 0) {
    fgets(sendBuffer, BUFSIZE, stdin);
    if (strcmp(sendBuffer, "exit\n") == 0) {
      exit(0);
    } else if (isUpperCase(sendBuffer)) {
      printf("%s %s\n", "[Server]:", "\nCapital Letters Became Internet Code for Yelling\n");
    } else {
      char tempFormatString[BUFSIZE];
      sprintf(tempFormatString, "%s: %s", nickname, sendBuffer);
      send(sockFd, tempFormatString, BUFSIZE, 0);
    }
  } else {
    bytesReceived = recv(sockFd, receiveBuffer, BUFSIZE, 0);
    receiveBuffer[bytesReceived] = '\0';
    printf("%s", receiveBuffer);
    fflush(stdout);
  }
}

void connRequest(int *sockFd, struct sockaddr_in *server_addr) {
  if ((*sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket");
    exit(1);
  }
  
  server_addr->sin_family = AF_INET;
  server_addr->sin_port = htons(PORT);
  server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(server_addr->sin_zero, '\0', sizeof server_addr->sin_zero);

  if (connect(*sockFd, (struct sockaddr *)server_addr, sizeof(struct sockaddr)) == -1) {
    perror("connect");
    exit(1);
  }
}
