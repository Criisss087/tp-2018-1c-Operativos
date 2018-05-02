/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <parsi/parser.h>

#define IP "127.0.0.1"
#define PUERTO "8080"
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(int argc, char **argv){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);

	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	// Las siguientes dos lineas sirven para no lockear el address
		int activado = 1;
		setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int con = connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al servidor: %d \n",con);

//	while(enviar){
	while(1){

		fp = fopen(argv[1], "r");
		if (fp == NULL){
		    perror("Error al abrir el archivo: ");
		    // informar a quien corresponda -- exit(EXIT_FAILURE);
		}

		while ((read = getline(&line, &len, fp)) != -1) {
		     t_esi_operacion parsed = parse(line);
			        if(parsed.valido){
			            switch(parsed.keyword){
			                case GET:
			                    printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
			                    break;
			                case SET:
			                    printf("SET\tclave: <%s>\tvalor: <%s>\n", parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
			                    break;
			                case STORE:
			                    printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
			                      break;
			                default:
			                    fprintf(stderr, "No pude interpretar <%s>\n", line);
			                    // informar a quien corresponda -- exit(EXIT_FAILURE);
			            }

			        destruir_operacion(parsed);
			        } else {
			            fprintf(stderr, "La linea <%s> no es valida\n", line);
			            // informar a quien corresponda -- exit(EXIT_FAILURE);
			        }
			    }

			    fclose(fp);
			    if (line)
			        free(line);

		// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		fgets(message, PACKAGESIZE, stdin);/*
			// Chequeo que el usuario no quiera salir
			if (!strcmp(message,"exit\n")) enviar = 0;
			// Solo envio si el usuario no quiere salir.*/
			//if (enviar)
				send(serverSocket, message, strlen(message)+1, 0);
		}

	close(serverSocket);
		return 0;
}

