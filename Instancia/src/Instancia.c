/*
 ============================================================================
 Name        : Instancia.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Instancia.h"
#include "FuncionesInstancia.c"

int conexionConCoordinador() {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP_COORDINADOR);
	direccionServidor.sin_port = htons((int) PUERTO_COORDINADOR);

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

void enviarResultadoSentencia(int socketCoordinador, int keyword) {
	printf("Envio de header para respuesta de sentencia...\n");

	int resultado;
	int resultadoEjecucion;
	int * r;

	switch (keyword) {
	case SET_:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

		printf("Enviando Respuesta de sentencia SET...\n");

		resultadoEjecucion = EXITO_I;

		r = &resultadoEjecucion;

		resultado = send(socketCoordinador, r, sizeof(int), 0);

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n", *r);
		break;

	case STORE_:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

		printf("Enviando Respuesta de sentencia STORE...\n");

		resultadoEjecucion = ERROR_I;

		r = &resultadoEjecucion;

		resultado = send(socketCoordinador, r, sizeof(int), 0);

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n", resultadoEjecucion);
		break;

	default:
		break;
	}
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

	int tamanioTotal = config->cantTotalEntradas * config->tamanioEntradas;

	tablaEntradas = malloc(tamanioTotal);

	if (tablaEntradas != NULL) {
		printf("\tAlocado memoria para guardada de Valores\n");

	} else {
		printf("\tNo se pudo alocar la memoria para el guardado de Valores\n");
	}
}

t_sentencia * recibirSentencia(int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

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

	if (sentenciaPreliminarRecibida->keyword == SET_) {
		// Se obtiene el valor y se forma la sentencia completa
		char * valorRecibido = recibirValor(socketCoordinador,
				sentenciaPreliminarRecibida->tam_valor);
		sentenciaRecibida = construirSentenciaConValor(
				sentenciaPreliminarRecibida, valorRecibido);

		//return sentenciaRecibida;
	}

	else {

		sentenciaRecibida = construirSentenciaConValor(sentenciaPreliminarRecibida, NULL);
		//return sentenciaRecibida;
	}

	return sentenciaRecibida;
}

t_list * obtenerIndicesDeClave(char clave[40]) {
	printf("Obteniendo indices que contienen la clave: %s\n", clave);
	_Bool existeClave(t_indice_entrada * entrada) {
		return (strcmp(entrada->clave, clave) == 0);
	}

	return (list_filter(l_indice_entradas, (void*) existeClave));
}

int cantidadDeEntradasRequeridasParaValor(int tamanioTotalValor) {
	int entradasNecesariasParaGuardarValor = tamanioTotalValor
			/ configTablaEntradas->tamanioEntradas;
	int resto = tamanioTotalValor % configTablaEntradas->tamanioEntradas;

	if (resto > 0) {
		entradasNecesariasParaGuardarValor++;
	}

	return entradasNecesariasParaGuardarValor;
}

t_indice_entrada * guardarIndiceAtomicoEnTabla(t_sentencia * sentenciaRecibida) {

	t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
	strcpy(indiceEntrada->clave, sentenciaRecibida->clave);

	// verificar si el indice ya contiene valor, eliminar todas las entradas asociadas
	if (entradaExistenteEnIndice(numeroEntrada)) {
		// TODO en lugar de puntero a char debe ser un char[40]
		char * clave = obtenerClaveExistenteEnEntrada(numeroEntrada);
		eliminarEntradasAsociadasAClave(clave);
	}

	indiceEntrada->numeroEntrada = numeroEntrada;
	numeroEntrada++;

	indiceEntrada->esAtomica = true;
	indiceEntrada->tamanioValor = strlen(sentenciaRecibida->valor);

	indiceEntrada->puntero = tablaEntradas
			+ (indiceEntrada->numeroEntrada
					* configTablaEntradas->tamanioEntradas);

	list_add(l_indice_entradas, indiceEntrada);

	printf("Indice agregado correctamente\n");

	return indiceEntrada;
}

void guardarValorEnEntrada(char * valor, char* puntero) {
	printf("Guardando valor: %s en puntero: %p...\n", valor, puntero);
	memcpy(puntero, valor, strlen(valor));

	printf("Valor guardado: %s\n", puntero);
}

void aplicarAlgoritmoDeReemplazo(t_sentencia * sentenciaRecibida) {
	int algoritmo = circ;
	printf("Aplicando algoritmo de reemplazo: %d", algoritmo);
	switch (algoritmo) {
	case circ:
		numeroEntrada = 0;
		printf("El puntero fue posicionado en la entrada: %d", numeroEntrada);
		break;
	default:
		break;
	}
}

void guardarClaveValor(t_sentencia * sentenciaRecibida) {

// TODO: Si no existe creo el archivo asociado a una CLAVE (vacio) para guardar los VALORES cuando sea necesario
// char * punteroArchivo = crearArchivoParaClave(sentenciaRecibida->clave);

	t_list * indicesQueContienenClave = obtenerIndicesDeClave(
			sentenciaRecibida->clave);
	int cantidadDeIndices = list_size(indicesQueContienenClave);
	printf("Cantidad de indices que contienen clave: %d\n", cantidadDeIndices);

	if (cantidadDeIndices > 0) {
		printf("La clave '%s' ya existe...\n", sentenciaRecibida->clave);
		// TODO: Reemplazar valor (ver logica)
	} else {
		printf("La clave no existe... guardar...\n");

		int tamanioTotalValor = strlen(sentenciaRecibida->valor);

		int entradasNecesariasParaGuardarValor =
				cantidadDeEntradasRequeridasParaValor(tamanioTotalValor);

		if (entradasNecesariasParaGuardarValor > 1) {
			// Guardar varias entradas

			if (numeroEntrada >= configTablaEntradas->cantTotalEntradas) {
				printf("No hay mas lugar para guardar un valor NO atomico");
			}

			printf("\tTamanio total del Valor: %d\n", tamanioTotalValor);
			printf("\tTamanio maximo a guardar por entrada: %d\n",
					configTablaEntradas->tamanioEntradas);

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
				printf("\tPuntero: %p\n", indiceEntrada->puntero);

				// indiceEntrada->punteroArchivo = punteroArchivo;
				// printf("\tPuntero de archivo: %p", punteroArchivo);

				list_add(l_indice_entradas, indiceEntrada);

				printf("Indice agregado correctamente\n");

				// guardarValorEnEntrada(sentenciaRecibida->valor,	indiceEntrada->puntero);

				printf("Guardando valor: %s en puntero: %p...\n",
						sentenciaRecibida->valor, indiceEntrada->puntero);
				memcpy(indiceEntrada->puntero, sentenciaRecibida->valor,
						strlen(sentenciaRecibida->valor));

				printf("Valor guardado: %s\n", indiceEntrada->puntero);
			}

		} else {

			printf("Agregando unica entrada, es decir valor atomico\n");

			if (numeroEntrada >= configTablaEntradas->cantTotalEntradas) {
				printf("Es necesario aplicar algoritmo de reemplazo...\n");
				aplicarAlgoritmoDeReemplazo(sentenciaRecibida);
			}

			printf("No es necesario aplicar algoritmo de reemplazo\n");

			t_indice_entrada * indiceEntrada = guardarIndiceAtomicoEnTabla(
					sentenciaRecibida);

			// guardarValorEnEntrada(sentenciaRecibida->valor,indiceEntrada->puntero);

			printf("Guardando valor: %s en puntero: %p...\n",
					sentenciaRecibida->valor, indiceEntrada->puntero);
			memcpy(indiceEntrada->puntero, sentenciaRecibida->valor,
					strlen(sentenciaRecibida->valor));

			printf("Valor guardado: %s\n", indiceEntrada->puntero);
			imprimirTablaEntradas();
		}

		printf(
				"TODO: Se supera la cantidad maxima de entradas definida por el coordinador. Se requiere reemplazar o compactar\n");
	}
}

void make_directory(const char* name) {
	//char * check;
	int check;
	check = mkdir(name, 0777);

	if (!check)
		printf("Directorio creado: %s\t check = %d\n", name,check);

	else {
		printf("No se puede crear el directorio... o ya existe...\t check = %d\n",check);
	}
}

char * obtenerPathArchivo(char clave[40]) {
	char * puntoDeMontaje = strdup(PUNTO_DE_MONTAJE);
	printf("Directorio donde se guardara el archivo: %s\n", puntoDeMontaje);

	char * archivo;
	archivo = string_new();
	string_append(&archivo, clave);
	string_append(&archivo, ".txt");
	printf("Nombre del archivo que se creara: %s\n", archivo);

	char * path = string_new();
	string_append(&path, puntoDeMontaje);
	string_append(&path, archivo);
	printf("Path del archivo a crear: %s\n", path);

	return path;
}

size_t getFileSize(const char* filename) {
	struct stat st;
	stat(filename, &st);
	return st.st_size;
}

// &len
void * append(int fd, char * valor, size_t nbytes, void *map, size_t len) {
	int permisos = PROT_READ | PROT_WRITE;
	ssize_t written = write(fd, valor, nbytes);
	munmap(map, len);
	len += written;
	return mmap(NULL, len, permisos, MAP_SHARED, fd, 0);
}

void grabarArchivoPorPrimeraVez(int fd, char * valor, int tamanio) {
	write(fd, valor, tamanio);
}

void guardarPunteroDeArchivoEnIndices(t_list * indices, char * punteroDeArchivo) {
	void guardarPunteroDeArchivo(t_indice_entrada * entrada) {
		strcpy(entrada->punteroArchivo, punteroDeArchivo);
	}
	list_iterate(indices, (void *) guardarPunteroDeArchivo);
}

void realizarStoreDeClave(char clave[40]) {
	char * path = obtenerPathArchivo(clave);

	make_directory(PUNTO_DE_MONTAJE);

	int permisos = 0664;
	int flags = O_RDWR | O_CREAT | O_TRUNC;

	int fileDescriptor;
	if ((fileDescriptor = open(path, flags, permisos)) < 0) {
		printf("No se pudo crear el archivo: %s\n", path);
		printf("\tFile Descriptor: %d\n", fileDescriptor);
	} else {
		printf("File Descriptor: %d\n", fileDescriptor);
	}

	t_list * indices = obtenerIndicesDeClave(clave);

	t_indice_entrada * primerEntrada = list_get(indices, 0);

	int tamanioValorCompleto = obtenerTamanioTotalDeValorGuardado(indices);
	printf("Tamaño del valor guardado: %d\n", tamanioValorCompleto);

	char * valorCompleto = obtenerValorCompleto(primerEntrada->puntero,
			tamanioValorCompleto);
	printf("El valor completo es: %s\n", valorCompleto);

	if (primerEntrada->punteroArchivo == NULL) {
		grabarArchivoPorPrimeraVez(fileDescriptor, valorCompleto, tamanioValorCompleto);
	}

	char * punteroDeArchivo;

	//  ACA APLICAR EL APPEND PARA MANEJAR LA VARIACION DEL TAMAÑO DEL FD
	if ((punteroDeArchivo = mmap(0, tamanioValorCompleto, permisos,
	MAP_SHARED, fileDescriptor, 0)) == (caddr_t) -1) {
		printf("mmap error for output\n");
	}

//	punteroDeArchivo = append(fileDescriptor, valor, tamanioDelValor, void *map, size_t len);

	if ( punteroDeArchivo != MAP_FAILED) {
		printf("El Mapeo se efectuo correctamente\n\n");
		printf("Puntero de archivo: %p\n", punteroDeArchivo);
	} else {
		printf("Error en el Mapeo de archivo\n");
		switch (errno) {
		case EINVAL:
			printf(
					"\tEither address was unusable, or inconsistent flags were given.\n");
			break;
		case EACCES:
			printf(
					"\tfiledes was not open for the type of access specified in protect.\n");
			break;
		case ENOMEM:
			printf(
					"\tEither there is not enough memory for the operation, or the process is out of address space.\n");
			break;
		case ENODEV:
			printf("\tThis file is of a type that doesn't support mapping.\n");
			break;
		case ENOEXEC:
			printf(
					"\tThe file is on a filesystem that doesn't support mapping.\n");
			break;
		}
	}

	guardarPunteroDeArchivoEnIndices(indices, punteroDeArchivo);

}

void grabarArchivo(char clave[40]) {
	printf("Preparandose para grabar en archivo...\n");
// TODO: Remover los valores hardcodeados e implementar
	t_list * listaDeIndices = obtenerIndicesDeClave(clave);

	printf("Obteniendo indice base que contiene la clave %s\n", clave);
	t_indice_entrada * primerEntrada = list_get(listaDeIndices, 0);
	printf("\tEl numero de entrada es: %d\n", primerEntrada->numeroEntrada);

	char * puntero = malloc(sizeof(char *));
	puntero = primerEntrada->puntero;
	printf("Puntero donde se encuentra el valor: %p\n", puntero);

	int tamanio = obtenerTamanioTotalDeValorGuardado(listaDeIndices);

	char * path = obtenerPathArchivo(clave);

	char * addressOfNewMapping = (void *) mmap(0, tamanio, PROT_WRITE,
	MAP_SHARED, (int) path, 0);
	if ((int) addressOfNewMapping != -1) {
		printf("mmap se realizo correctamente y se guardo en %p",
				addressOfNewMapping);
	} else {
		printf("Error en el mmap\n");
	}
}

void interpretarOperacionCoordinador(t_content_header * header,
		int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

	char * punteroArchivo;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_CONFIG_INICIAL:

		configTablaEntradas = obtenerConfigTablaEntradas(socketCoordinador);

		crearTablaEntradas(configTablaEntradas);

		break;

	case COORDINADOR_INSTANCIA_SENTENCIA:

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		switch (sentenciaRecibida->keyword) {

		case SET_:
			// Imprimo tabla de entradas para verificar su estado

			guardarClaveValor(sentenciaRecibida);
			imprimirTablaEntradas();
			enviarResultadoSentencia(socketCoordinador, SET_);
			break;

		case STORE_:
			realizarStoreDeClave(sentenciaRecibida->clave);
			// Liberar el indice y la entrada
			enviarResultadoSentencia(socketCoordinador, STORE_);
			break;

		case GET_:
			printf(
					"TODO: Leer clave y devolver valor... (ver si corresponde implementar el GET)\n");
			// enviarResultadoSentencia(socketCoordinador, GET_);
			break;

		}

		break;
	}
}

int main(int argc, char **argv) {

	cargarArchivoDeConfig(argv[1]);

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
