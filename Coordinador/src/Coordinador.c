/*
 ============================================================================
 Name        : Coordinador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */
#include "Utilidades.h"
#include "FuncionesCoordinador.c"

#define TAMANIO_ENTRADAS 8
#define CANT_MAX_ENTRADAS 5

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	return serverInfo;
}

int nuevoIDInstancia(){
	id_counter++;
	return id_counter;
}

void enviarConfiguracionInicial(int socketInstancia){

	log_info(logger,"Enviando configuracion inicial a Instancia");

	t_configTablaEntradas * config = malloc(sizeof(t_configTablaEntradas));
	config->cantTotalEntradas = CANT_MAX_ENTRADAS;
	config->tamanioEntradas= TAMANIO_ENTRADAS;

	t_content_header * header = malloc(sizeof(t_content_header));
	header->cantidad_a_leer = sizeof(t_configTablaEntradas);
	header->proceso_origen = 4;
	header->proceso_receptor = 2;
	header->operacion = COORDINADOR_INSTANCIA_CONFIG_INICIAL;

	int sent_header = send(socketInstancia, header, sizeof(t_content_header),NULL);
	int sent = send(socketInstancia, config, sizeof(t_configTablaEntradas),NULL);

	log_info(logger,"Enviado configuración inicial a Instancia - send: %d",sent);

	free(config);
	free(header);

}

void guardarEnListaDeInstancias(int socketInstancia, char nombre[40]){
	hay_instancias = 1;
	t_instancia * nueva = malloc(sizeof(t_instancia));
	nueva->id= nuevoIDInstancia();
	nueva->socket = socketInstancia;
	strncpy(nueva->nombre, nombre, 40);
	list_add(lista_instancias, nueva);
}

t_instancia * siguienteEqLoad(){
	//Quizá convenga hacer que recorra la lista de atrás para adelante,
	//para no estar siempre con las nuevas dejando las viejas para lo último.
	//Como no va a haber la misma cantidad de conexiones con instancias como con esis por ejemplo, no creo que sea tan necesario.
	int cant = list_size(lista_instancias);
	indice_actual_lista++;
	if (indice_actual_lista>cant){indice_actual_lista = indice_actual_lista - cant;}
	int siguiente = indice_actual_lista % cant;
	return list_get(lista_instancias, siguiente);
}

t_instancia * siguienteInstanciaSegunAlgoritmo(){
	if(	hay_instancias == 0){
			log_error(logger,"No hay Instancias conectadas");
			t_instancia * instancia_error = malloc(sizeof(t_instancia));
			strncpy(instancia_error->nombre,"ERROR",5);
			return instancia_error;
	}
	switch(ALGORITMO){
	case EQUITATIVE_LOAD:
		return siguienteEqLoad();
		break;
	default:
		return siguienteEqLoad();
	}
}

void enviarSentenciaInstancia(t_sentencia * sentencia){
	t_instancia * proxima = siguienteInstanciaSegunAlgoritmo();

	if (string_equals_ignore_case(proxima->nombre, "ERROR")){
		free(proxima);
		return;
	}

	t_content_header * header = malloc(sizeof(t_content_header));
	header->cantidad_a_leer = sizeof(t_content_header);
	header->operacion = COORDINADOR_INSTANCIA_SENTENCIA;
	header->proceso_origen = COORDINADOR;
	header->proceso_receptor = INSTANCIA;

	t_esi_operacion_sin_puntero * s_sin_p = malloc(sizeof(t_esi_operacion_sin_puntero));
	strncpy(s_sin_p->clave, sentencia->clave,40);
	s_sin_p->keyword = sentencia->keyword;
	s_sin_p->tam_valor = sizeof(sentencia->valor);
	s_sin_p->pid = sentencia->pid;

	int header_envio = send(proxima->socket,header,sizeof(t_content_header),NULL);
	int sentencia_envio = send(proxima->socket, sentencia, sizeof(t_esi_operacion_sin_puntero),NULL);
	int valor_envio = send(proxima->socket,sentencia->valor,sizeof(sentencia->valor),NULL);

	free(header);
	free(s_sin_p);
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			//TODO leer packete para obtener nombre.
			char nombre[40];// = recv(...... Hablarlo con Sebastián
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia, nombre);
			break;
		default:
			break;
	}
}

void interpretarOperacionPlanificador(t_content_header * hd, int socketCliente){
	switch(hd->operacion){
	case PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION:
		PROCESO_PLANIFICADOR.id = nuevoIDInstancia();
		PROCESO_PLANIFICADOR.socket = socketCliente;
	}
}

int puedoEnviarSentencia(t_sentencia * sentencia){
	//TODO Preguntarle a Planificador si la clave no esta bloqueada.
	return CORRECTO;
}

void devolverErrorAESI(socketCliente){
	//TODO Devolver resultado de error a ESI
}


void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando operación ESI - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
		;
		//Recibo de ESI sentencia parseada
		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		int cantleida = recv( socketCliente, sentencia, hd->cantidad_a_leer, NULL);

		//Recibo el valor - El esi me lo manda "pelado", directamente el string, ningún struct
		char * valor = malloc(sentencia->tam_valor);
		int valor_status = recv(socketCliente, valor, sentencia->tam_valor,NULL);

		//Armo una variable interna para manejar la sentencia
		t_sentencia * sentencia_con_punteros = malloc(sizeof(t_sentencia));
		strncpy(sentencia_con_punteros->clave, sentencia->clave,40);
		sentencia_con_punteros->valor = valor;
		sentencia_con_punteros->keyword = sentencia->keyword;
		sentencia_con_punteros->pid = sentencia->pid;

		int puedoEnviar = puedoEnviarSentencia(sentencia_con_punteros);
		switch(puedoEnviar){
			case CORRECTO:
				enviarSentenciaInstancia(sentencia_con_punteros);
				break;
			case CLAVE_BLOQUEADA:
				devolverErrorAESI(socketCliente);
				break;
			default:
				break;
		}
		free(sentencia);
		break;
	default:
		//TODO no se reconoció el tipo operación
		break;
	}
}

void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case ESI:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case INSTANCIA:
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case PLANIFICADOR:
		interpretarOperacionPlanificador(hd,socketCliente);
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    log_info(logger, "Cliente conectado. Esperando mensajes:");
    total_hilos++;
    log_info(logger, "total hilos: %d",total_hilos);

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header != 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	log_info(logger, "Recv - Status Header: %d", status_header);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado"); total_hilos--;
    	}
    	else {
    		log_info(logger, "Interpretando header...");
    		interpretarHeader(header, socketCliente);
    	};
   	}
    free(header);
    close(socketCliente);
}

int main()
{
	seteosIniciales();

	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(logger,"Socket de escucha creado %d", listenningSocket);

    // Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    log_info(logger, "Socket de escucha bindeado");
    freeaddrinfo(serverInfo);

    log_info(logger, "Escuchando...");
    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

	while (1){
    	log_info(logger, "Esperando conexiones...");
    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
		log_info(logger, "Conexión recibida - Accept: %d ",socketCliente);

		crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);
	}

    close(listenningSocket);
    log_destroy(logger);
    return 0;


}
