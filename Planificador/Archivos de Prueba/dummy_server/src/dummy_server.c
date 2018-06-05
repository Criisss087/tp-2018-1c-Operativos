/*
 ============================================================================
 Name        : dummy_server.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
    C socket server example
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write

#include <stdlib.h>
#include <sys/socket.h>		// Para crear los sockets
#include <sys/types.h>
#include <sys/time.h>		//timeval en select
#include <netdb.h> 			// Para getaddrinfo

struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;
enum procesos { esi, instancia, planificador, coordinador };

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    char message[1000];
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    int activado = 1;
    setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));


    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    /*
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        write(client_sock , client_message , strlen(client_message));

    }
     */

    while(1)
    {
    	printf("Enter message : ");
    	scanf("%s" , message);

    	t_content_header *content_header = malloc(sizeof(t_content_header));

    	content_header->proceso_origen = coordinador;
    	content_header->proceso_receptor = planificador;
    	content_header->operacion = 1;
    	content_header->cantidad_a_leer = sizeof(message);

		//Envio primero la cabecera
		if( send(client_sock  , content_header , sizeof(t_content_header) , 0) < 0)
		{
			puts("Send failed 1");
			return 1;
		}

		if( send(client_sock  , message , strlen(message) , 0) < 0)
		{
			puts("Send failed 2");
			return 1;
		}

		printf("sending message : %s ",message);

		/*
		//Receive a reply from the server
		if( recv(client_sock  , client_message, 2000 , 0) < 0)
		{
			puts("recv failed");
			break;
		}

		puts("Server reply :");
		puts(client_message);
		*/

    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    return 0;
}
