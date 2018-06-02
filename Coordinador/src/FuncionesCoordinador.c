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

t_esi_operacion_sin_puntero * armar_esi_operacion_sin_puntero(t_sentencia * sentencia){
	t_esi_operacion_sin_puntero * op_sin_punt = malloc(sizeof(t_esi_operacion_sin_puntero));
	strncpy(op_sin_punt->clave, sentencia->clave,40);
	op_sin_punt->keyword = sentencia->keyword;
	op_sin_punt->tam_valor = sizeof(sentencia->valor);
	op_sin_punt->pid = sentencia->pid;
	return op_sin_punt;
}

t_sentencia * armar_sentencia(t_esi_operacion_sin_puntero * op_sin_punt, char * valor){
	t_sentencia * sentencia_con_punteros = malloc(sizeof(t_sentencia));
	strncpy(sentencia_con_punteros->clave, op_sin_punt->clave,40);
	sentencia_con_punteros->valor = valor;
	sentencia_con_punteros->keyword = op_sin_punt->keyword;
	sentencia_con_punteros->pid = op_sin_punt->pid;

}
