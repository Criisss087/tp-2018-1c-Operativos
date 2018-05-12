/*
 * Utilidades.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef SRC_UTILIDADES_H_
#define SRC_UTILIDADES_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IP "127.0.0.1"
#define PUERTO 8080
#define PACKAGESIZE 1024

#define TYPE_INSTANCIA 2

//Codigos de las operaciones:
#define CORDINADOR_ENVIA_SENTENCIA_INSTANCIA 4201
#define INSTANCIA_ENVIA_RESULTADO_COORDINADOR 2401

#define GET_KEYWORD 0
#define SET_KEYWORD 1


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
