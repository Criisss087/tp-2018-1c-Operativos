/*
 * FuncionesInstancia.c
 *
 *  Created on: 6 jun. 2018
 *      Author: utnso
 */

#include "Instancia.h"
// #include "CargarArchivoDeConfiguracion.c"

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

	free(infoInstancia);

}

char * recibirValor(int socket, int tamanioValor) {
		char * valorRecibido = malloc(tamanioValor);
		int statusValorSentencia = recv(socket, valorRecibido,
				tamanioValor, (int) NULL);

		printf("status header: %d \n", statusValorSentencia);
		printf("Valor de Sentencia recibido: \n");
		printf("\tValor: %s\n", valorRecibido);

		return valorRecibido;
}

t_sentencia * construirSentenciaConValor(t_esi_operacion_sin_puntero * sentenciaPreliminar, char * valor) {
		t_sentencia * sentenciaRecibida = malloc(sizeof(t_sentencia));

		strcpy(sentenciaRecibida->clave, sentenciaPreliminar->clave);
		sentenciaRecibida->keyword = sentenciaPreliminar->keyword;
		if(valor!=NULL){
			sentenciaRecibida->valor = strdup(valor);
		}


		printf(
				"Se asigna la sentencia correctamente... Lista para ser procesada...\n");

		return sentenciaRecibida;
}

void imprimirEntrada(t_indice_entrada * entrada) {
	printf(
			"\t| %d        | %s    |   %d    |   %d      |   %p   |     %p    |\n",
			entrada->numeroEntrada, entrada->clave, entrada->tamanioValor,
			entrada->esAtomica, entrada->puntero, entrada->punteroArchivo);
}

void imprimirTablaEntradas() {
	printf("Imprimiendo tabla administrativa:\n");
	printf(
			"\t| N° entrada |     Clave     | Tamanio | Valor Atomico | PunteroValor | PunteroArchivo |\n");
	list_iterate(l_indice_entradas, (void*) imprimirEntrada);
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

int sumarElementosDeLista(t_list * tamanios) {
	int total = 0;

	void sumarElemento(t_indice_entrada * entrada) {
		total = total + entrada->tamanioValor;
	}

	list_iterate(tamanios, (void*) sumarElemento);

	return total;
}

char * obtenerValorCompleto(char * puntero, int tamanio) {
	char * valorCompleto = malloc(tamanio);
	valorCompleto = memcpy(valorCompleto, puntero, tamanio);
	return valorCompleto;
}

int obtenerTamanioTotalDeValorGuardado(t_list * listaDeIndices) {

	int obtenerTamanioDeEntrada(t_indice_entrada * entrada) {
		printf("Obteniendo tamanio de la entrada: %d", entrada->tamanioValor);
		return entrada->tamanioValor;
	}

	int tamanioTotal = sumarElementosDeLista(listaDeIndices);
	printf("Obteniendo tamanio total de valor referenciado en entradas: %d\n",
			tamanioTotal);

	return tamanioTotal;

}

_Bool entradaExistenteEnIndice(int nroEntrada) {
	_Bool entradaOcupada(t_indice_entrada * entrada) {
			return (entrada->numeroEntrada == nroEntrada);
		}
	return (list_any_satisfy(l_indice_entradas, (void *) entradaOcupada));
}

// TODO en lugar de puntero a char debe ser un char[40]
char* obtenerClaveExistenteEnEntrada(int nroEntrada) {
	_Bool entradaOcupada(t_indice_entrada * entrada) {
				return (entrada->numeroEntrada == nroEntrada);
			}
	t_indice_entrada * entrada = list_find(l_indice_entradas, (void*)entradaOcupada);
	return entrada->clave;
}

// TODO en lugar de puntero a char debe ser un char[40]
void eliminarEntradasAsociadasAClave(char * clave) {
	_Bool contieneClave(t_indice_entrada * entrada) {
				return (strcmp(entrada-> clave, clave));
			}
	list_remove_by_condition(l_indice_entradas, (void*)contieneClave);
}

