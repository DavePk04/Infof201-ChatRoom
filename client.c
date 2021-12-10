#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "common.h"



int main(int argc, char const *argv[]) {
    //parsing pour récupérer le pseudo, l'ip_serveur et le port
    int size_of_pseudo= strlen(argv[0]); // on récupère la taille du pseudo
    char pseudo[size_of_pseudo];            // on initialise la taille d'un string pour le pseudo
    strcpy(pseudo,*argv[0]);                // on récupère le pseudo des paramètres et on le mets dans pseudo
    unsigned const long int ip_serveur = (unsigned long int)(argv[1]);
    unsigned const long int port = (unsigned long int)(argv[2]);

    // variables pour le message
    size_t longueur_du_message;
    time_t timestamp;
    char* message[1024];
    char* recvmessage[1024];

    int sock = checked(socket(AF_INET, SOCK_STREAM, 0));
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // on mets le port qui a été entré en paramètre lors de l'appel

    // Conversion de string vers IPv4 ou IPv6 en binaire
    checked(inet_pton(AF_INET, ip_serveur, &serv_addr.sin_addr)); // on remplace l'ip par celui qui a été donné en paramtre

    checked(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)));



    while (fgets(message, 1024, stdin)) {
        longueur_du_message = strlen(message);
        checked(write(sock, message, longueur_du_message + 1));
        ssize_t nbytes = checked(read(sock, recvmessage, 1024));
        printf("Client received %s\n", recvmessage);
    }
    return 0;
}