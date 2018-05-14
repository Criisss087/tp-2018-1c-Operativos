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

#define NOMBRE_INSTANCIA "Instancia1"
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR 8080
#define PACKAGESIZE 1024
#define CANT_MAX_ENTRADAS 5 // Cantidad maxima de entradas que puede tener la instancia
#define INTERVALO_DUMP 10 // Intervalo dado en segundos para guardar la tabla de entradas en archivo de texto plano
#define PUNTO_DE_MONTAJE "home/utnso/instanciaX"

#define TYPE_INSTANCIA 2

// Codigos de operaciones:
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
} __attribute__((packed)) t_sentencia_sin_puntero;

// Estructura de Tabla de Entradas
typedef struct{
	char clave[40];
	int numeroEntrada;
	int tamanioEntrada;
} __attribute__((packed)) t_entrada;

#endif /* SRC_UTILIDADES_H_ */
