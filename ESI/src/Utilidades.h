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
#include <redis_lib.h>

//Esto es para los dummys...
#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

//Respuestas del coordinador
//#define OK 1
//#define HUBO_UN_PROBLEMA -1
#define LISTO 2
#define ABORTAR 3

//Codigos de las operaciones:
#define RECIBIR_ORDEN_EJECUCION 1
#define ENVIAR_RESULTADO_PLANIF 2
#define ENVIAR_SENTENCIA_COORD 1
#define RECIBIR_RESULTADO_SENTENCIA_COORD 2

//Structs
typedef struct{
	int resultado_del_parseado;
} respuesta_coordinador;

struct confirmacion_sentencia{
	int pid;
	int ejec_anterior;
	int resultado;
};
typedef struct confirmacion_sentencia t_confirmacion_sentencia;


//Funciones
struct addrinfo* crear_addrinfo(char *, char *);
int conectar_coordinador(char *, char *);
int conectar_planificador(char *, char *);

#include "Funciones.c"
#endif /* SRC_UTILIDADES_H_ */
