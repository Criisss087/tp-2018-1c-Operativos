/*
 * Utilidades.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "funcionesInstancia.c"
#include "Utilidades.h"
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

void cargarArchivoConfiguracion(char * path){
	log_info(logger,"cargar archivo configuracion");
	t_config * config_file = config_create(path);
	log_info(logger,"null file? : %d", config_file!=NULL);
	log_info(logger,"cant de keys: %d", config_keys_amount(config_file));
	if (config_has_property(config_file,ARCH_CONFIG_ALGORITMO_DISTRIBUCION)){
		ALGORITMO_DISTRIBUCION = config_get_string_value(config_file, ARCH_CONFIG_ALGORITMO_DISTRIBUCION);
		log_info(logger,"Se obtuvo configuración 'Algoritmo de distribución': %s", ALGORITMO_DISTRIBUCION);
	}
	if (config_has_property(config_file,ARCH_CONFIG_CANTIDAD_ENTRADAS)){
		CANT_MAX_ENTRADAS = config_get_int_value(config_file, ARCH_CONFIG_CANTIDAD_ENTRADAS);
		log_info(logger,"Se obtuvo configuración 'Cantidad de entradas': %d", CANT_MAX_ENTRADAS);
	}
	if (config_has_property(config_file,ARCH_CONFIG_PUERTO)){
		PUERTO = config_get_int_value(config_file, ARCH_CONFIG_PUERTO);
		log_info(logger,"Se obtuvo configuración 'Puerto': %d", PUERTO);
	}


}

void seteosIniciales(char * path){

	logger = log_create("log_coordinador.txt","Coordinador",true, LOG_LEVEL_INFO);

	//char * path = "config.txt";
		//cargarArchivoConfiguracion(path);
		ALGORITMO_DISTRIBUCION = EQUITATIVE_LOAD;
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
