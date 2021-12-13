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

#include "../common.h"

int main(int argc, char *argv[]) {
    int opt = 1;
    int master_socket = checked(socket(AF_INET, SOCK_STREAM, 0));
    checked(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)));

    // type of socket created
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    checked(bind(master_socket, (struct sockaddr *) &address, sizeof(address)));
    checked(listen(master_socket, 3));

    size_t addrlen = sizeof(address);
    fd_set readfds;
    int clients[1024];
    int nclients = 0;
    while (true) {
        FD_ZERO(&readfds);
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
            printf("accept\n");

        } else {
            // Sinon, c'est un message d'un client
            for (int i = 0; i < nclients; i++) {
                if (FD_ISSET(clients[i], &readfds)) {
                    char *buffer;
                    size_t nbytes = receive(clients[i], (void *) &buffer);
                    printf("buffer :%s\n", buffer);
                    if (nbytes > 0) {  // closed
                        printf("nclients :%d\n", nclients);
                        for (int j = 0; j < nclients; j++){
                          ssend(clients[j], buffer, nbytes);
                        }
                        //free(buffer);
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
}