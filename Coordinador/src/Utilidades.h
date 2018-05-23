/*
 * Utilidades.h
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_UTILIDADES_H_
#define SRC_UTILIDADES_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>
#include <commons/string.h>
#include <parsi/parser.h>
#include <commons/collections/list.h>

#define IP "127.0.0.1"
#define PUERTO "8080"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

//***Cod Procesos
#define ESI 1
#define PLANIFICADOR 3
#define COORDINADOR 4
#define INSTANCIA 2

//***

//***Cod ops
#define ESI_COORDINADOR_SENTENCIA 1401
#define COORDINADOR_ESI_RESULTADO_EJECUCION_SENTENCIA 4102
#define INSTANCIA_COORDINADOR_CONEXION 2402
//***

struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

typedef struct{
	int keyword;
	char clave[40];
	char valor[40];
} __attribute__((packed)) t_esi_operacion_sin_puntero;

typedef struct{
	int socket;
	int id;
} t_instancia;

typedef struct{
	int tamanio;
	int cantidad;
} configuracion_inicial;

t_log * logger;

#include "Utilidades.c"
#endif /* SRC_UTILIDADES_H_ */
