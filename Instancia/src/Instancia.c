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
#include "liberar_clave_atomica.c"

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

t_respuesta_instancia * armarRespuestaParaCoordinador(int resultadoEjecucion) {
	int entradasDisponibles = obtenerEntradasDisponibles();

	t_respuesta_instancia * resultadoAEnviar = malloc(
			sizeof(t_respuesta_instancia));

	resultadoAEnviar->entradas_libres = entradasDisponibles;
	resultadoAEnviar->rdo_operacion = resultadoEjecucion;

	printf("Resultado a enviar:\n");
	printf("\tEntradas disponibles: %d\n", resultadoAEnviar->entradas_libres);
	printf("\tResultado de operacion: %d\n", resultadoAEnviar->rdo_operacion);

	return resultadoAEnviar;

}

void enviarResultadoSentencia(int socketCoordinador, int keyword) {
	printf("Envio de header para respuesta de sentencia...\n");

	int resultado;
	int resultadoEjecucion;

	t_respuesta_instancia * resultadoAEnviar;
	switch (keyword) {

	case OBTENER_VALOR:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, strlen(valorConsultado) + 1);

		printf("Enviando Valor asociado a la clave consultada...\n");
		resultado = send(socketCoordinador, valorConsultado,
				strlen(valorConsultado) + 1, 0);

		if (resultado == -1) {
			printf("Error en el send");
			// TODO: Implementar exit para abortar ejecucion
		}

		printf("\tResultado: %d\n", resultado);
		printf("\tValor obtenido y enviado: %s\n", valorConsultado);
		printf("--------------------------------------------------------\n");
		break;

	case SET_:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

		printf("Enviando Respuesta de sentencia SET...\n");

		resultadoAEnviar = armarRespuestaParaCoordinador(
				respuestaParaCoordinador);

		resultado = send(socketCoordinador, resultadoAEnviar,
				sizeof(t_respuesta_instancia), 0);

		if (resultado == -1) {
			printf("Error en el send");
			// TODO: Implementar exit para abortar ejecucion
		}

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n",
				respuestaParaCoordinador);
		printf("--------------------------------------------------------\n");
		break;

	case STORE_:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

		printf("Enviando Respuesta de sentencia STORE...\n");

		resultadoAEnviar = armarRespuestaParaCoordinador(
				respuestaParaCoordinador);

		resultado = send(socketCoordinador, resultadoAEnviar,
				sizeof(t_respuesta_instancia), 0);

		if (resultado == -1) {
			printf("Error en el send");
			// TODO: Implementar exit para abortar ejecucion
		}

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n",
				respuestaParaCoordinador);
		printf("--------------------------------------------------------\n");
		break;

	case COORDINADOR_INSTANCIA_RECUPERAR_CLAVES:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));
		printf(
				"Enviando Respuesta de Reconexion (cantidad de entradas disponibles)...\n");

		resultadoEjecucion = EXITO_I;

		resultadoAEnviar = armarRespuestaParaCoordinador(resultadoEjecucion);

		resultado = send(socketCoordinador, resultadoAEnviar,
				sizeof(t_respuesta_instancia), 0);

		if (resultado == -1) {
			printf("Error en el send");
			// TODO: Implementar exit para abortar ejecucion
		}

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n", resultadoEjecucion);
		printf("--------------------------------------------------------\n");
		break;

	case COORDINADOR_INSTANCIA_COMPACTAR:
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA, sizeof(int));

		printf("Enviando aviso de finalizacion de Compactacion...\n");

		resultadoAEnviar = armarRespuestaParaCoordinador(
				respuestaParaCoordinador);

		resultado = send(socketCoordinador, resultadoAEnviar,
				sizeof(t_respuesta_instancia), 0);

		if (resultado == -1) {
			printf("Error en el send\n");
			// TODO: Implementar exit para abortar ejecucion
		}

		printf("\tResultado: %d\n", resultado);
		printf("\tResultado de ejecucion enviado: %d\n",
				respuestaParaCoordinador);
		printf("--------------------------------------------------------\n");
		break;

	default:
		break;
	}

	if (resultadoAEnviar)
		free(resultadoAEnviar);
}

t_configTablaEntradas * obtenerConfigTablaEntradas(int socketCoordinador) {

	t_configTablaEntradas * configRecibida = malloc(
			sizeof(t_configTablaEntradas));

	int statusHeader = recv(socketCoordinador, configRecibida,
			sizeof(t_configTablaEntradas), (int) NULL);

	if (statusHeader == -1) {
		printf("Error en el recv");
		// TODO: Implementar exit para abortar ejecucion
	}

	printf(
			"################################################################\n");
	printf("Configuracion inicial de la Tabla de Entradas:\n");
	printf("\tCantidad total de entradas: %d\tTamaño de entradas:%d\n",
			configRecibida->cantTotalEntradas, configRecibida->tamanioEntradas);
	printf(
			"################################################################\n");

	return configRecibida;
}

char * obtenerPathArchivo(char clave[40]) {
	char * puntoDeMontaje = strdup(PUNTO_DE_MONTAJE);
// printf("Directorio donde se guardara el archivo: %s\n", puntoDeMontaje);

	char * archivo;
	archivo = string_new();
	string_append(&archivo, clave);
	string_append(&archivo, ".txt");
// printf("Nombre del archivo que se creara: %s\n", archivo);

	char * path = string_new();
	string_append(&path, puntoDeMontaje);
	string_append(&path, archivo);
	// printf("Path del archivo a crear: %s\n", path);

	return path;
}

void recuperarClave(char clave[40]) {
	printf("Clave a recuperar: %s\n", clave);

	char * path = obtenerPathArchivo(clave);

	int permisos = 0664;
	int flags = O_RDONLY;

	int fileDescriptor;
	if ((fileDescriptor = open(path, flags, permisos)) < 0) {
		printf("No se pudo crear el archivo: %s\n", path);
		printf("\tFile Descriptor: %d\n", fileDescriptor);
	} else {
		// printf("File Descriptor: %d\n", fileDescriptor);

		char * valor = calloc(PACKAGESIZE, sizeof(char));

		int tamanioValor = read(fileDescriptor, valor, PACKAGESIZE);

		printf("Valor recuperado: %s\n", valor);
		printf("Tamaño del Valor recuperado: %d\n", tamanioValor);

		guardarClaveValor(clave, valor);

		free(valor);

		printf("Cerrando el fileDescriptor...\n");
		if (close(fileDescriptor) == 0) {
			// printf("\tArchivo cerrado correctamente.\n");
		} else {
			printf("\tError en el cerrado del archivo.\n");
		}
	}
}

void obtenerClavesARecuperar(int socketCoordinador, int cantClaves) {
	printf("Total de claves a recuperar: %d\n", cantClaves);

	char * clave = malloc(40);

	for (int i = 0; i < cantClaves; i++) {
		recv(socketCoordinador, clave, 40, 0);
		printf("[%d] Clave recibida para recuperar: %s\n", i + 1, clave);

		recuperarClave(clave);
	}

	free(clave);
}

void crearTablaEntradas(t_configTablaEntradas * config) {

	l_indice_entradas = list_create();

	int tamanioTotal = config->cantTotalEntradas * config->tamanioEntradas;

	tablaEntradas = calloc(tamanioTotal, sizeof(char));

	// Esta linea es para limpiar la memoria que se esta reservando
	memset(tablaEntradas, 0, tamanioTotal);

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

	if (statusSentenciaPreliminar == -1) {
		printf("Error en el recv");
		// TODO: Implementar exit para abortar ejecucion
	}

	++contadorOperacion;

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
	}

	else {
		sentenciaRecibida = construirSentenciaConValor(
				sentenciaPreliminarRecibida, NULL);
	}

	return sentenciaRecibida;
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

t_indice_entrada * guardarIndiceAtomicoEnTabla(char clave[40], char * valor,
		int nroEntrada) {

	char * claveAReemplazar;

	t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
	if (reemplazoActivo == 0) {
		strcpy(indiceEntrada->clave, clave);

		printf("Verificando si el indice [%d] ya contiene una entrada...\n",
				nroEntrada);
		if (entradaExistenteEnIndice(nroEntrada)) {
			// TODO en lugar de puntero a char debe ser un char[40]
			char * clave = obtenerClaveExistenteEnEntrada(nroEntrada,
					l_indice_entradas);
			printf(
					"Eliminando todas las entradas asociadas a la clave: %s...\n",
					clave);
			eliminarEntradasAsociadasAClave(clave);
		}

		indiceEntrada->numeroEntrada = nroEntrada;
		numeroEntrada++;

		indiceEntrada->esAtomica = true;
		indiceEntrada->nroDeOperacion = contadorOperacion;
		indiceEntrada->tamanioValor = strlen(valor);

		indiceEntrada->puntero = tablaEntradas
				+ (indiceEntrada->numeroEntrada
						* configTablaEntradas->tamanioEntradas);

		list_add(l_indice_entradas, indiceEntrada);

		printf("Indice agregado correctamente\n");

	} else {
		printf("El reemplazo se encuentra activo..\n");

		claveAReemplazar = obtenerClaveExistenteEnEntrada(nroEntrada,
				l_indice_entradas);
		t_list * listaDeIndices = obtenerIndicesDeClave(claveAReemplazar);
		int tamanioDeValorAReemplazar = obtenerTamanioTotalDeValorGuardado(
				listaDeIndices);

		printf("La clave a reemplazar es: %s y su valor tiene tamaño: %d\n",
				claveAReemplazar, tamanioDeValorAReemplazar);

		int entradasOcupadasPorClaveExistente =
				cantidadDeEntradasRequeridasParaValor(
						tamanioDeValorAReemplazar);

		printf("Cantidad de entradas ocupadas por la clave existente: %d\n",
				entradasOcupadasPorClaveExistente);
		if (entradasOcupadasPorClaveExistente == 1) {
			strcpy(indiceEntrada->clave, clave);

			printf("Verificando si el indice ya contiene una entrada...\n");
			if (entradaExistenteEnIndice(nroEntrada)) {
				// TODO en lugar de puntero a char debe ser un char[40]
				char * clave = obtenerClaveExistenteEnEntrada(nroEntrada,
						l_indice_entradas);
				printf(
						"Eliminando todas las entradas asociadas a la clave: %s...\n",
						clave);
				eliminarEntradasAsociadasAClave(clave);
			}

			indiceEntrada->numeroEntrada = nroEntrada;

			if (strcmp(ALGORITMO_DE_REEMPLAZO, "CIRC") == 0) {
				numeroEntrada = nroEntrada + 1;
			} else {
				numeroEntrada++;
			}

			indiceEntrada->esAtomica = true;
			indiceEntrada->nroDeOperacion = contadorOperacion;
			indiceEntrada->tamanioValor = strlen(valor);

			indiceEntrada->puntero = tablaEntradas
					+ (indiceEntrada->numeroEntrada
							* configTablaEntradas->tamanioEntradas);

			list_add(l_indice_entradas, indiceEntrada);

			printf("Indice agregado correctamente\n");

		} else {
			printf(
					"La entrada a reemplazar contiene un valor NO atomico. Por enunciado no se reemplaza..\n");
			nroEntrada = nroEntrada + entradasOcupadasPorClaveExistente;
			printf("Numero de entrada: %d\n", nroEntrada);
			// Vuelvo a intentar guardar el valor en el nuevo numero de entrada
			guardarIndiceAtomicoEnTabla(clave, valor, nroEntrada);

		}

	}

	return indiceEntrada;
}

t_indice_entrada * guardarEnEntradaDisponible(char clave[40], char * valor) {
	t_indice_entrada * indiceEntrada;

	int nroEntradaDisponible = buscarIndiceDeEntradaDisponible();

	indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
			nroEntradaDisponible);

	return indiceEntrada;
}

void guardarValorEnEntrada(char * valor, char* puntero) {
	printf("Guardando valor: %s en puntero: %p...\n", valor, puntero);
	memcpy(puntero, valor, strlen(valor));

	printf("Valor guardado: %s\n", puntero);
}

void ordenarAscPorCodDeOperacion(t_list * lista) {
	// El comparador devuelve si el primer parametro debe aparecer antes que el segundo en la lista
	_Bool menorCodigoDeOperacion(t_indice_entrada * entrada1,
			t_indice_entrada * entrada2) {
		return (entrada1->nroDeOperacion <= entrada2->nroDeOperacion);
	}

	printf("Ordenando lista por Numero de Operacion (ascendentemente)...\n");
	list_sort(lista, (void*) menorCodigoDeOperacion);

	return;
}

_Bool entradaAtomica(t_indice_entrada * entrada) {
	return (_Bool) entrada->esAtomica;
}

void agregarClaveALista(char * clave, t_list * lista) {
	char * item = malloc(strlen(clave));
	strcpy(item, clave);
	printf("Agregando clave: %s a lista auxiliar...\n", item);
	list_add(lista, item);
	// free(item);
}

void ordenarDescPorTamanio(t_list * indices) {

	printf("Ordenando lista por mayor tamanño de valor (Desc)...\n");

	_Bool mayorTamanioDeValor(t_indice_entrada * entrada1,
			t_indice_entrada * entrada2) {
		return entrada1->tamanioValor >= entrada2->tamanioValor;
	}

	list_sort(indices, (void *) mayorTamanioDeValor);
}

t_list * obtenerClavesDeMayorEspacioUtilizado() {
	printf("Obteniendo clave/s con valores asociados de mayor tamaño...\n");

	t_list * entradasAtomicas = list_filter(l_indice_entradas,
			(void *) entradaAtomica);

	ordenarDescPorTamanio(entradasAtomicas);

	t_list * clavesConMayorValor = list_create();

	t_indice_entrada * entradaConMayorValor;
	t_indice_entrada * segundaEntrada;

	entradaConMayorValor = list_get(entradasAtomicas, 0);
	agregarClaveALista(entradaConMayorValor->clave, clavesConMayorValor);
	int mayorTamanio = entradaConMayorValor->tamanioValor;

	if (list_size(entradasAtomicas) > 1) {
		segundaEntrada = list_get(entradasAtomicas, 1);
		if (segundaEntrada->tamanioValor == mayorTamanio) {
			agregarClaveALista(segundaEntrada->clave, clavesConMayorValor);

		}
	}

	list_destroy(entradasAtomicas);

	return clavesConMayorValor;
}

t_indice_entrada * guardarIndiceNoAtomicoEnTabla(char clave[40], char * valor,
		int numeroEntrada) {

	t_indice_entrada * indiceBase;

	char * punteroBase = tablaEntradas
			+ (numeroEntrada * configTablaEntradas->tamanioEntradas);

	int tamanioTotalValor = strlen(valor);

	printf("\tTamanio total del Valor: %d\n", tamanioTotalValor);
	printf("\tTamanio maximo a guardar por entrada: %d\n",
			configTablaEntradas->tamanioEntradas);

	int entradasNecesariasParaGuardarValor =
			cantidadDeEntradasRequeridasParaValor(tamanioTotalValor);

	printf("\tGuardando un total de %d entradas...\n",
			entradasNecesariasParaGuardarValor);

	for (int i = 1; i <= entradasNecesariasParaGuardarValor; i++) {
		printf("Guardando %d entrada requerida\n", i);

		t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
		strcpy(indiceEntrada->clave, clave);

// validar que no exceda canntidad total de entradas
		indiceEntrada->numeroEntrada = numeroEntrada;
		printf("\tGuardando entrada en indice: %d\n",
				indiceEntrada->numeroEntrada);
		numeroEntrada++;

		indiceEntrada->esAtomica = false;
		indiceEntrada->nroDeOperacion = contadorOperacion;

		if (tamanioTotalValor > configTablaEntradas->tamanioEntradas) {
			indiceEntrada->tamanioValor = configTablaEntradas->tamanioEntradas;
		} else {
			indiceEntrada->tamanioValor = tamanioTotalValor;
		}
		tamanioTotalValor = tamanioTotalValor - indiceEntrada->tamanioValor;
		printf("\tTamanio de valor a guardar en indice: %d\n",
				indiceEntrada->tamanioValor);
		printf(
				"\t\tAun queda pendiente guardar %d del tamanio total del valor\n",
				tamanioTotalValor);

		indiceEntrada->puntero = tablaEntradas
				+ (indiceEntrada->numeroEntrada
						* configTablaEntradas->tamanioEntradas);
		printf("\tPuntero: %p\n", indiceEntrada->puntero);

		list_add(l_indice_entradas, indiceEntrada);

		printf("Indice agregado correctamente\n");

// Asignando como indice Base el primer indice correspondiente a la clave
		if (i == 1) {
			indiceBase = indiceEntrada;
		}

	}

	return indiceBase;
}

t_indice_entrada * aplicarAlgoritmoDeReemplazo(char clave[40], char * valor) {
	printf("Aplicando algoritmo de reemplazo: %s\n", ALGORITMO_DE_REEMPLAZO);

	t_indice_entrada * indiceEntrada;
	reemplazoActivo = 1;

	if (strcmp(ALGORITMO_DE_REEMPLAZO, "CIRC") == 0) {
		numeroEntrada = 0;
		printf("El puntero fue posicionado en la entrada: %d\n", numeroEntrada);

		int entradasNecesariasParaGuardarValor =
				cantidadDeEntradasRequeridasParaValor(strlen(valor));
		printf("Entradas requeridas para guardar el valor: %d\n",
				entradasNecesariasParaGuardarValor);

		if (entradasNecesariasParaGuardarValor > 1) {
			indiceEntrada = guardarIndiceNoAtomicoEnTabla(clave, valor,
					numeroEntrada);

		} else {
			indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
					numeroEntrada);
		}

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "LRU") == 0) {

		t_list * indicesAtomicos = list_filter(l_indice_entradas,
				(void *) entradaAtomica);

		ordenarAscPorCodDeOperacion(indicesAtomicos);

		t_indice_entrada * entradaMenosUsada = list_get(indicesAtomicos, 0);

// TODO en lugar de puntero a char debe ser un char[40]
		char * claveAReemplazar = obtenerClaveExistenteEnEntrada(
				entradaMenosUsada->numeroEntrada, indicesAtomicos);

		printf("Clave a ser reemplazada: %s\n", claveAReemplazar);

// TODO: Eliminar los elementos de la lista destruida
		list_destroy(indicesAtomicos);

		t_list * indicesQueContienenClave = obtenerIndicesDeClave(
				claveAReemplazar);

		t_indice_entrada * entradaBase = list_get(indicesQueContienenClave, 0);

		indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
				entradaBase->numeroEntrada);

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "BSU") == 0) {

		t_list * claves = obtenerClavesDeMayorEspacioUtilizado();

		// Compruebo que no haya empates en el algoritmo
		if (list_size(claves) == 1) {
			char * claveAReemplazar = list_get(claves, 0);
			printf("Clave a ser reemplazada: %s\n", claveAReemplazar);

			t_list * indicesQueContienenClave = obtenerIndicesDeClave(
					claveAReemplazar);

			t_indice_entrada * entradaBase = list_get(indicesQueContienenClave,
					0);

			indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
					entradaBase->numeroEntrada);
		} else {
			// TODO: Modularizar el algoritmo circular
			printf(
					"Hay empate con el algoritmo BSU. Ejecutando algoritmo desempatador (CIRC)...\n");

			numeroEntrada = 0;
			printf("El puntero fue posicionado en la entrada: %d\n",
					numeroEntrada);
			indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
					numeroEntrada);
		}

		list_destroy(claves);
	}

	return indiceEntrada;
}

void actualizarEntradasConNuevoValor(char clave[40], char * valor) {
	t_list * indices = obtenerIndicesDeClave(clave);

	int entradasRequeridas = cantidadDeEntradasRequeridasParaValor(
			strlen(valor));

	for (int i = 0; i < list_size(indices); i++) {
		t_indice_entrada * entradaActual = list_get(indices, i);

		if (i >= entradasRequeridas) {

			printf("Entrada [%d] eliminada.\n", entradaActual->numeroEntrada);
			//list_remove_and_destroy_element(l_indice_entradas, entradaActual->numeroEntrada,
			//		destruir_indice_entrada);

			_Bool contieneNroEntrada(t_indice_entrada * entradaAEliminar) {
				return entradaAEliminar->numeroEntrada
						== entradaActual->numeroEntrada;
			}

			list_remove_by_condition(l_indice_entradas,
					(void *) contieneNroEntrada);
			printf("Destruida");
		} else {
			if (i == 0) {
				int viejoTamanio = obtenerTamanioTotalDeValorGuardado(indices);
				memset(entradaActual->puntero, 0, viejoTamanio);
				strcpy(entradaActual->puntero, valor);
			}

			if (i == entradasRequeridas - 1) {
				// ultima entrada
				entradaActual->tamanioValor = strlen(valor)
						- (i * configTablaEntradas->tamanioEntradas);
			} else {
				//entradas intermedias
				entradaActual->tamanioValor =
						configTablaEntradas->tamanioEntradas;
			}

			entradaActual->nroDeOperacion = contadorOperacion;

			printf("Entrada [%d] actualizada.\n", entradaActual->numeroEntrada);
		}
	}

// list_destroy(indices);

	contadorOperacion++;

	imprimirTablaEntradas();

	printf("%d entradas actualizadas correctamente\n", entradasRequeridas);
}

_Bool hayEntradasContiguasDisponibles(int cantRequerida) {

	printf("Obteniendo mayor cantidad de entradas contiguas disponibles...\n");
	int mayorCantDeEntradasContiguasDisp = 0;
	int contAux = 0;

	for (int i = 0; i < configTablaEntradas->cantTotalEntradas; i++) {
		_Bool entradaLibre = entradaExistenteEnIndice(i);
// printf("\t\tEntrada [%d] ocupada: %d\n", i, entradaLibre);
		if (!entradaLibre) {
			// printf("\t\tIncrementando contador auxiliar..\n");
			contAux++;
			if (i == (configTablaEntradas->cantTotalEntradas - 1)) {
				// printf("\t\tUltima entrada...\n");
				mayorCantDeEntradasContiguasDisp = contAux;
				nroEntradaBaseAux = i + 1;
			}
		} else {
			contAux = 0;
			if (contAux > mayorCantDeEntradasContiguasDisp) {
				mayorCantDeEntradasContiguasDisp = contAux;
				nroEntradaBaseAux = i + 1;
			}
		}

	}

	nroEntradaBaseAux = nroEntradaBaseAux - mayorCantDeEntradasContiguasDisp;

	printf(
			"Numero de entrada donde se debe guardar el proximo indice NO atomico: %d\n",
			nroEntradaBaseAux);

	printf("\tMayor cantidad de entradas contiguas disponibles: %d\n",
			mayorCantDeEntradasContiguasDisp);

	return mayorCantDeEntradasContiguasDisp >= cantRequerida;
}

int obtenerCantidadAtomicos() {
	bool esAtomica(t_indice_entrada * entrada) {
		return entrada->esAtomica;
	}
	t_list * atomicos = list_filter(l_indice_entradas, (void*) esAtomica);
	int cantidad = list_size(atomicos);

//libero la lista filtrada (porque filter devuelve una nueva)
	/*void lib_punt(t_indice_entrada * ent){free(ent->puntero);};
	 list_iterate(atomicos,*lib_punt);
	 list_destroy(atomicos);
	 */
	return cantidad;

}

void guardarClaveValor(char clave[40], char * valor) {

	t_list * indicesQueContienenClave = obtenerIndicesDeClave(clave);
	int cantidadDeIndices = list_size(indicesQueContienenClave);
	printf("Cantidad de indices que contienen clave: %d\n", cantidadDeIndices);

	if (cantidadDeIndices > 0) {
		printf("La clave '%s' ya existe. Reemplazar entrada/s...\n", clave);

		int entradasNecesarias = cantidadDeEntradasRequeridasParaValor(
				strlen(valor));

		if (entradasNecesarias > cantidadDeIndices) {
			printf(
					"SET no podra hacer que se ocupen mas entradas (enunciado)\n");
			respuestaParaCoordinador = ERROR_I;

		} else {
			printf("Actualizando entradas existentes con nuevo valor: %s\n",
					valor);
			actualizarEntradasConNuevoValor(clave, valor);
		}

// actualizarNroDeOperacion(indicesQueContienenClave);

		respuestaParaCoordinador = EXITO_I;
	} else {
		printf("La clave no existe... guardar...\n");

		int entradasNecesariasParaGuardarValor =
				cantidadDeEntradasRequeridasParaValor(strlen(valor));

		if (entradasNecesariasParaGuardarValor > 1) {
			// Guardar valor NO atomico en varias entradas

			int entradasDisponibles = obtenerEntradasDisponibles();

			if (entradasDisponibles >= entradasNecesariasParaGuardarValor) {

				if (hayEntradasContiguasDisponibles(
						entradasNecesariasParaGuardarValor)) {

					printf("No es necesario compactar tabla de entradas...\n");

					if (numeroEntrada
							>= configTablaEntradas->cantTotalEntradas) {

						numeroEntrada = nroEntradaBaseAux;
						printf("Valor de numeroEntrada: %d. El aux: %d\n",
								numeroEntrada, nroEntradaBaseAux);
					}

					t_indice_entrada * indiceBase =
							guardarIndiceNoAtomicoEnTabla(clave, valor,
									numeroEntrada);

					numeroEntrada = numeroEntrada
							+ entradasNecesariasParaGuardarValor;

					// guardarValorEnEntrada(sentenciaRecibida->valor,	indiceEntrada->puntero);

					printf("Guardando valor: %s en puntero: %p...\n", valor,
							indiceBase->puntero);
					memcpy(indiceBase->puntero, valor, strlen(valor));

					printf("Valor guardado: %s\n", indiceBase->puntero);

					respuestaParaCoordinador = EXITO_I;
				} else {

					printf(
							"Es necesario compactar... Enviando solicitud al Coordinador...\n");
					respuestaParaCoordinador = COMPACTAR;

				}

			} else {
				//cantidadAtomicas + entradaslibres > ccantidad entradas necesarias para el nuevo valor no atomico?
				int entradasDisponibles = obtenerEntradasDisponibles();
				int cantidadValoresAtomicos = obtenerCantidadAtomicos();
				int totalLibreMasAtomicos = entradasDisponibles
						+ cantidadValoresAtomicos;
				printf(
						"**********entradas necesarias: %d, disp: %d, cant atom: %d, tot libre + atom: %d\n\n",
						entradasNecesariasParaGuardarValor, entradasDisponibles,
						cantidadValoresAtomicos, totalLibreMasAtomicos);
				if (entradasNecesariasParaGuardarValor
						> totalLibreMasAtomicos) {
					//abortar
					printf(
							"No hay mas lugar para guardar un valor NO atomico.\n");
					respuestaParaCoordinador = ERROR_I;
					//TODO: Devolver un error al coordinador
				} else {
					//libre + atomico mayor o igual al necesario
					//while no hay espacio libre suficiente:
					//libero una atomica

					//hay espacio contiguo para el nuevo valor?
					//no: compactar
					//si: reemplazo segun algoritmo
					while (entradasNecesariasParaGuardarValor
							> entradasDisponibles) {
						//liberar una clave
						liberar_clave_atom();
						entradasDisponibles = obtenerEntradasDisponibles();
						imprimirTablaEntradas();
						printf(
								"**********entradas necesarias: %d, disp: %d, cant atom: %d, tot libre + atom: %d\n\n",
								entradasNecesariasParaGuardarValor,
								entradasDisponibles, cantidadValoresAtomicos,
								totalLibreMasAtomicos);
					};
					if (entradasDisponibles
							>= entradasNecesariasParaGuardarValor) {
						if (hayEntradasContiguasDisponibles(
								entradasNecesariasParaGuardarValor)) {

							printf(
									"No es necesario compactar tabla de entradas...\n");

							if (numeroEntrada
									>= configTablaEntradas->cantTotalEntradas) {

								numeroEntrada = nroEntradaBaseAux;
								printf(
										"Valor de numeroEntrada: %d. El aux: %d\n",
										numeroEntrada, nroEntradaBaseAux);
							}

							t_indice_entrada * indiceBase =
									guardarIndiceNoAtomicoEnTabla(clave, valor,
											numeroEntrada);

							numeroEntrada = numeroEntrada
									+ entradasNecesariasParaGuardarValor;

							// guardarValorEnEntrada(sentenciaRecibida->valor,	indiceEntrada->puntero);

							printf("Guardando valor: %s en puntero: %p...\n",
									valor, indiceBase->puntero);
							memcpy(indiceBase->puntero, valor, strlen(valor));

							printf("Valor guardado: %s\n", indiceBase->puntero);

							respuestaParaCoordinador = EXITO_I;
						} else {
							printf("**********SE NECESITA COMPACTAR\n\n");

							printf(
									"Es necesario compactar... Enviando solicitud al Coordinador...\n");
							respuestaParaCoordinador = COMPACTAR;
						}
					}
				}
			}

		} else {

			printf("Agregando unica entrada, es decir valor atomico\n");

			t_indice_entrada * indiceEntrada;

			if (numeroEntrada >= configTablaEntradas->cantTotalEntradas) {
				int entradasDisponibles = obtenerEntradasDisponibles();

				if (entradasDisponibles != 0) {
					indiceEntrada = guardarEnEntradaDisponible(clave, valor);
				} else {
					printf("Es necesario aplicar algoritmo de reemplazo...\n");
					indiceEntrada = aplicarAlgoritmoDeReemplazo(clave, valor);
				}
			} else {
				printf("No es necesario aplicar algoritmo de reemplazo\n");
				indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
						numeroEntrada);
			}

			// guardarValorEnEntrada(sentenciaRecibida->valor,indiceEntrada->puntero);

			printf("Guardando valor: %s en puntero: %p...\n", valor,
					indiceEntrada->puntero);
			memset(indiceEntrada->puntero, 0,
					entradasNecesariasParaGuardarValor
							* configTablaEntradas->tamanioEntradas);
			memcpy(indiceEntrada->puntero, valor, strlen(valor));

			printf("Valor guardado: %s\n", indiceEntrada->puntero);

			respuestaParaCoordinador = EXITO_I;
		}

		imprimirTablaEntradas();

	}
}

void make_directory(const char* name) {
//char * check;
	int check;
	check = mkdir(name, 0777);

	if (!check)
		printf("Directorio creado: %s\t check = %d\n", name, check);

	else {
		// printf("No se puede crear el directorio... o ya existe...\t check = %d\n",check);
	}
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

void obtenerValorAsociadoAClave(char clave[40]) {
	printf("Obteniendo valor asociado a la clave: %s...\n", clave);
	t_list * listaIndices = obtenerIndicesDeClave(clave);
	int tamanioValor = obtenerTamanioTotalDeValorGuardado(listaIndices);
	t_indice_entrada * indiceBase = list_get(listaIndices, 0);

// Desalojo memoria reservada para la variable global y vuelvo a alocar el nuevo espacio necesario
	free(valorConsultado);
	valorConsultado = malloc(tamanioValor);

	valorConsultado = obtenerValorCompleto(indiceBase->puntero, tamanioValor);

	printf("Valor copiado a variable global: %s\n", valorConsultado);
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
		// printf("File Descriptor: %d\n", fileDescriptor);
	}

	t_list * indices = obtenerIndicesDeClave(clave);

	if (list_size(indices) > 0) {
		if (pthread_self() == threadId[0]) {
			// En caso de ser el hilo DUMP, no quiero actualizar operacion
			actualizarNroDeOperacion(indices);

			imprimirTablaEntradas();
		}

		t_indice_entrada * primerEntrada = list_get(indices, 0);

		int tamanioValorCompleto = obtenerTamanioTotalDeValorGuardado(indices);
		// printf("Tamaño del valor guardado: %d\n", tamanioValorCompleto);

		char * valorCompleto = obtenerValorCompleto(primerEntrada->puntero,
				tamanioValorCompleto + 1);
		printf("El valor completo es: %s\n", valorCompleto);

		grabarArchivoPorPrimeraVez(fileDescriptor, valorCompleto,
				tamanioValorCompleto);

		char * punteroDeArchivo;

//  ACA APLICAR EL APPEND PARA MANEJAR LA VARIACION DEL TAMAÑO DEL FD
		if ((punteroDeArchivo = mmap(0, tamanioValorCompleto, permisos,
		MAP_SHARED, fileDescriptor, 0)) == (caddr_t) -1) {
			printf("mmap error for output\n");
		}

//	punteroDeArchivo = append(fileDescriptor, valor, tamanioDelValor, void *map, size_t len);

		if (punteroDeArchivo != MAP_FAILED) {
			// printf("El Mapeo se efectuo correctamente\n\n");
			// printf("\tPuntero de archivo: %p\n", punteroDeArchivo);
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
				printf(
						"\tThis file is of a type that doesn't support mapping.\n");
				break;
			case ENOEXEC:
				printf(
						"\tThe file is on a filesystem that doesn't support mapping.\n");
				break;
			}
		}

		if (close(fileDescriptor) == 0) {
			// printf("\tArchivo cerrado correctamente.\n");
		} else {
			printf("\tError en el cerrado del archivo.\n");
		}

		respuestaParaCoordinador = EXITO_I;

	} else {
		respuestaParaCoordinador = ERROR_I;
		printf("La clave %s no se encuentra en la tabla de entradas...\n",
				clave);
	}

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

void ordenarAscPorNroDeEntrada(t_list * lista) {

	printf(
			"Ordenando tabla de entradas por numero de operacion ascendentemente...\n");

	_Bool menorNroDeEntrada(t_indice_entrada * entrada1,
			t_indice_entrada * entrada2) {
		return entrada1->numeroEntrada < entrada2->numeroEntrada;
	}

	list_sort(lista, (void *) menorNroDeEntrada);
}

void compactarEntradas() {

	int cantEntradasExistentes = list_size(l_indice_entradas);

	printf("La tabla de entradas contiene %d indices...\n",
			cantEntradasExistentes);

	imprimirTablaEntradas();

	t_list * listaAuxiliar = list_create();
	printf("Lista auxiliar creada...\n");

	for (int i = 0; i < cantEntradasExistentes; i++) {
		t_indice_entrada * entrada = list_get(l_indice_entradas, i);
		t_indice_entrada * entradaAux = malloc(sizeof(t_indice_entrada));

		entradaAux->numeroEntrada = i;
		strcpy(entradaAux->clave, entrada->clave);
		entradaAux->tamanioValor = entrada->tamanioValor;
		entradaAux->esAtomica = entrada->esAtomica;
		entradaAux->nroDeOperacion = entrada->nroDeOperacion;

		entradaAux->puntero = tablaEntradas
				+ (i * configTablaEntradas->tamanioEntradas);
		strcpy(entradaAux->puntero, entrada->puntero);

		list_add(listaAuxiliar, entradaAux);

	}

	list_clean(l_indice_entradas);
	printf("Clean efectuado correctamente sobre la tabla de entradas...\n");

	printf("Asignando lista auxiliar a la tabla de entradas...\n");

	for (int i = 0; i < list_size(listaAuxiliar); i++) {
		t_indice_entrada * entrada = list_get(listaAuxiliar, i);
		t_indice_entrada * entradaAux = malloc(sizeof(t_indice_entrada));

		entradaAux->numeroEntrada = entrada->numeroEntrada;
		strcpy(entradaAux->clave, entrada->clave);
		entradaAux->tamanioValor = entrada->tamanioValor;
		entradaAux->esAtomica = entrada->esAtomica;
		entradaAux->nroDeOperacion = entrada->nroDeOperacion;
		entradaAux->puntero = entrada->puntero;

		list_add(l_indice_entradas, entradaAux);
	}
//list_add_all(l_indice_entradas, listaAuxiliar);

	list_destroy_and_destroy_elements(listaAuxiliar, (void*) free);
}

void interpretarOperacionCoordinador(t_content_header * header,
		int socketCoordinador) {

	t_sentencia * sentenciaRecibida;

	switch (header->operacion) {

	case COORDINADOR_INSTANCIA_CONFIG_INICIAL:

		configTablaEntradas = obtenerConfigTablaEntradas(socketCoordinador);

		crearTablaEntradas(configTablaEntradas);

		break;

	case COORDINADOR_INSTANCIA_RECUPERAR_CLAVES:

		printf("Recibiendo claves a recuperar...\n");

		obtenerClavesARecuperar(socketCoordinador, header->cantidad_a_leer);

		enviarResultadoSentencia(socketCoordinador,
		COORDINADOR_INSTANCIA_RECUPERAR_CLAVES);

		break;

	case COORDINADOR_INSTANCIA_SENTENCIA:

		pthread_mutex_lock(&mutexDump);

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		switch (sentenciaRecibida->keyword) {

		case OBTENER_VALOR:
			obtenerValorAsociadoAClave(sentenciaRecibida->clave);

			enviarResultadoSentencia(socketCoordinador, OBTENER_VALOR);
			break;

		case SET_:
			guardarClaveValor(sentenciaRecibida->clave,
					sentenciaRecibida->valor);
			enviarResultadoSentencia(socketCoordinador, SET_);
			break;

		case STORE_:
			realizarStoreDeClave(sentenciaRecibida->clave);
			enviarResultadoSentencia(socketCoordinador, STORE_);
			break;
		}

		pthread_mutex_unlock(&mutexDump);

		break;

	case COORDINADOR_INSTANCIA_COMPROBAR_CONEXION:
		printf("Confirmando a coordinador conexion activa de Instancia...\n");
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_CONFIRMA_CONEXION_ACTIVA, 0);
		break;

	case COORDINADOR_INSTANCIA_COMPACTAR:
		printf("Iniciando proceso de Compactacion...\n");

// Esta linea se debe eliminar luego de implementar compactar()
// respuestaParaCoordinador = EXITO_I;

		compactarEntradas();

		imprimirTablaEntradas();

		enviarResultadoSentencia(socketCoordinador,
		COORDINADOR_INSTANCIA_COMPACTAR);

	}
}

void ejecutarDump() {
	void storearClave(t_indice_entrada * entrada) {
		realizarStoreDeClave(entrada->clave);
	}

	list_iterate(l_indice_entradas, (void*) storearClave);

}

void iniciarDump() {
	while (1) {
		usleep(INTERVALO_DUMP * 1000000); // El producto es necesario ya que usleep recibe microsegundos como parametro
		printf("\n#           Comenzando proceso Dump...          #\n");

		pthread_mutex_lock(&mutexDump);
		ejecutarDump();
		pthread_mutex_unlock(&mutexDump);

		printf("#          Dump realizado correctamente.        #\n\n");
	}
}

int main(int argc, char **argv) {

	cargarArchivoDeConfig(argv[1]);
	configurar_signals();

	socketCoordinador = conexionConCoordinador();

	enviarNombreInstanciaACoordinador(socketCoordinador);

	pthread_mutex_init(&mutexDump, NULL);

	int err = pthread_create(&(threadId[1]), NULL, (void *) iniciarDump, NULL);
	if (err != 0)
		printf("No se pudo crear el thread para DUMP: [%s]\n", strerror(err));
	else
		printf("Thread para DUMP creado correctamente\n");

	t_content_header * header = malloc(sizeof(t_content_header));
	int status = 1;

	status = interpretarHeader(socketCoordinador, header);
	while (status != -1 && status != 0) {
		switch (header->proceso_origen) {

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

	free(header);
	pthread_cancel(threadId[1]);

	close(socketCoordinador);

	return 0;
}

void configurar_signals(void) {
	struct sigaction signal_struct;
	signal_struct.sa_handler = captura_sigint;
	signal_struct.sa_flags = 0;

	sigemptyset(&signal_struct.sa_mask);

	sigaddset(&signal_struct.sa_mask, SIGINT);
	if (sigaction(SIGINT, &signal_struct, NULL) < 0) {
		fprintf(stderr, "sigaction error\n");
		exit(1);
	}

}

void captura_sigint(int signo) {
	printf(
			"\n\nLiberando memoria alocada...\n\n");

	if (signo == SIGINT) {
		finalizar_instancia();

		exit(EXIT_FAILURE);
	}

}

void finalizar_instancia(void) {

//Destruyo las configs
	if (IP_COORDINADOR) {
		free(IP_COORDINADOR);
		IP_COORDINADOR = NULL;
	}

	if (PUNTO_DE_MONTAJE) {
		free(PUNTO_DE_MONTAJE);
		PUNTO_DE_MONTAJE = NULL;
	}

	if (NOMBRE_INSTANCIA) {
		free(NOMBRE_INSTANCIA);
		NOMBRE_INSTANCIA = NULL;
	}

	if (ALGORITMO_DE_REEMPLAZO) {
		free(ALGORITMO_DE_REEMPLAZO);
		ALGORITMO_DE_REEMPLAZO = NULL;
	}

	destruir_tabla_entradas();

	pthread_cancel(threadId[1]);

	close(socketCoordinador);

}

void destruir_indice_entrada(t_indice_entrada * indice_entrada) {
	/* if (indice_entrada->puntero) {
	 free(indice_entrada->puntero);
	 }*/

	free(indice_entrada);
}

void destruir_tabla_entradas(void) {

	list_destroy_and_destroy_elements(l_indice_entradas,
			(void*) destruir_indice_entrada);

	if (configTablaEntradas)
		free(configTablaEntradas);

	if (tablaEntradas)
		free(tablaEntradas);

}
