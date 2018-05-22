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

int existeClave(t_entrada * entrada) {
	printf("TODO:Remover claver hardcodeada\n\n");
	return (int) strcmp(entrada->clave, "deportessasaf:messi") != 1;
}

int conexionConCoordinador() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons(PUERTO_COORDINADOR);

	int server = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(server, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	} else
		return server;
}

int recibirOperacion(int server) {
	int statusHeader = 1;	// Estructura que manjea el status de los receive.
	printf("Esperando mensajes:\n");

	ContentHeader * header = malloc(sizeof(ContentHeader));
	statusHeader = recv(server, header, sizeof(ContentHeader), (int) NULL);

	int pt = header->proceso_tipo;
	int po = header->operacion;

	printf("status header: %d, p tipo %d, p op: %d \n", statusHeader, pt, po);

	return po;

}

t_configTablaEntradas * obtenerConfigTablaEntradas() {

	t_configTablaEntradas * config;

	config = malloc(sizeof(t_configTablaEntradas));

	config->cantTotalEntradas = CANT_MAX_ENTRADAS;
	config->tamanioEntradas = TAMANIO_ENTRADAS;

	printf(
			"################################################################\n");
	printf("Configuracion inicial de la Tabla de Entradas:\n");
	printf("\tCantidad total de entradas: %d\tTamaño de entradas:%d\n",
			config->cantTotalEntradas, config->tamanioEntradas);
	printf(
			"################################################################\n");

	return config;
}

t_sentencia_sin_puntero * recibirSentencia(int server) {
	t_sentencia_sin_puntero * sentenciaRecibida = malloc(
			sizeof(t_sentencia_sin_puntero));

	int statusHeader = recv(server, sentenciaRecibida,
			sizeof(t_sentencia_sin_puntero), (int) NULL);

	printf("status header: %d \n", statusHeader);
	printf("sentencia recibida: \n");
	printf("\tKeyword: %i - Clave: %s - Valor: %s\n",
			sentenciaRecibida->keyword, sentenciaRecibida->clave,
			sentenciaRecibida->valor);

	return sentenciaRecibida;
}

void guardarEntrada(t_sentencia_sin_puntero * sentenciaRecibida) {
	printf("TODO: Guardar valor en el Storage con mmap()\n");

	if (list_size(l_entradas) < configTablaEntradas->cantTotalEntradas) {

		printf("El tamaño de la lista de entradas es: %d\n",
				list_size(l_entradas));
		printf("\nGuardar Entrada:\n");
		t_entrada * entrada;

		entrada = malloc(sizeof(t_entrada));

		strcpy(entrada->clave, sentenciaRecibida->clave);
		entrada->numeroEntrada = 0;
		entrada->tamanioEntrada = sizeof(t_sentencia_sin_puntero);

		printf("La clave guardada es %s\n", entrada->clave);

		list_add(l_entradas, entrada);

		printf("El tamaño de la lista de entradas es: %d\n",
				list_size(l_entradas));
		// t_entrada * valorBuscado = buscarEntrada();

		//////////////////////////////////////////////////////

		printf("\n\nConsulta de valor:\n");

		t_entrada * entradaBuscada = malloc(sizeof(t_entrada));

		entradaBuscada = list_find(l_entradas, (void*) existeClave);

		printf(
				"La clave es: %s, el Numero de Entrada: %d, el tamaño de entrada: %d\n",
				entradaBuscada->clave, entradaBuscada->numeroEntrada,
				entradaBuscada->tamanioEntrada);

	} else {
		printf(
				"TODO: Se supera la cantidad maxima de entradas definida por el coordinador");
	}

}

/*t_entrada * buscarEntrada() {
 printf("\n\nConsulta de valor:\n");

 t_entrada * entradaBuscada = malloc(sizeof(t_entrada));

 entradaBuscada = list_find(l_entradas, (void*) existeClave);

 printf(
 "La clave es: %s, el Numero de Entrada: %d, el tamaño de entrada: %d\n",
 entradaBuscada->clave, entradaBuscada->numeroEntrada,
 entradaBuscada->tamanioEntrada);

 return entradaBuscada;

 }*/

int main(void) {

	int server = conexionConCoordinador();

	configTablaEntradas = obtenerConfigTablaEntradas();

	// Definicion de Tabla de Entradas
	l_entradas = list_create();

	// Recibe una sentencia del coordinador

	int po = recibirOperacion(server);

	if (po == COORDINADOR_ENVIA_SENTENCIA_INSTANCIA) {

		t_sentencia_sin_puntero * sentenciaRecibida = recibirSentencia(server);

		if (sentenciaRecibida->keyword == SET_KEYWORD) {

			guardarEntrada(sentenciaRecibida);

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

