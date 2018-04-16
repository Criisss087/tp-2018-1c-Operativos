/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*#include <stdio.h>
#include <stdlib.h>

int main(void) {
	puts(""); // prints
	return EXIT_SUCCESS;
}
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const char* HOST_NAME = "127.0.0.1";
const int PORT = 8080;

int main(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(HOST_NAME);
	printf("%s", HOST_NAME);
	direccionServidor.sin_port = htons(PORT);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(cliente, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	}

	// Se envia mensaje al coordinador
	while (1) {
		char mensaje[1000];
		scanf("%s", mensaje);

		send(cliente, mensaje, strlen(mensaje), 0);
	}

	return 0;
}

