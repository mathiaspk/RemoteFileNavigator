#include <netinet/in.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <arpa/inet.h>
#include "app_proto.h"

struct sockaddr_in serveraddr;
char *hostname;
char ipAdress[INET6_ADDRSTRLEN];

//Function that uses getaddrinfo()
//to resolve host name, and get ipadress
int getIP(void){
  struct addrinfo hints;
  struct addrinfo *res;
  int status;
  void* addr;

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  //Getting the addressinfo, checking for error
  if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 1;
  }
  if (res->ai_family == AF_INET) { // If it is IPv4
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
  }else { // IF it is IPv6
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    addr = &(ipv6->sin6_addr);
  }
  //Set the variable "ipAdress" to the host ip address
  inet_ntop(res->ai_family, addr, ipAdress, sizeof ipAdress);
  //Done with this - free it
  freeaddrinfo(res);
  return 0;
}
//The loop used for the clients communication with the server.
int commun(int sock){
  //Input buffer = PATH_MAX to support longest possible path
  char command[PATH_MAX];
  //Running until getting "q"
  while(1){
    //prompt
    printf("cmd (? for help)> ");
    //Read from stdin
    fgets(command, sizeof(command), stdin);
    //remove newline to trim message
    strtok(command, "\n");
    //Check for "help"
    if(!strcmp(command, "?")){
      //Menu is handled localy, no need to send it from server
      printmenu();
      //Check for "quit", close the connection
    }else if(!strcmp(command, "q")){
      close(sock);
      printf("Connection closed\n");
      exit(0);
    }else{
      //If all is well, send the command/msg
      send_info(sock, command);
      //Wait for feedback from server + check for lost connection
      if(rec_info(sock, 0) == -2){
        printf("Lost connection to server\n");
        close(sock);
        exit(0);
      }
    }        
  } 
}

int main(int argc, char *argv[]){
  int sock, port;
    //Verifying correct usage
    if(argv[1] == NULL || argv[2] == NULL){
      printf("Usage: specify 'port' and 'hostname'\n");
      exit(1);
    }else{
      //Setting variables
      port = atoi(argv[1]);
      hostname = argv[2];
    }
    //Get a socket
    sock = socket(AF_INET, SOCK_STREAM, 
                              IPPROTO_TCP);

    memset((void*) &serveraddr, 0, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    //Get the host ip address
    getIP();
    //Set the IP address
    serveraddr.sin_addr.s_addr = inet_addr(ipAdress);
    //Set the port
    serveraddr.sin_port = htons(port);
    //Connect to the server, check for error
    if(connect(sock, (struct sockaddr *)&serveraddr, sizeof serveraddr)){
      perror("connect");
      close(sock);
      return EXIT_FAILURE;
    } 
  printf("Connected to %s on IP: %s\n", hostname, ipAdress);
  //Print the menu, and start communicating (commun)
  printmenu();
  commun(sock);
  
  return 0;
}
