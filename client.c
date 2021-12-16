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

#define LENGTH 2048

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char pseudo[32];


struct buffer
{
	size_t msg_lenght;
	time_t timestamp;
	char message[LENGTH];
};

//oblige le système à vider le tampon associé au flux de sortie spécifié
void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

// remplace le retour a la ligne par le caractere de fin de ligne
void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void send_msg_handler() {
  char message[LENGTH] = {};
  char send_msg[LENGTH + 32] = {};
	//char buffer[LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();       // Optionnel
    fgets(message, LENGTH, stdin);   // recupere le msg entered in stdin
    str_trim_lf(message, LENGTH);

    struct buffer buf;
    strcpy(buf.message, message);
    buf.msg_lenght = sizeof(buf.message);
    buf.timestamp = time(NULL);
    // printf("msg : %s", buf.message);
    // printf("msg_lenght : %d", buf.msg_lenght);
    // printf("msg : %ld", buf.timestamp);
    if (strcmp(message, "exit") == 0) {   // optionnel
			break;
    } else {
      sprintf(send_msg, "%s: %s %d %ld", pseudo, buf.message, buf.msg_lenght, buf.timestamp);  // stocke le msg "%s: %s\n" dans buffer(le tampon)
      send(sockfd, send_msg, strlen(send_msg), 0);
    }

		bzero(message, LENGTH);
    bzero(buf.message, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}

void recv_msg_handler() {
	char message[LENGTH] = {};
  while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {
      printf("%s\n", message);  // affiche le msg send by server
      str_overwrite_stdout();
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));  // optionnel
  }
}


int main(int argc, char **argv){
	printf("args : %d\n", argc);
	if(argc != 4){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int size_of_pseudo= strlen(argv[1]); // on récupère la taille du pseudo
    strcpy(pseudo,argv[1]);                // on récupère le pseudo des paramètres et on le mets dans pseudo

    int size_of_ip_server= strlen(argv[2]); // on récupère la taille du pseudo
    char ip_server[size_of_ip_server];            // on initialise la taille d'un string pour le pseudo
    strcpy(ip_server,argv[2]);

    int size_of_port_= strlen(argv[3]); // on récupère la taille du pseudo
    char port_[size_of_port_];            // on initialise la taille d'un string pour le pseudo
    strcpy(port_,argv[3]);
    long int value_port = strtol(port_, NULL, 10);
    long int port=value_port;

    // printf("ip serveur : %s \n",ip_server);
    // printf("port : %ld \n",*&port);
    // printf("pseudo : %s \n",pseudo);              //TODO convertir en char*

	signal(SIGINT, catch_ctrl_c_and_exit);   //TODO signal CTRL+D

	// TODO name = argv[1] "pseudo"
  str_trim_lf(pseudo, strlen(pseudo));


	if (strlen(pseudo) > 32 || strlen(pseudo) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}
	printf("welcome : %s", pseudo);

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip_server);
  server_addr.sin_port = htons(port);


  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, pseudo, 32, 0);   // send pseudo to server

	printf("=== WELCOME TO THE CHATROOM ===\n");   // optionnel

	pthread_t send_msg_thread;

  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}