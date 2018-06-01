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

#define PACKAGESIZE 1024
#define HEADER_LENGTH 10
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR "8888"//8080
#define IP_PLANIFICADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR "8080"//8082

//#define OK 1
//#define HUBO_UN_PROBLEMA -1
#define LISTO 2

//Codigos de las operaciones:
#define ENVIA_ORDEN 1
#define ENVIA_SENTENCIA 1
#define RESULTADO_EJECUCION_SENTENCIA 2
#define RESPUESTA_EJECUCION_SENTENCIA 2

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
