// Example code: A simple server side code, which echos back the received message.
// Handle multiple socket connections with select and fd_set on Linux
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //strlen
#include <sys/socket.h>
#include <sys/time.h>  //FD_SET, FD_ISSET, FD_ZERO macros
#include <unistd.h>    //close

#include "common.h"

// TODO: gerer les erreurs lors du parsing des arguments
int main(int argc, char *argv[]) {

    // variables pour le message
    size_t longueur_du_message;
    time_t timestamp;
    char* message;

    unsigned const long int port = (unsigned long int) (argv[0]);
    //TODO: fonction qui vérifie si le port entré est correcte --> https://wallu.pagesperso-orange.fr/pag-ports.htm max 65535
    int opt = 1;
    int master_socket = checked(socket(AF_INET, SOCK_STREAM, 0));
    checked(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)));

    // type of socket created
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    checked(bind(master_socket, (struct sockaddr *) &address, sizeof(address)));
    checked(listen(master_socket, 3));

    size_t addrlen = sizeof(address);
    fd_set readfds;
    int clients[1024];   // like fds table
    int nclients = 0;    // on doit definir ici le nbre de clients qui vont se connecter

    while (true) {
        FD_ZERO(&readfds);
        // Etape d'ajout du socket server et de tous les sockets clients dans le fd_set
        FD_SET(master_socket, &readfds);
        int max_fd = master_socket;
        for (int i = 0; i < nclients; i++) {
            FD_SET(clients[i], &readfds);
            if (clients[i] > max_fd) {
                max_fd = clients[i];
            }
        }
        // wait for an activity on one of the sockets, timeout is NULL
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(master_socket, &readfds)) {
            // Si c'est le master socket qui a des donnees, c'est une nouvele connexion.
            clients[nclients] = accept(master_socket, (struct sockaddr *) &address, (socklen_t *) &addrlen);
            nclients++;
        } else {
            // Sinon, c'est un message d'un client
            for (int i = 0; i < nclients; i++) {
                if (FD_ISSET(clients[i], &readfds)) {
                    char *buffer;
                    size_t nbytes = receive(clients[i], (void *) &buffer);
                    if (nbytes > 0) {  // closed
                        ssend(clients[i], buffer, nbytes);
                        free(buffer);
                    } else {
                        close(clients[i]);
                        // On deplace le dernier socket a la place de libre pur ne pas faire de trou.
                        clients[i] = clients[nclients - 1];
                        nclients--;
                    }
                }
            }
        }
    }

    return 0;
}