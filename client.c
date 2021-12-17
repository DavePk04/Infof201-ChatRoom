#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "common.h" //include le common.h

#define LENGTH 2048

// Global variables
int sockfd = 0;
char pseudo[32];


struct buffer {
    size_t msg_lenght;
    time_t timestamp;
    char *message;
};

//oblige le système à vider le tampon associé au flux de sortie spécifié
void str_overwrite_stdout() {
    printf("%s", "> ");
    fflush(stdout);
}

// remplace le retour a la ligne par le caractere de fin de ligne
void str_trim_lf(char *arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}



void recv_msg_handler() {
    char message[LENGTH] = {};
    while (1) {
        int receive = checked(recv(sockfd, message, LENGTH, 0));
        if (receive > 0) {
            printf("%s\n", message);  // affiche le msg send by server
            str_overwrite_stdout();
        }  else {
            printf("fermeture en cours...");
            exit(0);
        }
//        memset(message, 0, sizeof(message));  // optionnel
    }
}


int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // parsing des arguments et récupération
    strcpy(pseudo, argv[1]);                // on récupère le pseudo des paramètres et on le mets dans pseudo

    int size_of_ip_server = strlen(argv[2]); // on récupère la taille de l'ip du serveur
    char ip_server[size_of_ip_server];         // on initialise la taille d'un string pour l'ip serveur
    strcpy(ip_server, argv[2]);

    int size_of_port_ = strlen(argv[3]); // on récupère la taille du port
    char port_[size_of_port_];            // on initialise la taille d'un string pour le port
    strcpy(port_, argv[3]);


    long int value_port = strtol(port_, NULL, 10);
    long int port = value_port;

    str_trim_lf(pseudo, strlen(pseudo));


    printf("welcome : %s\n", pseudo);

    struct sockaddr_in server_addr;

    /* Socket settings */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_server);
    server_addr.sin_port = htons(port);


    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err == -1) {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // Send name
    send(sockfd, pseudo, 32, 0);   // send pseudo to server

    printf("=== WELCOME TO THE CHATROOM ===\n");   // optionnel

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    char *message = malloc(LENGTH + 1);
    while (fgets(message, LENGTH + 1, stdin)) {
        char *send_msg = malloc(LENGTH + 1);
        str_trim_lf(message, LENGTH + 1);

        struct buffer buf;
        buf.message = strdup(message);                  // à la place du strcpy on utilise strdup
        buf.msg_lenght = strlen(buf.message);           // on récupère la taille du message
        buf.timestamp = time(NULL);
        
        sprintf(send_msg, "%s: %ld %ld %s", pseudo, buf.msg_lenght, buf.timestamp,buf.message);  // stocke le msg "%s: %s\n" dans buffer(le tampon)
        checked(send(sockfd, send_msg, strlen(send_msg), 0));
        free(send_msg);
    }
    free(message);
    close(sockfd);

    return EXIT_SUCCESS;
}