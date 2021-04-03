//CS 484 (Networks 1)
//Authors: Joshua Alexander, Alex Robic
//File name: chat.c
//Purpose: cchat program to interact between users
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

typedef struct arguments {
    int sock;
    struct sockaddr_in address;
    char *recv_name;
    char *dest_ip;
    char *password;
} arguments;

char *encrypt(char *plaintext, char *password) {
    //Uses a viegenere type encryption
    char *r = (char *) calloc(strlen(plaintext), sizeof(char));
    int j = 0;
    for(int i = 0; i < strlen(plaintext); i++) {
        r[i] = (plaintext[i] + password[j]);
        j = (j + 1) % strlen(password);
    }
    return r;
}

char *decrypt(char *ciphertext, char *password) {
    //Uses a viegenere type encryption
    char *r = (char *) calloc(strlen(ciphertext), sizeof(char));
    int j = 0;
    for(int i = 0; i < strlen(ciphertext); i++) {
        r[i] = (ciphertext[i] - password[j]);
        j = (j + 1) % strlen(password);
    }
    return r;
}

void *listen_for_messages(void *a) {
    arguments *args = (arguments*) a;
    while(1) {
        socklen_t addrlen = sizeof(args->address);
        char line[1024] = "";
        
        fgets(line, 1023, stdin);
        inet_aton(args->dest_ip, &(args->address.sin_addr));
        if(args->password != NULL) {
            strcpy(line, encrypt(line, args->password));
        }
        int s = sendto(args->sock, line, strlen(line), 0, (struct sockaddr*) &(args->address), addrlen);
        if(s >= 0) {
            strcpy(line, decrypt(line, args->password));
            printf("%s -> %s\n", args->recv_name, line);
        }
    }  
}

int main(int argc, char **argv) {
    char *destination_ip = argv[1];
    char *destination_name = argv[2];
    char *self_name = argv[3];
    char *password = argc >= 5 ? argv[4] : NULL;
    printf("Starting a chat with %s at IP %s\n", destination_name, destination_ip);
    int sock_send = socket(AF_INET, SOCK_DGRAM, 0);
    int sock_recv = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address, destination; 
    address.sin_family = AF_INET, destination.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY;
    destination.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    destination.sin_port = htons(PORT);
    arguments a = {sock_send, destination, self_name, destination_ip, password};
    bind(sock_recv, (struct sockaddr *)&address, sizeof(address));
    pthread_t thread_a;
    pthread_create(&thread_a, NULL, listen_for_messages, &a);
    while(1) {
        char buffer[1024] = "";
        socklen_t addrlen = sizeof(address);
        inet_pton(AF_INET, destination_ip, &(address.sin_addr));
        recvfrom(sock_recv, buffer, 1024, 0, (struct sockaddr*) &(address), &addrlen);
        
        if(password != NULL) {
            strcpy(buffer, decrypt(buffer, password));
        }
        if(strlen(buffer)) {
            printf("%s --> %s\n", destination_name, buffer);
        }
    }
    printf("End of loop\n");
    exit(0);
}