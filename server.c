#include <netinet/in.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>

int main(int argc, char *argv[]){
    struct sockaddr_in serveraddr, clientaddr; 
    int clientaddrlen;
    int request_sock, sock;
    char buf[12]; 
    int port = atoi(argv[1]);

    if((request_sock = socket(AF_INET, SOCK_STREAM, 
                                        IPPROTO_TCP)) == -1){
        perror("socket");
        close(request_sock);
        return EXIT_FAILURE;
    }

    memset((void *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    bind(request_sock, (struct sockaddr *)&serveraddr, sizeof serveraddr);

    listen(request_sock, SOMAXCONN);

    sock = accept(request_sock,(struct sockaddr *)&clientaddr, 
                                                    &clientaddrlen);

    while(1){
        if((read(sock, buf,1)) == -1){
            perror("read");
            close(sock);
            close(request_sock);
            return EXIT_FAILURE;
        }
        buf[11] = '\0';
        printf("%c \n",buf);
    }
    close(sock);
    close(request_sock);
    return 0;

}
