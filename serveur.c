#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
 
 
int main(int argc, char **argv)
{
  int sockfd, sockfd2;          // descripteurs de socket
  fd_set readfds;               // ensemble des descripteurs en lecture qui seront surveilles par select
  int t[FD_SETSIZE];            // tableau qui contiendra tous les descripteurs de sockets, avec une taille egale a la taille max de l'ensemble d'une structure fd_set
  int taille=0;                 // nombre de descripteurs dans le tableau precedent
  char buf[1024];               // espace necessaire pour stocker le message recu
 
  memset(buf,'\0',1024);        // initialisation du buffer qui sera utilisé
 
  struct sockaddr_in my_addr;   // structure d'adresse qui contiendra les param reseaux du recepteur
  struct sockaddr_in client;    // structure d'adresse qui contiendra les param reseaux de l'expediteur
 
  // taille d'une structure sockaddr_in utile pour la fonction recvfrom
  socklen_t sin_size = sizeof(struct sockaddr_in);
 
  // verification du nombre d'arguments sur la ligne de commande
  if(argc != 2)
  {
    printf("Usage: %s port_local\n", argv[0]);
    exit(-1);
  }
 
  // creation de la socket
  sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 
  // initialisation de la structure d'adresse du serveur (pg local)
 
  // famille d'adresse
  my_addr.sin_family = AF_INET;
 
  // recuperation du port du serveur
  my_addr.sin_port = ntohs(atoi(argv[1]));
 
  // adresse IPv4 du serveur
  my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
 
  // association de la socket et des param reseaux du serveur
  if(bind(sockfd,(struct sockaddr*)&my_addr,sizeof(my_addr)) != 0)
  {
    perror("Erreur lors de l'appel a bind -> ");
    exit(1);
  }
 
  // indication de la limite MAX de la file d'attente des connexions entrantes
  if(listen(sockfd,10) != 0)
  {
    perror("Erreur lors de l'appel a listen -> ");
    exit(2);
  }
 
  printf("Attente de connexion\n");
 
  t[0]=sockfd;    // on ajoute deja la socket d'ecoute au tableau de descripteurs
  taille++;       // et donc on augmente "taille"
 
  while(1){              
            FD_ZERO(&readfds);                                        //il faut remettre tt les elements ds readfds a chaque recommencement de la boucle, vu que select modifie les ensembles
            int j;
            int sockmax=0;
            for(j=0;j<taille;j++){
                                   if(t[j] != 0)             
                                              FD_SET(t[j],&readfds);  // on remet donc tous les elements dans readfds
                                   if(sockmax < t[j])                 // et on prend ici le "numero" de socket maximal pour la fonction select
                                              sockmax = t[j];
                                   }
                                  
            if(select(sockmax+1,&readfds,NULL,NULL,NULL) == -1){      // on utilise le select sur toutes les sockets y compris celle d'ecoute
                                          perror("Erreur lors de l'appel a select -> ");
                                          exit(1);
                                          }
                                           
            if(FD_ISSET(sockfd,&readfds)){                            // si la socket d'ecoute est dans readfds, alors qqch lui a ete envoye (=connection d'un client)
                                          if((sockfd2 = accept(sockfd,(struct sockaddr*)&client,&sin_size)) == -1){     // on accepte la connexion entrante et on cree une socket...
                                                      perror("Erreur lors de accept -> ");
                                                      exit(3);
                                                      }
                                          printf("Connexion etablie avec %s\n", inet_ntoa(client.sin_addr));
                                          taille++;                                                                     // ...qui est donc ajoutee au tableau de descripteurs
                                          t[taille-1]=sockfd2;
                                          }
            int i;
            for(i=1;i<taille;i++){                                     // on parcourt tous les autres descripteurs du tableau
                            if(FD_ISSET(t[i],&readfds)){               // si une socket du tableau est dans readfds, alors qqch a ete envoye au serveur par un client
                                                        if(recv(t[i],&buf,1024,0) == -1){                                         // on stocke alors le message
                                                                                     perror("Erreur lors de la reception -> ");
                                                                                     exit(4);
                                                                                     }
                                                        printf("La chaine recue est: %s\n",buf);                                  // et on l'affiche
                                                        int k;
                                                        for(k=1;k<taille;k++){                                                    // puis on l'envoie a tous les clients...
                                                                              if(k != i){                                         // ...sauf l'envoyeur
                                                                                   if(send(t[k],&buf,1024,0) == -1){             
                                                                                                             perror("Erreur lors de l'appel a send -> ");
                                                                                                             exit(1);
                                                                                                             }
                                                                                   }
                                                                              }
                                                        }              // reste donc a resoudre le probleme de supprimer la socket du tableau, et modifier "taille"...
                            }
           }
}