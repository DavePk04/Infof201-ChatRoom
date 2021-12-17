#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

int main(int argc, char **argv) {
    int sockfd_server, sockfd_client;          // descripteurs de socket
    fd_set readfds;               // ensemble des descripteurs en lecture qui seront surveilles par select
    int clients[FD_SETSIZE];            // tableau qui contiendra tous les descripteurs de sockets, avec une taille egale a la taille max de l'ensemble d'une structure fd_set
    int taille = 0;                 // nombre de descripteurs dans le tableau precedent
    int opt = 1;


    struct sockaddr_in addr_server;   // structure d'adresse qui contiendra les param reseaux du recepteur
    struct sockaddr_in addr_client;    // structure d'adresse qui contiendra les param reseaux de l'expediteur

    socklen_t sin_size = sizeof(struct sockaddr_in);

    if (argc != 2) {
        printf("Usage: %s port_local\n", argv[0]);
        exit(-1);
    }

    sockfd_server = checked(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    checked(setsockopt(sockfd_server, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)));

    addr_server.sin_family = AF_INET;
    addr_server.sin_port = ntohs(atoi(argv[1]));
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY);

    checked(bind(sockfd_server, (struct sockaddr *) &addr_server, sizeof(addr_server)));
    checked(listen(sockfd_server, 10));


    printf("Attente de connexion\n");

    clients[0] = sockfd_server;    // on ajoute deja la socket d'ecoute au tableau de descripteurs
    taille++;       // et donc on augmente "taille"

    while (1) {
        FD_ZERO(&readfds);                                        //il faut remettre tt les elements ds readfds a chaque recommencement de la boucle, vu que select modifie les ensembles
        int sockmax = 0;
        for (int j = 0; j < taille; j++) {
            if (clients[j] != 0)
                FD_SET(clients[j], &readfds);  // on remet donc tous les elements dans readfds
            if (sockmax < clients[j])                 // et on prend ici le "numero" de socket maximal pour la fonction select
                sockmax = clients[j];
        }

        checked(select(sockmax + 1, &readfds, NULL, NULL, NULL));


        if (FD_ISSET(sockfd_server, &readfds)) {                            // si la socket d'ecoute est dans readfds, alors qqch lui a ete envoye (=connection d'un client)
            sockfd_client = checked(accept(sockfd_server, (struct sockaddr *) &addr_client, &sin_size));
            printf("Connexion etablie avec %s\n", inet_ntoa(addr_client.sin_addr));
            taille++;                                                                     // ...qui est donc ajoutee au tableau de descripteurs
            clients[taille - 1] = sockfd_client;
        }

        else{
          for (int i = 1; i < taille; i++) {                                     // on parcourt tous les autres descripteurs du tableau
            if (FD_ISSET(clients[i], &readfds)) {               // si une socket du tableau est dans readfds, alors qqch a ete envoye au serveur par un client
                char *buf;               // espace necessaire pour stocker le message recu
                size_t nbytes = recv(clients[i], (void*)&buf, 1024, 0);
                if (nbytes < 0) {
                    close(clients[i]);
                    clients[i] = clients[taille - 1];
                    taille--;                                       // on stocke alors le message
                    //perror("Erreur lors de la reception -> ");
                    //exit(4);
                }
                else{
                  //printf("%s\n", buf);                                  // et on l'affiche
                  for (int k = 1; k < taille; k++) {                                                    // puis on l'envoie a tous les clients...
                    if (send(clients[k], (void*)&buf, 1024, 0) < 0) {
                        perror("Erreur lors de l'appel a send -> ");
                        //exit(1);
                    }
                  }
                }
            }   
        }
        }
    }
}