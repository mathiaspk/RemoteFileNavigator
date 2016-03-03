#include <netinet/in.h> 
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <ctype.h>
#include "app_proto.h"

#define MAX_MSG_SIZE 256
//Function to find a clients current directory
char* findDirectory(int socket){
    CLIENT *tmp = head->next;
    while(1){
      //run through until the client is found
        if(tmp->socket == socket){
          //return current directory
          return tmp->current_directory;
        }else{
          tmp = tmp->next;
        }
    }
    return NULL;
}
//Update a clients current directory (cd)
int setDirectory(int socket, char *new_dir){
    CLIENT *tmp = head->next;
    while(1){
      //Run through until the client is found
        if(tmp->socket == socket){
          //Free, then set new directory
          free(tmp->current_directory);
          tmp->current_directory = strdup(new_dir);
          return 0;
        }else{
          tmp = tmp->next;
        }
    }
    return 0;
}
//Concatenates current directory with requested file,
//the tries to read the file on given path
int cat_file(char file[], int socket){
  //Find current directory
  char *directory = findDirectory(socket);
  //allocate space for the path
  char *path = calloc(strlen(directory)+strlen(file)+2,
                                       sizeof(char));
  char *buffer;
  FILE *fp;
  int c, filesize, count = 0;
  struct stat st;
  //Concatenate directory with filename
  strcat(path, directory);
  strcat(path, "/");
  strcat(path, file);
  //Check if path is valid
  if(lstat(path, &st) == -1){
    send_info(socket, "ERROR: unvalid path");
    free(path);
    return 0;
    //Check if its a regular file
  }else if(!S_ISREG(st.st_mode)){
    send_info(socket, "ERROR: unvalid filename");
    free(path);
    return 0;
  }
  else{
    //If all is well, read the file
    fp = fopen(path, "r");
    //Get the file size for allocating space
    filesize = st.st_size;
    buffer = calloc(filesize, sizeof(char));
    while(1){
      //Run through file
      c = fgetc(fp);
      if(c == EOF){
        //break on end of file
        buffer[count-1] = '\0';
        break;
      }
      //Check if character is printable, but ignore newline
      if(isprint(c) || c == '\n'){
        buffer[count] = c;
      }else{
        //If not printable, set dot
        buffer[count] = '.';
      }
      count++;
    }
    fclose(fp);
  }
  //Send the message, and free variables no longer in use
  send_info(socket, buffer);
  free(path);
  free(buffer);
  return 0;
}
//Prints a menu of available files for command 4 and 5 (stat, cat)
int file_menu(int socket, int id){
  int menusize = 256;
  int const_size = 256;
  char *menu = calloc(menusize, sizeof(char));
  //Start the menu with an explainatory request
  char *greet = "Please choose a file (ex: file.txt)\n";
  char ls[4] = "ls \0";
  FILE *in;
  int c, count = strlen(greet), i;
  //Get current directory
  char *directory = findDirectory(socket);
  //Allocate space for final command
  char *final_ls = calloc(strlen(directory) + strlen(ls) + 1, sizeof(char));
  //Add ls and then the path
  strcat(final_ls, ls);
  strcat(final_ls, directory);
  //using popen to get the output from ls
  //Checking for error
  if(!(in = popen(final_ls, "r"))){
    send_info(socket, "ERROR: Something went wrong");
    perror("popen");
    return EXIT_FAILURE;
  }
  //If no error, start by entering "greet" to the menu
  for(i = 0; i < strlen(greet); i++){
    menu[i] = greet[i];
  }
  while(1){
    //read the file written to by popen
    c = fgetc(in);
    //Realloc to make room for all the chars
    if(count == menusize-1){
      char *tmp = realloc(menu, menusize+=const_size);
      memset(&tmp[count], 0, const_size);
      menu = tmp;
    }
    if(feof(in)){
      //If end of file is reached, terminate the string,
      //then break.
      menu[count] = '\0';
      break;
    }
    menu[count] = c;
    count++;
  }
  //Send the menu to the client
  send_info(socket, menu);
  pclose(in);
  //Using id to know who requested the file menu in the first place
  if(id == 2){
    rec_info(socket, 2);//cat requested the file menu
    free(menu);
    free(final_ls);
    return 0;
  }else if(id == 3){
    rec_info(socket, 3);//fileinfo(stat) requested the file info
    free(menu);
    free(final_ls);
    return 0;
  }
  free(menu);
  free(final_ls);
  return 0;
}
//Gets the file info on a given path, 
//and returns whether its a regular file, link, directory or special file
int get_file_info(char filename[], int socket){
  struct stat file_info;
  char *file_type, *path = calloc(PATH_MAX, sizeof(char));
  int total_length;
  //Concatenate to get full path
  strcpy(path, findDirectory(socket));
  strcat(path, "/");
  strcat(path, filename);
  //Check to see if the path is valid
  if((lstat(path, &file_info)) == -1){
    send_info(socket, "ERROR: Unvalid path/filename");
    return 0;
  }
  //Check to see if its a regular file
  if(S_ISREG(file_info.st_mode)){
    char *regular = " is a regular file";
    file_type = regular;
  }
  //Check to see if its a directory
  else if(S_ISDIR(file_info.st_mode)){
    char *directory = " is a directory";
    file_type = directory;
  }
  //Check to see if its a link
  else if(S_ISLNK(file_info.st_mode)){
    char *link = " is a symbolic link";
    file_type = link;
  }
  //If non of the above, its reasonable to assume its a special file
  else{
    char *special = " is a file of special type";
    file_type = special;
  }
  //Get the length of the message to be sent back to the client
  total_length = (strlen(filename) + strlen(file_type));
  //Allocate the space necessary
  char *return_info = calloc(total_length+1, sizeof(char));

  int i, j, k = 0;
  //insert filename
  for(i = 0; i < strlen(filename); i++){
    return_info[i] = filename[i];
  }
  //After that, insert what type of file it is
  for(j = strlen(filename); j < total_length; j++){
    return_info[j] = file_type[k];
    k++;
  }
  //Terminate the string, then send and free
  return_info[total_length] = '\0';
  send_info(socket, return_info);
  free (path);
  free (return_info);
  return 0;
}
//Changing the current directory of a client
int change_directory(char command[], int socket){
  char *new_path;
  //Get the current path
  char *current_path = findDirectory(socket);
  struct stat file_info;
  int i, j, k = 1;
  //Moving to parent folder
  if(!strcmp(command, "..")){ 
    //Checking if the client is already in root
    if(strlen(root) < strlen(current_path)){
      //if not, find the last "/" in the path
      //and count characters after that
      for(i = 0; i < strlen(current_path); i++){
        k++;
        if(current_path[i] == '/'){
          k = 1;
        }
      }
      //Allocate space for the new path using the length
      //of the current path - number of chars after the last "/"
      //plus one for terminate
      new_path = calloc(strlen(current_path)-k+1, sizeof(char));
      //Now make the new path
      for(j = 0; j < strlen(current_path)-k; j++){
        new_path[j] = current_path[j];
      }
      new_path[strlen(new_path)] = '\0';
      //Update clients current directory, send and free
      setDirectory(socket, new_path);
      send_info(socket, new_path);
      free(new_path);
      return 0;
    }else{
      //If client is already in rootfolder
      //an error message is sent
      send_info(socket, "ERROR: You can not navigate outside the root folder");
      return 0;
    }
    return 0;
  }
  //Moving to subfolder
  if(command[0] == '/'){
    //Allocate new space using length of current directory
    //and length of the subfolder name
    new_path = calloc(strlen(current_path)+strlen(command)+1,
                                             sizeof(char));
    //Concatenate current path with subfolder name
    strcat(new_path, current_path);
    strcat(new_path, command);
    //Check if its a valid path
    if((lstat(new_path, &file_info)) == -1){
      send_info(socket, "ERROR: Not a valid path");
      free(new_path);
      return 0;
    }
    //Check if its actually a directory
    else if(!S_ISDIR(file_info.st_mode)){
      send_info(socket, "ERROR: Not a valid directory");
      free(new_path);
      return 0;      
    }else{
      //If so, update current directory, send and free
      setDirectory(socket, new_path);
      send_info(socket, new_path);
      free(new_path);
      return 0;
    }
  }else{
    //If the command given is not recognized
    //the cd submenu will be printed one more time
    //for the client
    cd_submenu(socket);
    return 0;
  }
  free(new_path);
  return 0;
}
//Sends a change directory submenu to the client
int cd_submenu(int socket){
  char submenu[] = "Where do u want to go?\n('..') - Parent directory\n('/subdir') - Subdirectory";
  send_info(socket, submenu);
  rec_info(socket, 4);
  return 0;
}
//Gets a client current directory and sends it to the client
int getpwd(int socket){
  char *directory = findDirectory(socket);
  send_info(socket, directory);
  return 0;
}
//Concatenates the string "ls " with a path
//and then sends it for execution.
int getLS(int socket){
  char *directory = findDirectory(socket);
  char ls[4] = "ls \0";
  char *final_ls = calloc(strlen(directory) + strlen(ls) + 1 , sizeof(char));
  strcat(final_ls, ls);
  strcat(final_ls, directory);
  ls_exec(final_ls, socket);
  free(final_ls);
  return 0;
}
//Used by the server to differentiate commands
//from client
int command_intrp(char command[], int socket){
  if(!strcmp(command, "1")){
    getLS(socket);
    return 0;
  }
  if(!strcmp(command, "2")){
    getpwd(socket);
    return 0;
  }
  if(!strcmp(command, "3")){
    cd_submenu(socket);
    return 0;
  }
  if(!strcmp(command, "4")){
    file_menu(socket, 3);
    return 0;
  }
  if(!strcmp(command, "5")){
    file_menu(socket, 2);
    return 0;
  }
  else{
    //If the command is not recognized, 
    //the client recieves an error message
    send_info(socket, "ERROR: Unknown command");
  }
  return 0;
}
//Executes ls on a given path
int ls_exec(char command[], int socket){
  FILE *in;
  int c, count = 0;
  char *buf = calloc(1, sizeof(char));
  //Check for popen error
  if(!(in = popen(command, "r"))){
    send_info(socket, "ERROR: Something went wrong");
    perror("popen");
    return EXIT_FAILURE;
  }
  //If no error, run through the file
  //created by popen
  while(1){
    //make room for all the chars with realloc
    char *tmp = realloc(buf, count+1);
    buf = tmp;
    c = fgetc(in);
    if(c == EOF){
      //If end of file is reached, terminate the string then break
      buf[count] = '\0';
      break;
    }
    buf[count] = c;
    count++;
  }
  //Send message, close file then free memory
  send_info(socket, buf);
  pclose(in);
  free(buf);
  return 0;
}
//Used by client to see available commands
void printmenu(void){
  printf("Please press a key:\n");
  printf("[1] list content of current directory (ls)\n");
  printf("[2] print name of current directory (pwd)\n");
  printf("[3] change current directory (cd)\n");
  printf("[4] get file information\n");
  printf("[5] display file (cat)\n");
  printf("[?] this menu\n");
  printf("[q] quit\n");
}
//Sends a message from client to server and vice versa.
//Gets a socket and and the message to be sent (command)
int send_info(int socket, char command[]){
  int total_msg_size = strlen(command);
  int left_to_send = total_msg_size;
  int current_index, bytes_sent = 0;
  //Run as long as there is more left to send
  while(left_to_send > (MAX_MSG_SIZE - 2)){//Minus 2 for header
    //Keeps track of the current index 
    //of the total message (where to read)
    current_index = total_msg_size - left_to_send;
    //Allocate space for a packet
    unsigned char partial_msg[MAX_MSG_SIZE];
    memset(partial_msg, 0, sizeof(partial_msg));
    int i, j = 2;
    //set the header, first byte is packet size
    //second byte flagges whether more packets are coming
    partial_msg[0] = (MAX_MSG_SIZE - 2);
    partial_msg[1] = '1';//More to send
    //Make the partial message packets
    for(i = current_index; i < total_msg_size; i++){
      partial_msg[j] = command[i];
      j++;
      //When the limit for package size is reached, the packet is sent
      if(j > MAX_MSG_SIZE){
        partial_msg[MAX_MSG_SIZE] = '\0';
        bytes_sent = send(socket, &partial_msg, sizeof(partial_msg), 0);
        //Check for errors on send
        if(bytes_sent == -1){
          perror("send");
          return EXIT_FAILURE;
        }
        #ifdef DEBUG
        printf("BYTES SENT: %d\n", bytes_sent);
        #endif
        //Set j back to 2 for next packet
        j = 2;
        left_to_send -= (MAX_MSG_SIZE -2);
        break;
      }
    }
  }//msg smaller than MSG_MAX_SIZE
  current_index = total_msg_size - left_to_send;
  unsigned char msg_size = left_to_send;
  unsigned char msg[msg_size + 3];//plus 3 for header and terminate
  memset(msg, 0, sizeof(msg));
  int i, j = 2;

  msg[0] = msg_size;
  msg[1] = '0';//No more to send
  //Make the message
  for(i = current_index; i < (total_msg_size); i++){
    msg[j] = command[i];
    j++;
  }
  //Terminate the message
  msg[msg_size + 3] = '\0';
  //Send it
  bytes_sent = send(socket, &msg, strlen((const char*)msg), 0);
  if(bytes_sent == -1){
    perror("send");
    return EXIT_FAILURE;
  }
  #ifdef DEBUG
  printf("BYTES SENT: %d\n", bytes_sent);
  printf("TOTAL BYTES SENT: %d\n", total_msg_size);
  #endif
  return 0;
}
//Recieves the packets sent by "send_info".
//Gets a socket and an ID
//The ID is used to identify "who" is recieving the message
int rec_info(int socket, int id){
  unsigned char flag;
  unsigned char info[3];
  info[2] = '\0';
  int current_rec_size, status;

  status = recv(socket, &info, 2, 0);//Read first header
  if(status == -1){
    perror("read");
    return EXIT_FAILURE;
  }
  if(status == 0){
    //connection was closed
    return -2;
  }
  //Set the flag
  flag = info[1];
  //Read first packet and check for error
  char *msg = calloc(info[0]+1, sizeof(char));
  if(recv(socket, msg, info[0], 0) == -1){
    perror("recv");
    return EXIT_FAILURE;
  }
  //Update the current number of recieves bytes
  current_rec_size = strlen(msg);
  //If there are more packets coming
  //it enters the loop below
  while(flag == '1'){
    //Memset the header to get rid of the old one
    memset(info, 0, strlen((const char*) info));
    //Reads the header. Using MSG_WAITALL
    //to avoid partial recieve errors
    if(recv(socket, &info, 2, MSG_WAITALL) == -1){
      perror("recv");
      return EXIT_FAILURE;
    }
    //Set the new flag
    flag = info[1];
    //reallocate the space to make room for the new packet
    char *newmsg = realloc(msg, (strlen(msg) + (info[0]+1)));
    memset(&newmsg[current_rec_size], 0, info[0]+1);
    msg = newmsg;
    //Read the packet and check for error
    if(recv(socket, &msg[current_rec_size], info[0], MSG_WAITALL) == -1){
      perror("recv");
      return EXIT_FAILURE;
    }
    //This one is used to let the client 
    //know that the packet is beeing sent
    //(useful when calling cat on huge files)
    if(id == 0){//client
      printf("%d\r", current_rec_size);
    }
    //Update the current number of recieved bytes
    current_rec_size += info[0];
  }
  //Terminate the string
  msg[current_rec_size] = '\0';
  //To end the \r
  if(id == 0){
    printf("\n");
  }
  //print the recieved message
  printf("%s\n", msg);
  #ifdef DEBUG
  printf("TOTAL BYTES RECIEVED: %d\n", current_rec_size);
  #endif

  //Checks the id to determine what to do next
  if(id == 1){//server
    command_intrp(msg, socket);
  }
  else if(id == 2){//cat
    cat_file(msg, socket);
  }
  else if(id == 3){//stat
    get_file_info(msg, socket);
  }
  else if(id == 4){//cd
    change_directory(msg, socket);
  }
  free(msg);
  return 0;

}
