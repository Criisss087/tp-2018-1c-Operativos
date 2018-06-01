/*
 * Utilidades.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "funcionesInstancia.c"

void crear_hilo_conexion(int socket, void(*funcion_a_ejecutar)(int)){
	pthread_t hilo;
	pthread_create(&hilo,NULL,*funcion_a_ejecutar,socket);
	pthread_detach(&hilo);
}

void administrarSentencia(t_esi_operacion_sin_puntero *);
void interpretarOperacionInstancia(t_content_header *, int);
void interpretarOperacionPlanificador(t_content_header *, int);
void interpretarOperacionESI(t_content_header *, int);
void interpretarHeader(t_content_header * , int);
void *escucharMensajesEntrantes(int);

void seteosIniciales(){
	ALGORITMO = EQUITATIVE_LOAD;
	logger = log_create("log_coordinador.txt","Coordinador",true, LOG_LEVEL_INFO);
	lista_instancias = list_create();
	indice_actual_lista = -1; //TODO usar la funcion list_size para ver si mostrar o no el error
	t_instancia inst;

}

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	return serverInfo;
}
