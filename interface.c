#include <netinet/in.h> 
#include <sys/socket.h>
#include <netdb.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void commun(char c);

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

/*void commandexe(char i){
  switch(i){
    case '1':
      printf("LS\n");
      break;
    case '2':
      printf("PWD\n");
      break;
    case '3':
      printf("CD\n");
      break;
    case '4':
      printf("GET FILE INFO\n");
      break;
    case '5':
      printf("CAT\n");
      break;
    case '?':
      printmenu();
      break;
    case 'q':
      exit(0);
  }
}*/

void prompt(void){
  char command;
  printf("cmd (? for help)> ");
  while(1){
    if(command != '\n' & command != '\0'){
      commun(command);
      printf("cmd (? for help)> ");
    }
    scanf("%c", &command);
  }
}
