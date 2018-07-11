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
			printf("Error en el send");
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
	printf("Path del archivo a crear: %s\n", path);

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
		printf("File Descriptor: %d\n", fileDescriptor);

		char * valor = malloc(PACKAGESIZE);

		int tamanioValor = read(fileDescriptor, valor, PACKAGESIZE);

		printf("Valor recuperado: %s\n", valor);
		printf("Tamaño del Valor recuperado: %d\n", tamanioValor);

		guardarClaveValor(clave, valor);

		free(valor);

		printf("Cerrando el fileDescriptor...\n");
		if (close(fileDescriptor) == 0) {
			printf("\tArchivo cerrado correctamente.\n");
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

	tablaEntradas = malloc(tamanioTotal);

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

	t_indice_entrada * indiceEntrada = malloc(sizeof(t_indice_entrada));
	strcpy(indiceEntrada->clave, clave);

	printf("Verificando si el indice ya contiene una entrada...\n");
	if (entradaExistenteEnIndice(nroEntrada)) {
		// TODO en lugar de puntero a char debe ser un char[40]
		char * clave = obtenerClaveExistenteEnEntrada(nroEntrada,
				l_indice_entradas);
		printf("Eliminando todas las entradas asociadas a la clave: %s...\n",
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

t_list * obtenerClavesDeMayorEspacioUtilizado() {
	printf("Obteniendo clave/s con valores asociados de mayor tamaño...\n");

	t_indice_entrada * entradaAnterior = list_get(l_indice_entradas, 0);
	t_indice_entrada * entradaActual;
	t_list * clavesConMayorValor = list_create();

	char * claveAnterior = malloc(sizeof(char *));
	char * claveActual = malloc(sizeof(char *));
	strcpy(claveAnterior, entradaAnterior->clave);

	int tamanioParcial = 0;
	int mayorTamanio = 0;

	for (int i = 0; i < list_size(l_indice_entradas); i++) {
		printf("Recorriendo entrada: %d\n", i);
		entradaActual = list_get(l_indice_entradas, i);
		strcpy(claveActual, entradaActual->clave);

		if (strcmp(claveActual, claveAnterior) != 0) {
			printf("\tCambio de clave: %s a clave: %s...\n", claveAnterior, claveActual);
			tamanioParcial = 0;
		}

		tamanioParcial = tamanioParcial + entradaActual->tamanioValor;
		printf("Tamanio parcial: %d. Mayor tamanio: %d\n", tamanioParcial, mayorTamanio);

		if (tamanioParcial > mayorTamanio) {
				printf("\tNueva clave con mayor valor encontrada: %s\n", claveActual);
				list_clean(clavesConMayorValor);
				printf("\t\tClean de lista efectuado.\n");
				list_add(clavesConMayorValor, claveActual);
				printf("\t\tAgregando nueva clave de mayor valor: %s\n", claveActual);

			mayorTamanio = tamanioParcial;

		} else {
			if (tamanioParcial == mayorTamanio) {
				printf(
						"\tAgregando otra clave ya que tienen mismo tamanio maximo: %s\n", claveActual);
				list_add(clavesConMayorValor, claveActual);
			}
		}

		strcpy(claveAnterior, claveActual);
	}

	printf("Devolviendo claves con mayor valor...\n");

	for(int i = 0; i < list_size(clavesConMayorValor); i++) {
		printf("\tClave: %s\n", (char *) list_get(clavesConMayorValor, i));
	}

	return clavesConMayorValor;
}

t_indice_entrada * aplicarAlgoritmoDeReemplazo(char clave[40], char * valor) {
	printf("Aplicando algoritmo de reemplazo: %s\n", ALGORITMO_DE_REEMPLAZO);

	t_indice_entrada * indiceEntrada;

	if (strcmp(ALGORITMO_DE_REEMPLAZO, "CIRC") == 0) {
		numeroEntrada = 0;
		printf("El puntero fue posicionado en la entrada: %d\n", numeroEntrada);
		indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
				numeroEntrada);

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "LRU") == 0) {
		// TODO: Buscar entrada con menor nro de operacion y reemplazarla

		t_list * listaAux = list_duplicate(l_indice_entradas);

		ordenarAscPorCodDeOperacion(listaAux);

		t_indice_entrada * entradaMenosUsada = list_get(listaAux, 0);

		// TODO en lugar de puntero a char debe ser un char[40]
		char * claveAReemplazar = obtenerClaveExistenteEnEntrada(
				entradaMenosUsada->numeroEntrada, listaAux);

		printf("Clave a ser reemplazada: %s\n", claveAReemplazar);

		// TODO: Eliminar los elementos de la lista destruida
		list_destroy(listaAux);

		t_list * indicesQueContienenClave = obtenerIndicesDeClave(
				claveAReemplazar);

		t_indice_entrada * entradaBase = list_get(indicesQueContienenClave, 0);

		indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
				entradaBase->numeroEntrada);

	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "BSU") == 0) {

		t_list * claves = obtenerClavesDeMayorEspacioUtilizado();

		// Compruebo que no haya empates en el algoritmo
		if (list_size(claves) == 1) {
			// char claveAReemplazar[40] = list_get(claves, 0);
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
					"Hay empate con el algoritmo BSU. Ejecutando algoritmo desempatador...\n");

			numeroEntrada = 0;
			printf("El puntero fue posicionado en la entrada: %d\n",
					numeroEntrada);
			indiceEntrada = guardarIndiceAtomicoEnTabla(clave, valor,
					numeroEntrada);
		}
	}

	return indiceEntrada;
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

void actualizarEntradasConNuevoValor(char clave[40], char * valor) {
	t_list * indices = obtenerIndicesDeClave(clave);
	t_indice_entrada * indiceBase = list_get(indices, 0);
	eliminarEntradasAsociadasAClave(clave);

// Me guardo el numero de entrada para no perderlo al guardar claveValor
	int nroEntradaAux = numeroEntrada;

// Seteo el numero de entrada con el de las claves ya existentes.
	numeroEntrada = indiceBase->numeroEntrada;
	guardarClaveValor(clave, valor);

// Recupero el numero de entrada
	numeroEntrada = nroEntradaAux;
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
			// TODO: devolver error al Coordinador
		} else {
			printf("Actualizando entradas existentes con nuevo valor: %s\n",
					valor);
			actualizarEntradasConNuevoValor(clave, valor);
		}

		actualizarNroDeOperacion(indicesQueContienenClave);

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

						// TODO: Buscar donde se encuentra el PRIMER indiceBase para guardar la clave
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
				printf("No hay mas lugar para guardar un valor NO atomico.\n");
				respuestaParaCoordinador = ERROR_I;
				//TODO: Devolver un error al coordinador
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
		printf(
				"No se puede crear el directorio... o ya existe...\t check = %d\n",
				check);
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

	if (pthread_self() == threadId[0]) {
		// En caso de ser el hilo DUMP, no quiero actualizar operacion
		actualizarNroDeOperacion(indices);

		imprimirTablaEntradas();
	}

	t_indice_entrada * primerEntrada = list_get(indices, 0);

	int tamanioValorCompleto = obtenerTamanioTotalDeValorGuardado(indices);
	printf("Tamaño del valor guardado: %d\n", tamanioValorCompleto);

	char * valorCompleto = obtenerValorCompleto(primerEntrada->puntero,
			tamanioValorCompleto);
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
		printf("El Mapeo se efectuo correctamente\n\n");
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
			printf("\tThis file is of a type that doesn't support mapping.\n");
			break;
		case ENOEXEC:
			printf(
					"\tThe file is on a filesystem that doesn't support mapping.\n");
			break;
		}
	}

	if (close(fileDescriptor) == 0) {
		printf("\tArchivo cerrado correctamente.\n");
	} else {
		printf("\tError en el cerrado del archivo.\n");
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

	printf("Las tabla de entradas contiene %d indices...\n",
			cantEntradasExistentes);

	ordenarAscPorNroDeEntrada(l_indice_entradas);

	t_list * listaAuxiliar = list_create();
	printf("Lista auxiliar creada...\n");

	for (int i = 0; i < cantEntradasExistentes; i++) {
		t_indice_entrada * entrada = list_get(l_indice_entradas, i);
		t_indice_entrada * entradaAux;

		entradaAux->numeroEntrada = i;
		strcpy(entradaAux->clave, entrada->clave);
		entradaAux->tamanioValor = entrada->tamanioValor;
		entradaAux->esAtomica = entrada->esAtomica;
		entradaAux->nroDeOperacion = entrada->nroDeOperacion;
		entradaAux->puntero = tablaEntradas
				+ (i * configTablaEntradas->tamanioEntradas);

		list_add(listaAuxiliar, entradaAux);
	}

	list_clean(l_indice_entradas);
	printf("Clean efectuado correctamente sobre la tabla de entradas...\n");

	printf("Asignando lista auxiliar a la tabla de entradas...\n");
	list_add_all(l_indice_entradas, listaAuxiliar);

	list_destroy(listaAuxiliar);
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

		sentenciaRecibida = recibirSentencia(socketCoordinador);

		switch (sentenciaRecibida->keyword) {

		case OBTENER_VALOR:
			break;

		case SET_:
			guardarClaveValor(sentenciaRecibida->clave,
					sentenciaRecibida->valor);
			// TODO: Probablemente el siguiente print sea innecesario.
			// imprimirTablaEntradas();
			enviarResultadoSentencia(socketCoordinador, SET_);
			break;

		case STORE_:
			realizarStoreDeClave(sentenciaRecibida->clave);
			enviarResultadoSentencia(socketCoordinador, STORE_);
			break;

		}

		break;

	case COORDINADOR_INSTANCIA_COMPROBAR_CONEXION:
		printf("Confirmando a coordinador conexion activa de Instancia...\n");
		enviarHeader(socketCoordinador, instancia, coordinador,
		INSTANCIA_COORDINADOR_CONFIRMA_CONEXION_ACTIVA, 0);
		break;

	case COORDINADOR_INSTANCIA_COMPACTAR:
		printf("Iniciando proceso de Compactacion...\n");
		printf("TODO: compactar()\n");

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
		printf("#################################################\n");
		printf("#           Comenzando proceso Dump...          #\n");
		printf("#################################################\n");

		ejecutarDump();

		printf("#################################################\n");
		printf("#          Dump realizado correctamente.        #\n");
		printf("#################################################\n");
	}
}

int main(int argc, char **argv) {

	cargarArchivoDeConfig(argv[1]);

	int socketCoordinador = conexionConCoordinador();

	enviarNombreInstanciaACoordinador(socketCoordinador);

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

	pthread_cancel(threadId[1]);

	close(socketCoordinador);

	return 0;
}
