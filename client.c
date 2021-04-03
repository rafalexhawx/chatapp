//CS 484 (Networks 1)
//Authors: Joshua Alexander, Alex Robic
//File name: client.c
//Purpose: client program to interact with the server
//Date: 12/10/2020

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

#define PORT 8080 

//Uses the system function from stdlib to start the chat program with the appropriate arguments
void start_chat(char *dest_ip, char *dest_name, char *self_name, char *server_ip);


//Handles the client's inputs
void *client_handler(void *a) {
  
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in address; 
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  inet_pton(AF_INET, a, &address.sin_addr);
  address.sin_port = htons(PORT);
  socklen_t addrlen = sizeof(address);
  char buffer[1024] = "";
    char message[1024] = "";
    char username[1024] = "";
    char password[1024] = "";
    
     
    
  while(1){
      printf("Input 0 to sign up\nInput 1 to sign in.\n");
      int choice = 69;
      scanf("%d", &choice);
      if(choice == 0){
        printf("Enter a username\n");
        scanf("%s",username);
        printf("Enter a password\n");
        scanf("%s",password);
        
        strcpy(message, "-CreateUser- ");
        strcat(message, username);
        strcat(message, " ");
        strcat(message, password);
        sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
        recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
        printf("%s\n",buffer);
        continue;
      }
      else if(choice == 1){
        printf("Enter a username\n");
        scanf("%s",username);
        printf("Enter a password\n");
        scanf("%s",password);
        
        strcpy(message, "-LoginUser- ");
        strcat(message, username);
        strcat(message, " ");
        strcat(message, password);
        sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
        recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
        if(strcmp(buffer, "loginSuccess") == 0){
          printf("Welcome to the party, %s\n\n", username);
          break;
        }
        else{
          printf("bad login\n");
          continue;
        }

      }
      else{
        printf("Wrong input\n");
        continue;
      }
  }
      while(1){
    int choice = 69;
        printf("What do you want to do?\n0. View online users.\n1. Chat with somebody?\n2. View your message requests\n9. Logout\n-->");
        fscanf(stdin, "%d", &choice);
        if(choice == 9){
          //logout
          char exitMessage[1024] = "";
          strcpy(exitMessage, "-LogoutUser- ");
          strcat(exitMessage, username);
          sendto(sock, exitMessage, strlen(exitMessage)+1, 0, (struct sockaddr*) &address, addrlen);
          printf("\n Goodbye \n");
          exit(0);
        } else if(choice == 0) {
          char message[1024] = "-viewUsers-";
          char buffer[1024] = "";
          sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
          recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
          printf("Here are the online users\n--> %s\n", buffer);
        } else if(choice == 1) {
          char destname[100] = "";
          printf("Enter the user you want to chat with\n-->");
          fscanf(stdin, "%s", destname);
          printf("Connecting you to %s\n", destname);
          char message[1024] = "-getIP- ";
          strcat(message, destname);
          sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
          char dest_ip[1024] = "";
          recvfrom(sock, dest_ip, 1024, 0, (struct sockaddr*) &address, &addrlen);
          
          printf("The IP for the %s is %s\nConnecting you now\n", destname, dest_ip);
          

          start_chat(dest_ip, destname, username, a);
        }
         else if(choice == 2) {
          printf("Here are your message requests\n-->");
          char *message = "-getChatRequests-";
          char buffer[1024] = "";
          sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
          recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
          printf("Format = (requester_username, password chosen for encryption)\n->%s\n", buffer);
        }
      }
  
   return NULL;
}

void start_chat(char *dest_ip, char *dest_name, char *self_name, char *server_ip) {
  printf("Starting chat as %s with user %s at IP %s\n", self_name, dest_name, dest_ip);
  char command[1024] = "xterm -e ./chat ";
  strcat(command, dest_ip);
  strcat(command, " ");
  strcat(command, dest_name);
  strcat(command, " ");
  strcat(command, self_name);
  strcat(command, " ");
  strcat(command, " &");
  char password[1024] = "";
  printf("Enter the password for the conversation with %s (or simply press ENTER if there isn't one)\n-->", dest_name);
  fscanf(stdin, "%s", password);
  strcat(command, password);
  
  //The following section makes sure a chat window open up on the target machine
  struct sockaddr_in address; 
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons(PORT);
  inet_pton(AF_INET, server_ip, &address.sin_addr);
  socklen_t addrlen = sizeof(address);
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  char message[1024] = "-startChat- ";
  strcat(message, dest_name);
  if(strlen(password)) {
    strcat(message, " ");
    strcat(message, password);
  }
  printf("Sending connection request to %s\n", dest_ip);

  int s = sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
  char buffer[1024] = "";
  recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
  if(s <= 0) {
    //Detect send errors
    printf("Send error\n");
    return;
  } else if(strcmp(buffer, "chatAddOK") == 0) {
    printf("Sent the user a chat invite\n");
  }
  system(command);
  //printf("System executed [%s]\n", command); //Will replace this with: system(command);

}

int main(int argc, char **argv) {
    printf("Welcome!\n");
    client_handler(argv[1]);

    exit(0);
}