/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PUERTO 8080
#define PACKAGESIZE 1024

#define TYPE_INSTANCIA 2

typedef struct {
	int proceso_tipo;
	int operacion;
	int cantidad_a_leer;
	} __attribute__((packed)) ContentHeader;

int main(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PUERTO);


	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(server, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	}

	//Se envia mensaje al coordinador
	/*	while (1) {
		char mensaje[PACKAGESIZE];
		scanf("%s", mensaje);
		mensaje[strlen(mensaje)] = '\n';
		//fgets(mensaje, PACKAGESIZE, stdin);
		//diferencia entre fgets y scanf: fgets lee hasta un \n, scanf lee y pone \0.
		send(server,mensaje,strlen(mensaje)+1,0);
		}
	*/

	char mensaje[PACKAGESIZE];
	scanf("%s", mensaje);
	mensaje[strlen(mensaje)] = '\n';
	
	VariableCustom info = mensaje;

	ContentHeader * header = malloc(sizeof(ContentHeader));

	header.operacion->0000;
	header.proceso_tipo->TYPE_INSTANCIA;
	header.cantidad_a_leer->sizeof(VariableCustom);

	int resultado = send(server, &header, sizeof(ContentHeader), 0);

	// a continuación, enviamos el contenido del paquete;
	// Si el struct VariableCustom tiene campos tipo punteros, no será tan sencillo como hacer un sizeof

	send(server, &info, sizeof(VariableCustom), 0));

	return 0;
}

