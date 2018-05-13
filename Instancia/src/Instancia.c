/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include  "Utilidades.h"

int main(void) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons(PUERTO_COORDINADOR);


	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(server, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	}

	// Recibe una sentencia del coordinador

	int status_header = 1;		// Estructura que manjea el status de los receive.
	printf("Esperando mensajes:\n");

	ContentHeader * header = malloc(sizeof(ContentHeader));
	status_header = recv(server, header, sizeof(ContentHeader), NULL);

	int pt = header->proceso_tipo;
	int po = header->operacion;

	printf("status header: %d, p tipo %d, p op: %d \n", status_header, pt, po);

	if (po == CORDINADOR_ENVIA_SENTENCIA_INSTANCIA) {

		t_esi_operacion_sin_puntero * sentenciaRecibida = malloc(sizeof(t_esi_operacion_sin_puntero));

		status_header = recv(server, sentenciaRecibida, sizeof(t_esi_operacion_sin_puntero), NULL);

		printf("status header: %d \n", status_header);
		printf("sentencia recibida: \n");
		printf("\tKeyword: %i - Clave: %s - Valor: %s\n",sentenciaRecibida->keyword,sentenciaRecibida->clave,sentenciaRecibida->valor);

		if (sentenciaRecibida->keyword == SET_KEYWORD) {
			printf("TODO: Guardar clave y valor\n");

		} else if (sentenciaRecibida->keyword == GET_KEYWORD) {
			printf("TODO: Leer clave y devolver valor\n");
		}

	}

/*
	//Se envia mensaje al coordinador
		while (1) {
		char mensaje[PACKAGESIZE];
		scanf("%s", mensaje);
		mensaje[strlen(mensaje)] = '\n';
		//fgets(mensaje, PACKAGESIZE, stdin);
		//diferencia entre fgets y scanf: fgets lee hasta un \n, scanf lee y pone \0.
		send(server,mensaje,strlen(mensaje)+1,0);
		}
*/

/*
	char mensaje[PACKAGESIZE];
	scanf("%s", mensaje);
	mensaje[strlen(mensaje)] = '\n';
	
	// char* info = mensaje;

	ContentHeader * header = malloc(sizeof(ContentHeader));

	header->operacion = 0000;
	header->proceso_tipo = TYPE_INSTANCIA;
	header->cantidad_a_leer = sizeof(mensaje);

	int resultado = send(server, &header, sizeof(mensaje), 0);

	// a continuación, enviamos el contenido del paquete;
	// Si el struct VariableCustom tiene campos tipo punteros, no será tan sencillo como hacer un sizeof

	send(server, &mensaje, sizeof(mensaje), 0);
*/


	return 0;
}

