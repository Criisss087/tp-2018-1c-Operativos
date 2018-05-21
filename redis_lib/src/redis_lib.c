/*
 ============================================================================
 Name        : redis_lib.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "redis_lib.h"

void test(void){
	printf("\n\nEstoy usando la biblioteca compartida!\n\n");
	return;
}

int crear_listen_socket( char * puerto, int max_conexiones )
{
	struct sockaddr_in dir_sock;

	//Convierto el string a INT para htons
	unsigned int puerto_i = atoi(puerto);

	dir_sock.sin_family = AF_INET;
	dir_sock.sin_addr.s_addr = INADDR_ANY;
	dir_sock.sin_port = htons(puerto_i);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		return -1;
	}

	int activado = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(server_socket, (void*)&dir_sock, sizeof(dir_sock)) != 0)
	{
		return -1;
	}


	listen(server_socket, max_conexiones);

	return server_socket;

}

int conectar_a_server(char* ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;	 // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &hints, &server_info); // Carga en server_info los datos de la conexion


	int new_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (new_socket < 0)
	{
		return -1;
	}

	int activado = 1;
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int res_connect = connect(new_socket, server_info->ai_addr, server_info->ai_addrlen);
	if (res_connect < 0)
	{
		freeaddrinfo(server_info);
		close(new_socket);
		return -1;
	}

	freeaddrinfo(server_info);
	return new_socket;

}
