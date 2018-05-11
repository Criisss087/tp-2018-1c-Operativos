/*
 * Utilidades.h
 *
 *  Created on: 5 may. 2018
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
#include <parsi/parser.h>
#include "Funciones.c"

#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR "8080"

#define IP_PLANIFICADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR "8082"

//Codigos de las operaciones:
#define PLANIFICADOR_ENVIA_ORDEN_ESI 3101
#define ESI_ENVIA_COORDINADOR_SENTENCIA 1401
#define COORDINADOR_ENVIA_ESI_RESULTADO_EJECUCION_SENTENCIA 4102
#define ESI_ENVIA_PLANIFICADOR_RESULTADO_EJECUCION_SENTENCIA 1302



typedef struct {
	int proceso_tipo;
	int operacion;
	int cantidad_a_leer;
	} __attribute__((packed)) ContentHeader;


typedef struct{
	int keyword;
	char clave[40];
	char valor[40];
} __attribute__((packed)) t_esi_operacion_sin_puntero;


#endif /* SRC_UTILIDADES_H_ */
