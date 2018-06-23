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
#include <commons/collections/list.h> // Para manejo de listas
#include <commons/string.h> // Para manejo de strings
#include <sys/mman.h> // Para el uso de mmap()
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>


#include "CargarArchivoDeConfiguracion.c"

#define PACKAGESIZE 1024

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
		char* valor;
	} __attribute__((packed)) t_sentencia;

t_configTablaEntradas * configTablaEntradas;

// Lista de indice de Entradas

t_list * l_indice_entradas;

// Estructura de Tabla de indice de Entradas
typedef struct{
	int numeroEntrada;
	char clave[40];
	int tamanioValor;
	bool esAtomica;
	int nroDeOperacion;
	char* puntero;
} __attribute__((packed)) t_indice_entrada;

int numeroEntrada = 0;
int contadorOperacion = 0;

char * tablaEntradas;

#endif /* SRC_UTILIDADES_H_ */
