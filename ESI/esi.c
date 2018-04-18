/*
 * esi.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */


#include "../Utilidades.h"

int main() {

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons(PUERTO);

	Readline("asdf");
	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	}

	while (1) {
		char * mensaje = Readline("Ingrese mensaje:");

		send(cliente, &mensaje, strlen(mensaje), 0);
	}

	return 0;
}
}
