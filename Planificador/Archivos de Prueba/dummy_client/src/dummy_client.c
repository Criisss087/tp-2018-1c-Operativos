/*
 ============================================================================
 Name        : dummy_client.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
    C ECHO client example using sockets
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
enum procesos {  esi, instancia, planificador, coordinador };

struct confirmacion_sentencia{
	int pid;
	int ejec_anterior;
	int resultado;
};
typedef struct confirmacion_sentencia t_confirmacion_sentencia;

int main(int argc , char *argv[])
{
    int resultado;
	int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8080 );

    int activado = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));


    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //keep communicating with server
    while(resultado<2)
    {

    	printf("Esperando turno para ejecutar... \n ");


        t_content_header *content_header = malloc(sizeof(t_content_header));

        int read_size = recv(sock, content_header, sizeof(t_content_header), (int)NULL);
        if(content_header->operacion == 1)
        {
        	printf("Es mi turno! \n"); //Chequeo la operacion por formalidad nomas
        }

		t_confirmacion_sentencia *conf = malloc(sizeof(t_confirmacion_sentencia));

		read_size = recv(sock, conf , sizeof(t_confirmacion_sentencia), 0);

		printf("Mi pid es: ¡%d! \n", conf->pid);
		printf("¿Debo ejecutar la sentencia anterior?: %d \n", conf->ejec_anterior);

		free(content_header);

        printf("Resultado de la ejecucion:");
        /* -1 block
         * 1 ok
         * 2 ok y fin
         */

        scanf("%d" , &resultado);

        conf->resultado = resultado;

        content_header = malloc(sizeof(t_content_header));

        content_header->proceso_origen = esi;
		content_header->proceso_receptor = planificador;
		content_header->operacion = 2;
		content_header->cantidad_a_leer = sizeof(t_confirmacion_sentencia);

		//Envio primero la cabecera
		if( send(sock  , content_header , sizeof(t_content_header) , 0) < 0)
		{
			puts("Send failed 1");
			return 1;
		}


        //Send some data
        if( send(sock , conf , sizeof(t_confirmacion_sentencia) , 0) < 0)
        {
            puts("Send failed 2");
            return 1;
        }

    }

    close(sock);
    return 0;
}
