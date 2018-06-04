/*
 * Utilidades.h
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */

#ifndef SRC_UTILIDADES_H_
#define SRC_UTILIDADES_H_

#include <stdlib.h>
#include <redis_lib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <commons/collections/list.h> // Para manejo de strings

#define NOMBRE_INSTANCIA "Instancia1"
#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR 8888
#define PACKAGESIZE 1024
#define INTERVALO_DUMP 10 // Intervalo dado en segundos para guardar la tabla de entradas en archivo de texto plano
#define PUNTO_DE_MONTAJE "home/utnso/instanciaX"

#define GET_KEYWORD 0
#define SET_KEYWORD 1
#define STORE_KEYWORD 2

//***Cod ops
#define INSTANCIA_COORDINADOR_CONEXION 1
#define COORDINADOR_INSTANCIA_CONFIG_INICIAL 2
#define COORDINADOR_INSTANCIA_SENTENCIA 3
#define INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA 4


// struct para el envio de nombre de Instancia al Coordinador
typedef struct{
	char nombreInstancia[40];
} __attribute__((packed)) t_info_instancia;

typedef struct{
		int keyword;
		char clave[40];
		char * valor;
	} t_sentencia;

t_configTablaEntradas * configTablaEntradas;

// Lista de indice de Entradas

t_list * l_indice_entradas;

// Estructura de Tabla de indice de Entradas
typedef struct{
	int numeroEntrada;
	char clave[40];
	int tamanioValor;
	bool esAtomica;
	char* puntero;
} __attribute__((packed)) t_indice_entrada;

int numeroEntrada = 0;

char * tablaEntradas;

// Constante auxiliar para la busqueda de una clave en la tabla de indices
char claveBuscada[40];

#endif /* SRC_UTILIDADES_H_ */
