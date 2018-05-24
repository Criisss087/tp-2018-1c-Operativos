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

void enviarNombreInstanciaACoordinador(int coordinador) {
	t_content_header * header_a_coord_de_instancia = malloc(
			sizeof(t_content_header));
	header_a_coord_de_instancia->cantidad_a_leer = sizeof(t_info_instancia);
	header_a_coord_de_instancia->operacion = INSTANCIA_COORDINADOR_CONEXION;
	header_a_coord_de_instancia->proceso_origen = INSTANCIA;
	header_a_coord_de_instancia->proceso_receptor = COORDINADOR;

	printf("Enviando header..: \n");
	printf("Operacion: %d \n", header_a_coord_de_instancia->operacion);
	printf("Proceso origen: %d \n", header_a_coord_de_instancia->proceso_origen);
	printf("Proceso destino: %d \n", header_a_coord_de_instancia->proceso_receptor);
	printf("Cant a leer: %d \n", header_a_coord_de_instancia->cantidad_a_leer);

	int resultado = send(coordinador, header_a_coord_de_instancia,
			sizeof(t_content_header), 0);

	printf("header: %d \n", resultado);

	// Envio de Informacion inicial de Instancia (nombre)

	t_info_instancia * infoInstancia = malloc(
					sizeof(t_info_instancia));

	// t_info_instancia infoInstancia;
	memcpy(infoInstancia->nombreInstancia, NOMBRE_INSTANCIA, strlen(NOMBRE_INSTANCIA));

	printf("Enviando Informacion inicial de Instancia..: \n");
	resultado = send(coordinador, infoInstancia, sizeof(t_info_instancia), 0);
	printf("Resultado de envio: %d \n", resultado);

}

int conexionConCoordinador() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons(PUERTO_COORDINADOR);

	int socketCoordinador = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socketCoordinador, (void*) &direccionServidor,
			sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador");
		return 1;
	} else
		return socketCoordinador;
}

t_content_header * interpretarHeader(int socketCoordinador) {
	printf("Esperando mensajes:\n");

	t_content_header * header = malloc(sizeof(t_content_header));
	int statusHeader = recv(socketCoordinador, header, sizeof(t_content_header),
			(int) NULL);

	printf(
			"\tstatus header: %d, proceso origen %d, proceso receptor: %d, operacion: %d \n",
			statusHeader, header->proceso_origen, header->proceso_receptor,
			header->operacion);

	return header;
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

t_sentencia_sin_puntero * recibirSentencia(int socketCoordinador) {
	t_sentencia_sin_puntero * sentenciaRecibida = malloc(
			sizeof(t_sentencia_sin_puntero));

	int statusHeader = recv(socketCoordinador, sentenciaRecibida,
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

void interpretarOperacionCoordinador(t_content_header * header,
		int socketCoordinador) {
	t_sentencia_sin_puntero * sentenciaRecibida;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_SENTENCIA:

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		if (sentenciaRecibida->keyword == SET_KEYWORD) {

			guardarEntrada(sentenciaRecibida);

		} else if (sentenciaRecibida->keyword == GET_KEYWORD) {
			printf("TODO: Leer clave y devolver valor\n");

			// leerEntrada(sentenciaRecibida);

		}

		break;
	}
}

int main(void) {

	int socketCoordinador = conexionConCoordinador();

	enviarNombreInstanciaACoordinador(socketCoordinador);

	configTablaEntradas = obtenerConfigTablaEntradas();

	// Definicion de Tabla de Entradas
	l_entradas = list_create();

	// Recibe una sentencia del coordinador

	t_content_header * header = interpretarHeader(socketCoordinador);

	switch (header->proceso_origen) {

	case COORDINADOR:

		interpretarOperacionCoordinador(header, socketCoordinador);
		break;

	default:
		// TODO: No se reconoce el proceso
		break;

	}

	return 0;
}

