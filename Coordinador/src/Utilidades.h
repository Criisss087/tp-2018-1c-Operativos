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
#include <commons/collections/list.h>

#define IP "127.0.0.1"
#define PUERTO "8080"
#define BACKLOG 10			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

//***Cod Procesos
#define ESI 1
#define PLANIFICADOR 3
#define COORDINADOR 4
#define INSTANCIA 2

//***

//***Cod ops
#define ESI_COORDINADOR_SENTENCIA 1
#define COORDINADOR_ESI_RESULTADO_EJECUCION_SENTENCIA 2

#define INSTANCIA_COORDINADOR_CONEXION 1
#define COORDINADOR_INSTANCIA_CONFIG_INICIAL 2
#define COORDINADOR_INSTANCIA_SENTENCIA 3

#define PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION 1


//***

//*** Enums
	//Para rta de planificador sobre estado de clave
enum{
	CORRECTO,
	CLAVE_BLOQUEADA
};
//***

struct content_header {
	int proceso_origen;
	int proceso_receptor;
	int operacion;
	size_t cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

typedef struct{
	int keyword;
	char clave[40];
	int tam_valor;
	int pid;
} __attribute__((packed)) t_esi_operacion_sin_puntero;

typedef struct{
	int socket;
	int id;
	char * nombre;
} t_instancia;

typedef struct{
	char * valor;
	char clave[40];
	int keyword;
	int pid;
} t_sentencia;

typedef struct {
	int cantTotalEntradas;
	int tamanioEntradas;
} __attribute__((packed)) t_configTablaEntradas;

//*** Estructuras internas de Coordinador
#define EQUITATIVE_LOAD 1
t_log * logger;
t_list * lista_instancias;
int id_counter = 0;
int ALGORITMO_DISTRIBUCION;
signed int indice_actual_lista; //que item de la lista fue el ultimo al que se asigno trabajo
t_instancia PROCESO_PLANIFICADOR;
int total_hilos = 0; //borrable
int hay_instancias = 0; //No se porque si uso lista.element_count tira segmentation fault. que mierda pasa la concha de la lora.
int debug_var = 1;
//***

#include "Utilidades.c""
#endif /* SRC_UTILIDADES_H_ */
