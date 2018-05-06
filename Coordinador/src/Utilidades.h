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

#define IP "127.0.0.1"
#define PUERTO "8080"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

//***Cod ops
#define ESI_COORDINADOR_SENTENCIA 1401
#define COORDINADOR_ESI_RESULTADO_EJECUCION_SENTENCIA 4102
//***
void crear_hilo_conexion(int,void(* f)(int));
void *escucharRequests(int);
void atenderESI();
void atenderInstancia();
void atenderPlanificador();

typedef struct {
	int proceso_tipo;
	int operacion;
	int cantidad_a_leer;
	} __attribute__((packed)) ContentHeader;

#include "Utilidades.c"
#endif /* SRC_UTILIDADES_H_ */
