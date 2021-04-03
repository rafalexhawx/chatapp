//CS 484 (Networks 1)
//Authors: Joshua Alexander, Alex Robic
//File name: server.c
//Purpose: server program to interact with the client
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
#define N 50000

typedef struct account {
    char *username, *ip;
} account; //Structure used to represent a logged in user

typedef struct chatr {
  char *password;
  account *chat_requester;
  account *dest_account;
} chat_request; //Structure to represent a chat request

unsigned int hash(char *s) {
  //A simple hash function
  unsigned int h = 0;
  int i;
  for(i = 0; i < strlen(s); i++) {
    h += (i+1)*s[i];
  }

  return h;
}

void generate_hashtable(account *hashtable[N]) {
  //Initiates the hashtable
  int i;
  for(i = 0; i < N; i++) {
    hashtable[i] = NULL;
  }
}

account *create_account(char *username, char *ip) {
  account *r = (account*) calloc(1, sizeof(account));
  r->ip = (char*) calloc(strlen(ip), sizeof(char));
  r->username = (char*) calloc(strlen(username), sizeof(char));
  strcpy(r->ip, ip);
  strcpy(r->username, username);
  return strlen(r->ip) && strlen(r->username) ? r : NULL;
}

chat_request create_chat(char *requested, char *requester, char *password, account *hashtable[N]) {
  chat_request r;
  r.password = (char*) calloc(strlen(password), sizeof(char));
  strcpy(r.password, password);
  r.chat_requester = hashtable[hash(requester)];
  r.dest_account = hashtable[hash(requested)];
  return r;
}

void createUser(char* username, char* password){
  FILE *fp;
  char newAccount[1024] = "";
  fp = fopen("users.txt", "a");
  
  strcpy(newAccount, username);
  strcat(newAccount, " ");
  strcat(newAccount, password);
  strcat(newAccount, "\n");
  
  fputs(newAccount, fp);
  fclose(fp);
  return;
}


char* loginUser(char* username, char* password){
  FILE *fp;
  char account[1024] = "";
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  fp = fopen("users.txt", "r");
  
  strcpy(account, username);
  strcat(account, " ");
  strcat(account, password);
  strcat(account, "\n");
  
  while ((read = getline(&line, &len, fp)) != -1) {
    if(strcmp(account, line) == 0){
      fclose(fp);
      return "loginSuccess";
    }
  }
  
  
  fclose(fp);
  return "loginFail";
}

void addLoggedInList(char* username, char* ip, account *hashtable[N]){
  hashtable[hash(username) % N] = create_account(username, ip);
}

void removeLoggedInList(char* username, account *hashtable[N]){
  hashtable[hash(username) % N] = NULL;
}

char *get_loggedInUsers(account *hashtable[N]) {
  char *result = (char *) calloc(1024, sizeof(char));
  int i;
  for(i = 0; i < N; i++) {
    account *r = hashtable[i];
    if(r != NULL) {
      strcat(result, r->username);
      strcat(result, ", ");
    }
  }
  result[strlen(result)-2] = '\0';
  return result;
}

char *get_usernameFromIP(char *ip, account *hashtable[N]) {
  char *result = (char*) calloc(100, sizeof(char));
  int i;
  for(i = 0; i < N; i++) {
    if(hashtable[i] != NULL) {
      if(strcmp(hashtable[i]->ip, ip) == 0) {
        strcpy(result, hashtable[i]->username);
        return result;
      }
    }
    
  }
  return NULL;
}

char *get_messageRequests(char *self_name, chat_request chats[N], int max_index) { //FUNCTION OK
  char *result = (char *) calloc(1024, sizeof(char));
  int i;
  for(i = 0; i < max_index; i++) {
   if(strcmp(chats[i].dest_account->username, self_name) == 0) {
     strcat(result, "(");
     strcat(result, chats[i].chat_requester->username);
     strcat(result, ",");
     strcat(result, chats[i].password);
     strcat(result, ")");
   } 
  }
  return result;
}

int main(int argc, char **argv) {
  account *hashtable[N];
  generate_hashtable(hashtable);
  chat_request chats[N] = {};
    char buffer[1024] = "";
    char bufferSplit[1024] = "";
    char username[1024] = "";
    char password[1024] = "";
    char message[1024] = "";
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address; 
    int chat_index = 0;
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(PORT);
    int b = bind(sock, (struct sockaddr *)&address, sizeof(address));
    if(b < 0) {
        fprintf(stderr, "ERROR AT BIND\n");
    }
    while(1) {
        socklen_t addrlen = sizeof(address);
        recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &address, &addrlen);
        if(strlen(buffer)) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(address.sin_addr.s_addr), ip, INET_ADDRSTRLEN);
            socklen_t addrlen = sizeof(address);
            printf("Recevied [%s] from %s\n", buffer, ip);
            
            strcpy(bufferSplit, strtok(buffer," "));
            
            
            if(strcmp(bufferSplit, "-CreateUser-") == 0){//CreateUser
              strcpy(bufferSplit, strtok(NULL, " "));
              strcpy(username, bufferSplit);
              
              strcpy(bufferSplit, strtok(NULL, " "));
              strcpy(password, bufferSplit);
              
              createUser(username, password);
              strcpy(message, "Account was created successfully!");
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);

            }
            else if(strcmp(bufferSplit, "-LoginUser-") == 0){//loginUser
              strcpy(bufferSplit, strtok(NULL, " "));
              strcpy(username, bufferSplit);
              
              strcpy(bufferSplit, strtok(NULL, " "));
              strcpy(password, bufferSplit);
              
              strcpy(message, loginUser(username, password));
              if(strcmp(message, "loginSuccess") == 0){
                addLoggedInList(username, ip, hashtable);
              }
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
            }
            else if(strcmp(bufferSplit, "-LogoutUser-") == 0){//loginUser
              strcpy(bufferSplit, strtok(NULL, " "));
              strcpy(username, bufferSplit);
              
              removeLoggedInList(username, hashtable);
            } else if(strcmp(bufferSplit, "-viewUsers-") == 0) {
              char message[1024] = "";
              strcat(message, get_loggedInUsers(hashtable));
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
            } else if(strcmp(bufferSplit, "-getIP-") == 0) {
              char *username = strtok(NULL, " ");
              printf("Getting IP for username %s", username);
              char *ip = hashtable[hash(username)]->ip;
              printf("--> %s\n", ip);
              char message[1024] = "";
              strcat(message, ip);
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
            } else if(strcmp(bufferSplit, "-startChat-") == 0) {
              char *dest_name = strtok(NULL, " ");
              char *password = strtok(NULL, "");
              chats[chat_index] = create_chat(dest_name, get_usernameFromIP(ip, hashtable), password, hashtable);
              chat_index++;
              char *message = "chatAddOK";
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
            } else if(strcmp(bufferSplit, "-getChatRequests-") == 0) {
              char *requests = get_messageRequests(get_usernameFromIP(ip, hashtable), chats, chat_index);
              char message[1024] = "";
              strcpy(message, requests);
              sendto(sock, message, strlen(message)+1, 0, (struct sockaddr*) &address, addrlen);
            }
        }
    }

    exit(0);
}















