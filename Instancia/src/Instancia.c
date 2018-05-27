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

int conexionConCoordinador() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons(PUERTO_COORDINADOR);

	int socketCoordinador = socket(AF_INET, SOCK_STREAM, 0);

	printf("Conectando con Coordinador...\n");
	if (connect(socketCoordinador, (void*) &direccionServidor,
			sizeof(direccionServidor)) != 0) {
		perror("No se pudo conectar al Coordinador\n");
		return 1;
	} else
		printf("Conectado correctamente.\n\n");
	return socketCoordinador;
}

int existeClave(t_entrada * entrada) {
	printf("TODO:Remover claver hardcodeada\n\n");
	return (int) strcmp(entrada->clave, "deportessasaf:messi") != 1;
}

void enviarHeader(int socketCoordinador, int procesoOrigen, int procesoReceptor,
		int operacion, int cantidadALeer) {
	t_content_header * header = malloc(sizeof(t_content_header));
	header->cantidad_a_leer = cantidadALeer;
	header->operacion = operacion;
	header->proceso_origen = procesoOrigen;
	header->proceso_receptor = procesoReceptor;

	printf("Enviando header..: \n");
	printf("\tProceso origen: %d \n", header->proceso_origen);
	printf("\tProceso destino: %d \n", header->proceso_receptor);
	printf("\tOperacion: %d \n", header->operacion);
	printf("\tCant a leer: %d \n", header->cantidad_a_leer);

	int resultado = send(socketCoordinador, header, sizeof(t_content_header),
			0);

	printf("Resultado de envio Header: %d \n", resultado);

}

void enviarNombreInstanciaACoordinador(int socketCoordinador) {

	printf("Envio de header para info inicial de Instancia...\n");

	enviarHeader(socketCoordinador, INSTANCIA,
	COORDINADOR, INSTANCIA_COORDINADOR_CONEXION, sizeof(t_info_instancia));

	t_info_instancia * infoInstancia = malloc(sizeof(t_info_instancia));

	memcpy(infoInstancia->nombreInstancia, NOMBRE_INSTANCIA,
			strlen(NOMBRE_INSTANCIA));

	printf("Enviando Informacion inicial de Instancia...\n");
	int resultado = send(socketCoordinador, infoInstancia,
			sizeof(t_info_instancia), 0);
	printf("\tResultado: %d\n", resultado);

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

t_config_tabla_entradas * obtenerConfigTablaEntradas(int socketCoordinador) {

	t_config_tabla_entradas * configRecibida = malloc(
			sizeof(t_config_tabla_entradas));

	int statusHeader = recv(socketCoordinador, configRecibida,
			sizeof(t_config_tabla_entradas), (int) NULL);

	printf(
			"################################################################\n");
	printf("Configuracion inicial de la Tabla de Entradas:\n");
	printf("\tCantidad total de entradas: %d\tTamaño de entradas:%d\n",
			configRecibida->cantTotalEntradas, configRecibida->tamanioEntradas);
	printf(
			"################################################################\n");

	return configRecibida;
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
	t_config_tabla_entradas * configTablaEntradas;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_CONFIG_INICIAL:

		configTablaEntradas = obtenerConfigTablaEntradas(socketCoordinador);

		l_entradas = list_create();

		break;

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

	// while (true) {
	t_content_header * header = interpretarHeader(socketCoordinador);

	switch (header->proceso_origen) {

	// Recibir sentencias del coordinador
	case COORDINADOR:

		interpretarOperacionCoordinador(header, socketCoordinador);
		break;

	default:
		printf("El codigo de proceso no es correcto o no esta identificado.\n");
		break;

	}

	// }

	return 0;
}

