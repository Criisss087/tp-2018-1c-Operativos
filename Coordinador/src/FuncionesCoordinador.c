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
	log_info(logger,"Cargando archivo de configuración...");
	t_config * config_file = config_create(path);
	log_info(logger,"Se cargó archivo? : %d", config_file!=NULL);

	if (config_has_property(config_file,ARCH_CONFIG_ALGORITMO_DISTRIBUCION)){
		char * algo_dist = strdup(config_get_string_value(config_file, ARCH_CONFIG_ALGORITMO_DISTRIBUCION));
		if (string_equals_ignore_case(algo_dist, "EL")){
			//Equitative Load
			ALGORITMO_DISTRIBUCION = EQUITATIVE_LOAD;
			log_info(logger,"Se obtuvo configuración 'Algoritmo de distribución': %s - %d", algo_dist, ALGORITMO_DISTRIBUCION);
		}
		else if (string_equals_ignore_case(algo_dist, "KE")){
			//Key Explicit
			ALGORITMO_DISTRIBUCION = KEY_EXPLICIT;
			log_info(logger,"Se obtuvo configuración 'Algoritmo de distribución': %s - %d", algo_dist, ALGORITMO_DISTRIBUCION);
		}
		else if (string_equals_ignore_case(algo_dist, "LSU")){
			//Least Space Used
			ALGORITMO_DISTRIBUCION = LEAST_SPACE_USED;
			log_info(logger,"Se obtuvo configuración 'Algoritmo de distribución': %s - %d", algo_dist, ALGORITMO_DISTRIBUCION);
		}else {
			log_info(logger,"No se reconoció el algoritmo especificado en el archivo de configuración. Se setea por default EL.");}

	}else log_info(logger,"No se encontró algoritmo especificado en el archivo de configuración. Se setea por default EL.");

	if (config_has_property(config_file,ARCH_CONFIG_CANTIDAD_ENTRADAS)){
		CANT_MAX_ENTRADAS = config_get_int_value(config_file, ARCH_CONFIG_CANTIDAD_ENTRADAS);
		log_info(logger,"Se obtuvo configuración 'Cantidad de entradas': %d", CANT_MAX_ENTRADAS);
	}else log_info(logger,"No se encontró cantidad de entradas especificadas en el archivo de configuración. Se setea por default 50.");

	if (config_has_property(config_file,ARCH_CONFIG_PUERTO)){
		PUERTO = strdup(config_get_string_value(config_file, ARCH_CONFIG_PUERTO));
		log_info(logger,"Se obtuvo configuración 'Puerto': %s", PUERTO);
	}else log_info(logger,"No se encontró puerto especificado en el archivo de configuración. Se setea por default 8888.");

	if (config_has_property(config_file,ARCH_CONFIG_TAMANIO_ENTRADAS)){
		TAMANIO_ENTRADAS = config_get_int_value(config_file, ARCH_CONFIG_TAMANIO_ENTRADAS);
		log_info(logger,"Se obtuvo configuración 'Tamaño de entradas': %d", TAMANIO_ENTRADAS);
	}else log_info(logger,"No se encontró tamaño de entradas especificado en el archivo de configuración. Se setea por default 300.");

	if (config_has_property(config_file,ARCH_CONFIG_RETARDO)){
		RETARDO = config_get_int_value(config_file, ARCH_CONFIG_RETARDO);
		log_info(logger,"Se obtuvo configuración 'Retardo': %d", RETARDO);
	}else log_info(logger,"No se encontró retardo especificado en el archivo de configuración. Se setea por default 0.");

	config_destroy(config_file);
}

void inicializarLogger(){
	logger = log_create("log_coordinador.txt","Coordinador",true, LOG_LEVEL_INFO);
	logger_operaciones = log_create("log_operaciones_coordinador.txt","Coordinador",false, LOG_LEVEL_INFO);
}
void seteosIniciales(char *path){
	inicializarLogger();
	if (path != NULL){
		cargarArchivoConfiguracion(path);
		}
	else {
		log_warning(logger,"Configuraciones cargadas por defecto");
	}
	lista_instancias = list_create();
	lista_claves = list_create();
	indice_actual_lista = -1; //TODO usar la funcion list_size para ver si mostrar o no el error
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
	op_sin_punt->tam_valor = strlen(sentencia->valor);
	op_sin_punt->pid = sentencia->pid;
	return op_sin_punt;
}

t_sentencia * armar_sentencia(t_esi_operacion_sin_puntero * op_sin_punt, char * valor){

	t_sentencia * sentencia_con_punteros = malloc(sizeof(t_sentencia));
	strncpy(sentencia_con_punteros->clave, op_sin_punt->clave,40);
	sentencia_con_punteros->valor = strdup(valor);
	sentencia_con_punteros->keyword = op_sin_punt->keyword;
	sentencia_con_punteros->pid = op_sin_punt->pid;
	return sentencia_con_punteros;

}

void log_operacion_esi(t_sentencia * sentencia, t_log * logger){
	char * keyw;
	if (sentencia->keyword == GET) {keyw = strdup("GET");}
	else if(sentencia->keyword == SET) {keyw = strdup("SET");}
	else {keyw = strdup("STORE");}

	log_info(logger, "ESI %d = %s %s %v ",sentencia->pid,keyw,sentencia->clave,sentencia->valor);
	free(keyw);
}
void log_error_operacion_esi(t_sentencia * sentencia, int puedoEnviar){
	char * keyw;
	if (sentencia->keyword == GET) {keyw = strdup("GET");}
	else if(sentencia->keyword == SET) {keyw = strdup("SET");}
	else {keyw = strdup("STORE");}
	char * error;
	if (puedoEnviar == ABORTAR) {error = strdup("ABORTAR");}
	else {error = strdup("CLAVE BLOQUEADA");}

	log_info(logger_operaciones, "ESI %d = %s %s %v - ERROR: %s",sentencia->pid,keyw,sentencia->clave,sentencia->valor, error);
	free(keyw);
	free(error);
}
