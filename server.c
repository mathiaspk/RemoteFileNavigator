#include <netinet/in.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h> 
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <signal.h> 
#include <linux/limits.h>
#include "app_proto.h"

//Adding client to the linked-list structure
int addClient(int sock, char directory[]){
    CLIENT *client = malloc(sizeof(CLIENT));
    client->next = head->next;
    head->next = client;
    //Set the necessary variables
    client->socket = sock;
    client->current_directory = strdup(directory);
    //Message prompt on server side
    printf("Client with sockNr: %d added\n", client->socket);
    return 0;
}
//Remove client from the linked-list structure if disconnected
int delClient(int sock){
    CLIENT *previous = head, *tmp = head->next;
    while(1){
        //Find the correct client
        if(tmp->socket == sock){
            previous->next = tmp->next;
            //prompt on server side
            printf("Client with sockNr %d was removed\n", tmp->socket);
            //Free memory
            free(tmp->current_directory);
            free(tmp);
            return 0;
        }else{
            previous = previous->next;
            tmp = tmp->next;
        }
    }
    return 0; 
}

//Catches ctrl-c, cleans the linked-list structure, then exit
void signal_handler(int sig){
    printf("SERVER SHUTTING DOWN\n");
    CLIENT *current, *next;
    for(current = head->next; current ; current=next){
        next = current->next;
        free(current->current_directory);
        free(current);
    }
    free(head);
    exit(0);
}

int main(int argc, char *argv[]){
    fd_set master;
    fd_set read_fds;
    struct sockaddr_in serveraddr; 
    int server_sock, sock, fd_max, nbytes, i;
    signal(SIGINT, signal_handler);

    //Checking for correct usage
    if(argv[1] == NULL){
        printf("Usage: specify 'port'\n");
        exit(0);
    }
    //Initialize the linked list
    head = malloc(sizeof(CLIENT));
    head->next = NULL;
    //Get the root directory (Where server is running)
    if(getcwd(root, sizeof(root)) == NULL){
        perror("getcwd() error");
        return EXIT_FAILURE;
    }

    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    //Setting the port variable
    int port = atoi(argv[1]);
    //Getting a listening socket, and check for error
    if((server_sock = socket(AF_INET, SOCK_STREAM, 
                                        IPPROTO_TCP)) == -1){
        perror("socket");
        close(server_sock);
        return EXIT_FAILURE;
    }
    //Set necessary variables
    memset((void *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);
    //Bind the socket to the given port
    bind(server_sock, (struct sockaddr *)&serveraddr, sizeof serveraddr);
    //Listen for incomming connections
    if((listen(server_sock, 10)) == -1){
        perror("listen");
        return EXIT_FAILURE;
    }
    //Add "listener" to the master set
    FD_SET(server_sock, &master);

    //Keep track of the biggest fd
    fd_max = server_sock;

    //main loop
    while(1){
        read_fds = master;
        if(select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1){
            perror("select");
            return EXIT_FAILURE;
        }
        //run through existing connections looking for data to read
        for(i = 0; i <= fd_max; i++){            
            if(FD_ISSET(i, &read_fds)){ //We got one!!
                if(i == server_sock){
                    //handle new connections
                    printf("NEW CONNECTION\n");
                    struct sockaddr_in clientaddr; 
                    socklen_t clientaddrlen = sizeof(clientaddr);
                    //clientaddrlen = sizeof(clientaddr);
                    if((sock = accept(server_sock,(struct sockaddr *)&clientaddr, 
                                                &clientaddrlen)) == -1){
                        perror("accept");
                        close (server_sock);
                        return EXIT_FAILURE;
                    }
                    FD_SET(sock, &master); //Add to master set
                    if(sock > fd_max){ //Keep track of the max
                        fd_max = sock;
                    }
                    addClient(sock, root);
                } else {
                    //Handle data from client
                    if((nbytes = rec_info(i, 1)) == -2){
                        //connection is closed by client
                        printf("socket %d hung up\n", i);
                        delClient(i);
                        close(i);
                        FD_CLR(i, &master);  //remove client from master set
                    }//END CHECK IF DICONNECTED
                }//END HANDLE DATA FROM CLIENT 
            }//END GOT NEW INCOMING CONNECTION
        }//END LOOPING THROUGH FDs

    }//END WHILE LOOP
    return 0;
}//END MAIN
