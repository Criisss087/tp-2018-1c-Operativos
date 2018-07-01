/*
 * redis_lib.h
 *
 *  Created on: 20 may. 2018
 *      Author: utnso
 */

#ifndef REDIS_LIB_H_
#define REDIS_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// Para manejo de strings
#include <sys/socket.h>		// Para crear los sockets
#include <netdb.h> 			// Para getaddrinfo
#include <unistd.h> 		// Para close(socket)

/**********************************************/
/* PROTOCOLO								  */
/**********************************************/
struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

/**********************************************/
/* STRUCTS									  */
/**********************************************/
typedef struct {
	int cantTotalEntradas;
	int tamanioEntradas;
} __attribute__((packed)) t_configTablaEntradas;

typedef struct{
	int keyword;
	char clave[40];
	int tam_valor;
	int pid;
} __attribute__((packed)) t_esi_operacion_sin_puntero;

typedef struct{
	int rdo_operacion;
	int entradas_libres;
} __attribute__((packed)) t_respuesta_instancia;


/**********************************************/
/* ENUMS									  */
/**********************************************/
//Procesos
enum procesos {dummy, esi, instancia, planificador, coordinador };
//Rta Instancia a Coordinador
enum {ERROR_I,EXITO_I,COMPACTAR};
//Rta Coordinador ESI Planificador
enum {CORRECTO, CLAVE_BLOQUEADA, ABORTAR, LISTO};
//Operaciones de sentencias (keywords)
enum{OBTENER_VALOR,SET_,STORE_};


/**********************************************/
/* FUNCIONES								  */
/**********************************************/

/**
* @NAME: crear_listen_socket
* @DESC: Crea un socket escuchando un puerto y lo devuelve. Devuelve -1 si falla la conexion
* @PARAM:	puerto			->	Puerto a escuchar
* 			max_conexiones	->	Maximo de conexiones para listen
*/
int crear_listen_socket(char* puerto, int max_conexiones);

/**
* @NAME: conectar_a_server
* @DESC: Crea un nuevo socket, lo conecta a una ip puerto y lo devuelve. Devuelve -1 si falla la conexion
* @PARAM:	ip				-> 	IP a conectar
* 			puerto			->	Puerto a conectar
*/
int conectar_a_server(char* ip, char* puerto);

/**
* @NAME: crear_cabebera_mensaje
* @DESC: Crea la cabecera para un nuevo mensaje y la devuelve
*/
t_content_header* crear_cabecera_mensaje(int origen, int receptor, int operacion, size_t tamanio);

/**
* @NAME: destruir_cabebera_mensaje
* @DESC: Libera la memoria de la cabecera creada
*/
void destruir_cabecera_mensaje(t_content_header* header);










#endif /* REDIS_LIB_H_ */
