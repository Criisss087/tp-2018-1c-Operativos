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

#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR "8080"

#define IP_PLANIFICADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR "8082"

//Codigos de las operaciones:
#define ENVIA_ORDEN 1
#define ENVIA_SENTENCIA 1
#define RESULTADO_EJECUCION_SENTENCIA 2
#define RESPUESTA_EJECUCION_SENTENCIA 2


struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;
enum procesos { esi, instancia, planificador, coordinador };


struct confirmacion_sentencia{
	int pid;
	int ejec_anterior;
	int resultado;
};
typedef struct confirmacion_sentencia t_confirmacion_sentencia;


typedef struct{
	int keyword;
	char clave[40];
	char valor[40];
	int pid;
} __attribute__((packed)) t_esi_operacion_sin_puntero;

#include "Funciones.c"
#endif /* SRC_UTILIDADES_H_ */
