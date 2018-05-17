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

bool existeClave(t_entrada * entrada) {
				return entrada->clave == "deportes:messi";
			}

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

	// Definicion de Tabla de Entradas
	// l_entradas = crearListaEntradas();
	l_entradas = list_create();

	// Recibe una sentencia del coordinador

	int statusHeader = 1;		// Estructura que manjea el status de los receive.
	printf("Esperando mensajes:\n");

	ContentHeader * header = malloc(sizeof(ContentHeader));
	statusHeader = recv(server, header, sizeof(ContentHeader), NULL);

	int pt = header->proceso_tipo;
	int po = header->operacion;

	printf("status header: %d, p tipo %d, p op: %d \n", statusHeader, pt, po);

	if (po == CORDINADOR_ENVIA_SENTENCIA_INSTANCIA) {

		t_sentencia_sin_puntero * sentenciaRecibida = malloc(sizeof(t_sentencia_sin_puntero));

		statusHeader = recv(server, sentenciaRecibida, sizeof(t_sentencia_sin_puntero), NULL);

		printf("status header: %d \n", statusHeader);
		printf("sentencia recibida: \n");
		printf("\tKeyword: %i - Clave: %s - Valor: %s\n",sentenciaRecibida->keyword,sentenciaRecibida->clave,sentenciaRecibida->valor);

		if (sentenciaRecibida->keyword == SET_KEYWORD) {
			printf("El tamaño de la lista de entradas es: %d\n", list_size(l_entradas));

			printf("TODO: Guardar valor en el Storage con mmap()\n");

			if (list_size(l_entradas) <= CANT_MAX_ENTRADAS) {

				t_entrada * entrada;

				entrada = malloc(sizeof(t_entrada));

				strcpy(entrada->clave, sentenciaRecibida->clave);
				entrada->numeroEntrada = 0;
				entrada->tamanioEntrada = sizeof(t_sentencia_sin_puntero);

				list_add(l_entradas , entrada);

				printf("La clave guardada es %s\n",entrada->clave);

				printf("El tamaño de la lista de entradas es: %d\n", list_size(l_entradas));

				t_entrada * entradaBuscada; // = list_find(l_entradas, (void*)existeClave);

				entradaBuscada = malloc(sizeof(t_entrada));

				printf("Recibido: %s\n", list_find(l_entradas, (void*)existeClave));
				printf("La clave es: %s, el Numero de Entrada: %d, el tamaño de entrada: %d\n", entradaBuscada->clave, entradaBuscada->numeroEntrada, entradaBuscada->tamanioEntrada);

				free(entrada);

				free(entradaBuscada);

			} else {
				printf("TODO: Se supera la cantidad maxima de entradas definida por el coordinador");
			}

		} else if (sentenciaRecibida->keyword == GET_KEYWORD) {
			printf("TODO: Leer clave y devolver valor\n");

			// t_entrada * entradaBuscada = list_find(l_entradas, (void*)existeClave);

			// printf("La clave es: %s, el Numero de Entrada: %d, el tamaño de entrada: %d", entradaBuscada->clave, entradaBuscada->numeroEntrada, entradaBuscada->tamanioEntrada);
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

