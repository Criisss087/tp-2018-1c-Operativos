/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

// offset
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

void imprimirEntrada(t_indice_entrada * entrada) {
	printf("\t| %d        | %s    |   %d    |   %d      |   %s   |\n",
			entrada->numeroEntrada, entrada->clave, entrada->tamanioValor,
			entrada->esAtomica, entrada->puntero);
}

void imprimirTablaEntradas() {
	printf("Imprimiendo tabla administrativa:\n");
	printf(
			"\t| N° entrada |     Clave     | Tamanio | Valor Atomico | Puntero |\n");
	list_iterate(l_indice_entradas, (void*) imprimirEntrada);
}

int existeClave(t_indice_entrada * entrada) {
	printf(
			"Verificando si la clave: %s ya existe en la tabla adminisitrativa...\n\n",
			claveBuscada);

	int existe = strcmp(entrada->clave, claveBuscada) == 0;
	printf("\tExiste? --> %d\n\n", existe);
	// return existe;
	return false;
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

	enviarHeader(socketCoordinador, instancia, coordinador,
	INSTANCIA_COORDINADOR_CONEXION, sizeof(t_info_instancia));

	t_info_instancia * infoInstancia = malloc(sizeof(t_info_instancia));

	memcpy(infoInstancia->nombreInstancia, NOMBRE_INSTANCIA,
			strlen(NOMBRE_INSTANCIA));

	printf("Enviando Informacion inicial de Instancia...\n");
	int resultado = send(socketCoordinador, infoInstancia,
			sizeof(t_info_instancia), 0);
	printf("\tResultado: %d\n", resultado);
	printf("\tNombre enviado: %s\n", infoInstancia->nombreInstancia);

}

int interpretarHeader(int socketCoordinador, t_content_header * header) {
	printf("Esperando mensajes:\n");
	header->proceso_origen = 0;

	// header = malloc(sizeof(t_content_header));
	int statusHeader = recv(socketCoordinador, header, sizeof(t_content_header),
			(int) NULL);

	printf(
			"\tstatus header: %d, proceso origen %d, proceso receptor: %d, operacion: %d \n",
			statusHeader, header->proceso_origen, header->proceso_receptor,
			header->operacion);

	return statusHeader;
}

t_configTablaEntradas * obtenerConfigTablaEntradas(int socketCoordinador) {

	t_configTablaEntradas * configRecibida = malloc(
			sizeof(t_configTablaEntradas));

	int statusHeader = recv(socketCoordinador, configRecibida,
			sizeof(t_configTablaEntradas), (int) NULL);

	printf(
			"################################################################\n");
	printf("Configuracion inicial de la Tabla de Entradas:\n");
	printf("\tCantidad total de entradas: %d\tTamaño de entradas:%d\n",
			configRecibida->cantTotalEntradas, configRecibida->tamanioEntradas);
	printf(
			"################################################################\n");

	return configRecibida;
}

void crearTablaEntradas(t_configTablaEntradas * config) {

	l_indice_entradas = list_create();

	int tamanio = config->cantTotalEntradas * config->tamanioEntradas;

	tablaEntradas = malloc(tamanio);

	if (tablaEntradas != NULL) {
		printf("\tAlocado memoria para tabla de entradas\n");

	} else {
		printf("\tNo se pudo alocar la memoria para tabla de entradas\n");
	}
}

t_sentencia * recibirSentencia(int socketCoordinador) {
	t_esi_operacion_sin_puntero * sentenciaPreliminarRecibida = malloc(
			sizeof(t_esi_operacion_sin_puntero));

	int statusSentenciaPreliminar = recv(socketCoordinador,
			sentenciaPreliminarRecibida, sizeof(t_esi_operacion_sin_puntero),
			(int) NULL);

	printf("Status Sentencia Preliminar: %d \n", statusSentenciaPreliminar);
	printf("Sentencia preliminar recibida: \n");
	printf("\tKeyword: %d - Clave: %s - Tamanio del Valor: %d\n",
			sentenciaPreliminarRecibida->keyword,
			sentenciaPreliminarRecibida->clave,
			sentenciaPreliminarRecibida->tam_valor);

	// TODO: Hacer verficiacion de keyword recibida (si es un SET pasar el valor, sino no)
	if (sentenciaPreliminarRecibida->keyword == SET_KEYWORD) {
	// Ahora se recibe el VALOR real de la sentencia

	char * valorRecibido = malloc(sentenciaPreliminarRecibida->tam_valor);

	int statusValorSentencia = recv(socketCoordinador, valorRecibido,
			sentenciaPreliminarRecibida->tam_valor, (int) NULL);

	printf("status header: %d \n", statusValorSentencia);
	printf("Valor de Sentencia recibido: \n");
	printf("\tValor: %s\n", valorRecibido);

	// Armado de sentencia definitiva

	t_sentencia * sentenciaRecibida = malloc(sizeof(t_sentencia));

	strcpy(sentenciaRecibida->clave, sentenciaPreliminarRecibida->clave);
	sentenciaRecibida->keyword = sentenciaPreliminarRecibida->keyword;
	sentenciaRecibida->valor = strdup(valorRecibido);

	printf(
			"Se asigna la sentencia correctamente... Lista para ser procesada...\n");
	return sentenciaRecibida;
	}

	else return sentenciaPreliminarRecibida;

}

t_indice_entrada * obtenerIndiceDeClave(t_sentencia * sentenciaRecibida) {
	strcpy(claveBuscada, sentenciaRecibida->clave);
	t_indice_entrada * indiceBuscado = list_find(l_indice_entradas,
			(void*) existeClave);

	return indiceBuscado;
}

void guardarClaveValor(t_sentencia * sentenciaRecibida) {

	// TODO: Verificar si la clave existe, sino crearla

	strcpy(claveBuscada, sentenciaRecibida->clave);

	if (existeClave(sentenciaRecibida)) {
		printf("La clave '%s' ya existe...\n", sentenciaRecibida->clave);
		// TODO: Reemplazar valor (ver logica)
	} else {
		printf("La clave no existe... guardar...\n");

		int tamanioTotalValor = strlen(sentenciaRecibida->valor);

		if (tamanioTotalValor > configTablaEntradas->tamanioEntradas) {
			// Guardar varias entradas
			printf("\tTamanio total del Valor: %d\n", tamanioTotalValor);
			printf("\tTamanio maximo a guardar por entrada: %d\n",
					configTablaEntradas->tamanioEntradas);

			int entradasNecesariasParaGuardarValor = tamanioTotalValor
					/ configTablaEntradas->tamanioEntradas;
			int resto = tamanioTotalValor
					% configTablaEntradas->tamanioEntradas;

			if (resto > 0) {
				entradasNecesariasParaGuardarValor++;
			}
			printf("\tGuardando un total de %d entradas...\n",
					entradasNecesariasParaGuardarValor);

			for (int i = 1; i <= entradasNecesariasParaGuardarValor; i++) {
				printf("Guardando %d entrada requerida\n", i);
				// guardarEntrada(sentenciaRecibida);

				t_indice_entrada * indiceEntrada = malloc(
						sizeof(t_indice_entrada));
				strcpy(indiceEntrada->clave, sentenciaRecibida->clave);

				// validar que no exceda canntidad total de entradas
				indiceEntrada->numeroEntrada = numeroEntrada;
				printf("\tGuardando entrada en indice: %d\n",
						indiceEntrada->numeroEntrada);
				numeroEntrada++;

				indiceEntrada->esAtomica = false;

				if (tamanioTotalValor > configTablaEntradas->tamanioEntradas) {
					indiceEntrada->tamanioValor =
							configTablaEntradas->tamanioEntradas;
				} else {
					indiceEntrada->tamanioValor = tamanioTotalValor;
				}
				tamanioTotalValor = tamanioTotalValor
						- indiceEntrada->tamanioValor;
				printf("\tTamanio de valor a guardar en indice: %d\n",
						indiceEntrada->tamanioValor);
				printf(
						"\t\tAun queda pendiente guardar %d del tamanio total del valor\n",
						tamanioTotalValor);

				indiceEntrada->puntero = tablaEntradas
						+ (indiceEntrada->numeroEntrada
								* configTablaEntradas->tamanioEntradas);
				printf("\tPuntero: %s\n", indiceEntrada->puntero);

				list_add(l_indice_entradas, indiceEntrada);

				printf("Indice agregado correctamente\n");

				// TODO: Buscar la forma de modularizar el guardado de unica entrada o varias entradas
				printf("Guardando valor...\n");

				memcpy(indiceEntrada->puntero, sentenciaRecibida->valor,
						strlen(sentenciaRecibida->valor));

				printf("Valor guardado: %s\n", indiceEntrada->puntero);

			}

		} else {

			printf("Agregando unica entrada\n");

			t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
			strcpy(indiceEntrada->clave, sentenciaRecibida->clave);

			// validar que no exceda canntidad total de entradas
			indiceEntrada->numeroEntrada = numeroEntrada;
			numeroEntrada++;

			indiceEntrada->esAtomica = true;
			indiceEntrada->tamanioValor = tamanioTotalValor;

			indiceEntrada->puntero = tablaEntradas
					+ (indiceEntrada->numeroEntrada
							* configTablaEntradas->tamanioEntradas);

			list_add(l_indice_entradas, indiceEntrada);

			printf("Indice agregado correctamente\n");

			printf("Guardando valor...\n");
			memcpy(indiceEntrada->puntero, sentenciaRecibida->valor,
					strlen(sentenciaRecibida->valor));

			printf("Valor guardado: %s\n", indiceEntrada->puntero);

		}

		printf(
				"TODO: Se supera la cantidad maxima de entradas definida por el coordinador. Se requiere reemplazar o compactar\n");
	}
}

void interpretarOperacionCoordinador(t_content_header * header,
		int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_CONFIG_INICIAL:

		configTablaEntradas = obtenerConfigTablaEntradas(socketCoordinador);

		crearTablaEntradas(configTablaEntradas);

		break;

	case COORDINADOR_INSTANCIA_SENTENCIA:

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		switch (sentenciaRecibida->keyword) {

		case SET_KEYWORD:
			// Imprimo tabla de entradas para verificar su estado
			imprimirTablaEntradas();
			guardarClaveValor(sentenciaRecibida);
			imprimirTablaEntradas();
			break;

		case STORE_KEYWORD:
			// grabarArchivo(sentenciaRecibida);
			break;

		case GET_KEYWORD:
			printf(
					"TODO: Leer clave y devolver valor... (ver si corresponde implementar el GET)\n");
			break;

		}

		break;
	}
}

int main(void) {

	int socketCoordinador = conexionConCoordinador();

	enviarNombreInstanciaACoordinador(socketCoordinador);

	t_content_header * header = malloc(sizeof(t_content_header));
	int status = 1;

	status = interpretarHeader(socketCoordinador, header);
	while (status != -1 && status != 0) {
		switch (header->proceso_origen) {

		// Recibir sentencias del coordinador
		case coordinador:

			interpretarOperacionCoordinador(header, socketCoordinador);
			break;

		default:
			printf(
					"El codigo de proceso no es correcto o no esta identificado.\n");
			break;

		}

		status = interpretarHeader(socketCoordinador, header);

	}

	close(socketCoordinador);

	return 0;
}

