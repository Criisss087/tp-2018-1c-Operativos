/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include "Utilidades.h"

struct addrinfo* crear_addrinfo(ip, puerto){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	return serverInfo;
}


int main(int argc, char **argv){
	//proof of concept - send header
	struct addrinfo *serverInfoCoord = crear_addrinfo(IP, PUERTO);
	int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);

	int activado = 1;
	setsockopt(serverCoord, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int coord = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);
	freeaddrinfo(serverInfoCoord);
	printf("Conectado al servidor: %d \n",coord);
	printf("sizeof header %d \n", sizeof(ContentHeader));

	t_esi_operacion_sin_puntero * t = malloc(sizeof(t_esi_operacion_sin_puntero));
	char *c = "GET";
	int k = 0;

	memcpy(t->clave,c, strlen(c));
	t->clave[3] = '\0';

   //le envio al coordinador la linea parseada
	ContentHeader * header_a_coord_de_ESI = malloc(sizeof(ContentHeader));
	header_a_coord_de_ESI->cantidad_a_leer = sizeof(t_esi_operacion_sin_puntero);
	header_a_coord_de_ESI->operacion = 1401;
	header_a_coord_de_ESI->proceso_tipo = 1;
	printf("mandando header..: \n");
	printf("op %d \n",header_a_coord_de_ESI->operacion);
	printf("p tipo: %d \n",header_a_coord_de_ESI->proceso_tipo);
	printf("cant: %d \n",header_a_coord_de_ESI->cantidad_a_leer);

	int resultado = send(serverCoord, header_a_coord_de_ESI, sizeof(ContentHeader), 0);

	printf("header: %d \n",resultado);

	printf("mandando sentencia..: \n");
	resultado = send(serverCoord, t, sizeof(t_esi_operacion_sin_puntero),0);
	printf("sentencia: %d \n",resultado);

	printf("closing..\n");
	close(serverCoord);
	return 0;
}

