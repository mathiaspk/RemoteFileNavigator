#include <netinet/in.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interface.h"

struct sockaddr_in serveraddr;
int sock;
char buf[80]; 
int port; 
char *hostname;
char *command;
char ipAdress[INET6_ADDRSTRLEN];

int getIP(void){
  struct addrinfo hints;
  struct addrinfo *res;
  int status;
  void* addr;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;

  if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }
  if (res->ai_family == AF_INET) { // IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
  }else { // IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    addr = &(ipv6->sin6_addr);
  }
  inet_ntop(res->ai_family, addr, ipAdress, sizeof ipAdress);
  printf("%s\n", ipAdress);
  return 0;
}

int commun(char c){
  //if(c == 'q')exit(0);
  if(c == '?')printmenu();
  else{
    printf("command: %c\n", c);
    if((write(sock, &c, 11)) == -1){
      perror("write");
      close(sock);
      return EXIT_FAILURE;
    }
  }
  return 0;
}

int main(int argc, char *argv[]){
    //int IPadress = inet_pton(AF_INET, argv[2], serveraddr);
    if(argv[1] == NULL || argv[2] == NULL){
      printf("Please specify 'port' and 'hostname'\n");
      exit(1);
    }else{
      port = atoi(argv[1]);
      hostname = argv[2];
      printf("MIN PORT ER: %d\n", port);
      printf("MIN HOST ER: %s\n", hostname);
    }
    sock = socket(AF_INET, SOCK_STREAM, 
                              IPPROTO_TCP);

    memset((void *) &serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    getIP();
    serveraddr.sin_addr.s_addr = inet_addr(ipAdress);

    serveraddr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr *)&serveraddr, sizeof serveraddr)){
      perror("connect");
      close(sock);
      return EXIT_FAILURE;
    } 

    prompt();
return 0;
}
