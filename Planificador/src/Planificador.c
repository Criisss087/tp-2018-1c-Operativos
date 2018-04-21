/*
 ============================================================================
 Name        : Planificador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>

#define IP "127.0.0.1"
#define PORT "8080"

int main(void) {

	int server_socket;
	int familiaProt = PF_INET;
	int tipo_socket = SOCK_STREAM;
	int protocolo = 0; //ROTO_TCP;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PORT, &hints, &server_info); // Carga en server_info los datos de la conexion

	// TODO Hacerlo cliente
	server_socket = (server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (server_socket != -1)
		printf("CREO EL SOCKET");
	else
		printf("NOPS");


	int respuesta = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);
	freeaddrinfo(server_info);

	if (respuesta < 0) {
	    _exit_with_error(server_socket, "No me pude conectar al servidor", NULL);
	  }


	close(socket);


	return EXIT_SUCCESS;
}

	// TODO Armar la consola
