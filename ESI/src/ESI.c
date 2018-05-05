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
#define HEADER_LENGTH 10

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


typedef struct {

int proceso_tipo;

int operacion;

int cantidad_a_leer;

} __attribute__((packed)) ContentHeader;



int main(int argc, char **argv){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	struct addrinfo *serverInfoCoord = crear_addrinfo();
	struct addrinfo *serverInfoPlanif = crear_addrinfo();

	int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);
	int serverPlanif = socket(serverInfoPlanif->ai_family, serverInfoPlanif->ai_socktype, serverInfoPlanif->ai_protocol);


	/* Las siguientes dos lineas sirven para no lockear el address
		int activado = 1;
		setsockopt(serverSocket1, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));*/

	int coord = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);
	int planif = connect(serverPlanif, serverInfoPlanif->ai_addr, serverInfoPlanif->ai_addrlen);

	freeaddrinfo(serverInfoCoord);
	freeaddrinfo(serverInfoPlanif);

	int enviar = 1;

	printf("Conectado al servidor: %d \n",coord);
	printf("Conectado al servidor: %d \n",planif);

	///	while(enviar){
	while(1){

		fp = fopen(argv[1], "r");

		char package[PACKAGESIZE];
		char rtaCoord[PACKAGESIZE];
		char ordenDeLectura[PACKAGESIZE];

		int status = 1;

		while (status != 0){
			//recibo otden del planif

			ContentHeader * header_a_ESI_de_planif = malloc(sizeof(ContentHeader));
			recv(serverPlanif, &header_a_ESI_de_planif, sizeof(ContentHeader),0);

			if(header_a_ESI_de_planif->operacion == 3101){
				status = recv(serverPlanif, ordenDeLectura, sizeof(ordenDeLectura),0);
			}

		    if (status != 0) {
		    		if ((read = getline(&line, &len, fp)) != -1) {
		            t_esi_operacion parsed = parse(line);

		            if(parsed.valido){

		            	   //le envio al coordinador la linea parseada
                			ContentHeader * header_a_coord_de_ESI = malloc(sizeof(ContentHeader));
                 			header_a_coord_de_ESI.cantidad_a_leer->sizeof(parsed);
                			header_a_coord_de_ESI.operacion->1401;
                			header_a_coord_de_ESI.proceso_tipo->1;
                			int resultado = send(serverCoord, &header_a_coord_de_ESI, sizeof(ContentHeader), 0);
                			send(serverCoord, parsed, sizeof(parsed),0);

                			//recibo la rta del coord
                			ContentHeader * header_a_ESI_de_coord = malloc(sizeof(ContentHeader));
                		    recv(serverCoord, &header_a_ESI_de_coord, sizeof(ContentHeader),0);
                			if(header_a_ESI_de_coord->operacion == 4102){
                				recv(serverCoord, rtaCoord, sizeof(rtaCoord),0);
                			}

                			//envio al planif lo que me mando el coord
                			ContentHeader * header_a_planif_de_ESI = malloc(sizeof(ContentHeader));
                			send(serverPlanif, &header_a_planif_de_ESI, sizeof(ContentHeader),0);
                			if(header_a_planif_de_ESI->operacion == 1302){
                			   send(serverPlanif, rtaCoord, sizeof(rtaCoord),0);
                			}

			            }

			            destruir_operacion(parsed);
			        }
			    }
		}

			    fclose(fp);
			    if (line)
			        free(line);

	close(serverCoord);
	close(serverPlanif);
		return 0;
}

