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
#include <unistd.h> // Para el uso de usleep()
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
#include <pthread.h>

#include "CargarArchivoDeConfiguracion.c"

#define PACKAGESIZE 1024

//***Cod ops
#define INSTANCIA_COORDINADOR_CONEXION 1
#define COORDINADOR_INSTANCIA_CONFIG_INICIAL 2
#define COORDINADOR_INSTANCIA_SENTENCIA 3
#define INSTANCIA_COORDINADOR_RESPUESTA_SENTENCIA 4
#define COORDINADOR_INSTANCIA_RECONEXION 5
#define COORDINADOR_INSTANCIA_RECUPERAR_CLAVES 5
#define COORDINADOR_INSTANCIA_COMPACTAR 6

#define COORDINADOR_INSTANCIA_COMPROBAR_CONEXION -1
#define INSTANCIA_COORDINADOR_CONFIRMA_CONEXION_ACTIVA -1

// struct para el envio de nombre de Instancia al Coordinador
typedef struct {
	char nombreInstancia[40];
} __attribute__((packed)) t_info_instancia;

typedef struct {
		int keyword;
		char clave[40];
		char* valor;
	} __attribute__((packed)) t_sentencia;

t_configTablaEntradas * configTablaEntradas;

// Lista de indice de Entradas

t_list * l_indice_entradas;

// Estructura de Tabla de indice de Entradas
typedef struct {
	int numeroEntrada;
	char clave[40];
	int tamanioValor;
	bool esAtomica;
	int nroDeOperacion;
	char* puntero;
} __attribute__((packed)) t_indice_entrada;

// Cantidad de threads requeridos: principal + auxiliar para efectuar DUMP
pthread_t threadId[2];

int numeroEntrada = 0;
int contadorOperacion = 0;
int nroEntradaBaseAux = 0;

int respuestaParaCoordinador;

char * tablaEntradas;

// Funciones utilizadas

void guardarClaveValor(char clave[40], char * valor);

#endif /* SRC_UTILIDADES_H_ */
