all: client serveur

client: client.c 
	gcc -o client client.c 

server: serveur.c 
	gcc -o serveur serveur.c 
