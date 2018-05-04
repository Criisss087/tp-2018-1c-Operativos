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
#define PACKAGESIZE 1024
#define RESPUESTA 1024

/*#define IP "127.0.0.1"
#define PUERTO "8080"*/ //LO comento porque necesito que se conecte a 2 servidores, no solo el coordinador

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);

	return serverInfo;
}

int main(int argc, char **argv){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	struct addrinfo *serverInfo1 = crear_addrinfo();
	struct addrinfo *serverInfo2 = crear_addrinfo();

	int serverSocket1 = socket(serverInfo1->ai_family, serverInfo1->ai_socktype, serverInfo1->ai_protocol); //coordinador
	int serverSocket2 = socket(serverInfo2->ai_family, serverInfo2->ai_socktype, serverInfo2->ai_protocol); //planificador


	/* Las siguientes dos lineas sirven para no lockear el address
		int activado = 1;
		setsockopt(serverSocket1, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));*/

	int coord = connect(serverSocket1, serverInfo1->ai_addr, serverInfo1->ai_addrlen);
	int planif = connect(serverSocket2, serverInfo2->ai_addr, serverInfo2->ai_addrlen);

	freeaddrinfo(serverInfo1);
	freeaddrinfo(serverInfo2);

	int enviar = 1;

	printf("Conectado al servidor: %d \n",coord);
	printf("Conectado al servidor: %d \n",planif);

	char message1[] = "Error al abrir el archivo";

//	while(enviar){
	while(1){

		fp = fopen(argv[1], "r");
		if (fp == NULL){
		    send(serverSocket2, message1, strlen(message1)+1, 0);// informar al planificador -- exit(EXIT_FAILURE);
		}

		char package[PACKAGESIZE];
		char respuestaDelGet[RESPUESTA];
		char respuestaDelSet[RESPUESTA];
		char respuestaDelStore[RESPUESTA];

		int status = 1;

		while (status != 0){
		    	status = recv(serverSocket2, (void*) package, PACKAGESIZE, 0);//el planificador envia la orden(package)
		    	if (status != 0) {
		    		if ((read = getline(&line, &len, fp)) != -1) {
		            t_esi_operacion parsed = parse(line);
			        if(parsed.valido){
			            switch(parsed.keyword){
			                case GET:
			                    send(serverSocket1,parsed.argumentos.GET.clave, strlen(parsed.argumentos.GET.clave)+1,0); //le envia al coordinador la clave
			                    recv(serverSocket1, respuestaDelGet, RESPUESTA,0);//espera la rta del coordinador
			                    send(serverSocket2, respuestaDelGet, RESPUESTA,0);//cuando le llega la respuesta se la envia al planificador
			                    break;
			                case SET:
			                	send(serverSocket1,parsed.argumentos.SET.clave, strlen(parsed.argumentos.SET.clave)+1,0);//idem
			                	send(serverSocket1,parsed.argumentos.SET.valor, strlen(parsed.argumentos.SET.valor)+1,0);
			                	recv(serverSocket1, respuestaDelSet, RESPUESTA,0);//espera la rta del coordinador
			                    send(serverSocket2, respuestaDelSet, RESPUESTA,0);//le envia la rta del coordinador al planificador...
			                    break;
			                case STORE:
			                	send(serverSocket1,parsed.argumentos.STORE.clave, strlen(parsed.argumentos.STORE.clave)+1,0);
			                	recv(serverSocket1, respuestaDelStore, RESPUESTA,0);
			                	send(serverSocket2, respuestaDelStore, RESPUESTA,0);
			                    break;
			                default:
			                	send(serverSocket2, message2, strlen(message2)+1, 0);// informar al planificador
			            }
			            destruir_operacion(parsed);
			        }
		    	    }
			    }
		}

			    fclose(fp);
			    if (line)
			        free(line);

		}



	close(serverSocket1);
	close(serverSocket2);
		return 0;
}

