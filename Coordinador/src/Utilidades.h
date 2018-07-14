/*
 * Utilidades.h
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_UTILIDADES_H_
#define SRC_UTILIDADES_H_

#include <redis_lib.h>
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
#include <commons/config.h>
#include <semaphore.h>
#include <signal.h>			// Señales

#define IP "0.0.0.0"

#define BACKLOG 50			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo

#define CANTIDAD_LETRAS 25

//***Cod ops
#define ESI_COORDINADOR_SENTENCIA 1
#define COORDINADOR_ESI_RESULTADO_EJECUCION_SENTENCIA 2

#define INSTANCIA_COORDINADOR_CONEXION 1
#define COORDINADOR_INSTANCIA_CONFIG_INICIAL 2
#define COORDINADOR_INSTANCIA_SENTENCIA 3
#define INSTANCIA_COORDINADOR_RTA 4
#define COORDINADOR_INSTANCIA_CLAVES 5
#define COORDINADOR_INSTANCIA_COMPACTACION 6
#define COORDINADOR_INSTANCIA_CHEQUEO_CONEXION -1

#define PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION 1
#define COORD_PLANIFICADOR_OPERACION_CONSULTA_CLAVE_COORD 2
#define PLANIF_COORD_OPERACION_RES_CLAVE_COORD 3
#define PLANIFICADOR_COORDINADOR_CMD_STATUS 4

//Codigos de las operaciones de esi:
#define ENVIA_ORDEN 1
#define ENVIA_SENTENCIA 1
#define RESULTADO_EJECUCION_SENTENCIA 2
#define RESPUESTA_EJECUCION_SENTENCIA 2
//***

//*** Nombres claves de archivo de configuración
#define ARCH_CONFIG_ALGORITMO_DISTRIBUCION "algoritmo"
#define ARCH_CONFIG_PUERTO "puerto"
#define ARCH_CONFIG_TAMANIO_ENTRADAS "tamanio_entradas"
#define ARCH_CONFIG_CANTIDAD_ENTRADAS "cantidad_entradas"
#define ARCH_CONFIG_RETARDO "retardo"
//***

//***
struct status_clave{
	int tamanio_valor;
	int tamanio_instancia_nombre;
	int cod;
};
//0 = coord no tiene la clave, 1=inst caida, 2= inst simulada, 3=	correcto, 4= instancia no tiene la clave
//Devolver -1 en tamanio_valor cuando no tiene asociada una instancia la clave
typedef struct status_clave t_status_clave;
//TODO: pasar a redis_lib
enum {COORDINADOR_SIN_CLAVE, INSTANCIA_CAIDA, INSTANCIA_SIMULADA, CORRECTO_CONSULTA_VALOR, INSTANCIA_SIN_CLAVE};

enum tipo_logueo { escribir, loguear, escribir_loguear, l_trace, l_debug, l_info, l_warning, l_error, l_esi};

typedef struct {
	char * valor;
	char * nombre_instancia;
	int tamanio_valor;
	int tamanio_instancia_nombre;
	int cod;
} t_status_clave_interno;

typedef struct{
	int socket;
	int id;
	char * nombre;
	int entradas_libres;
	int flag_thread;
} t_instancia;

typedef struct{
	char clave[40];
	t_instancia * instancia;
} t_clave;

typedef struct{
	char * valor;
	char clave[40];
	int keyword;
	int pid;
} t_sentencia;

typedef struct {
	int cod;
	t_instancia * instancia;
	char * valor;
} rta_envio; //Struct para uso interno del coordinador, no se envia ni se recibe de ningun proceso ajeno

typedef struct{
	int resultado_del_parseado;
} respuesta_coordinador;//para el esi

typedef struct {
	int pid;
	char clave[40];
	int sentencia;
} t_consulta_bloqueo;//para el planif

//****** Estructuras internas de Coordinador
//Semaforos
pthread_mutex_t mutexInstancias;
sem_t semInstancias;
sem_t semInstanciasFin;
sem_t semInstanciasTodasFin;
pthread_mutex_t bloqueo_de_Instancias;

pthread_mutex_t consulta_planificador;
pthread_mutex_t consulta_planificador_terminar;
pthread_mutex_t lock_sentencia_global;
//Inicializo la variable para encontrar el error con los semaforos
int rdo_consulta_planificador = -1;
t_sentencia * sentencia_global;

//Algoritmos
#define LEAST_SPACE_USED 0
#define EQUITATIVE_LOAD 1
#define KEY_EXPLICIT 2

//TODO cmabiar todos los llamados a siguienteINstanciaSegunAlgoritmo para que se mande un segundo parametro:
enum {SIMULAR, ASIGNAR};
enum {CONECTADO, DESCONECTADO};
//CONFIG

int ALGORITMO_DISTRIBUCION = EQUITATIVE_LOAD;
char * PUERTO = "8888";
int TAMANIO_ENTRADAS = 300;
int CANT_MAX_ENTRADAS = 50;
int RETARDO = 0; //ms
char * PUERTO_ESCUCHA_PETICION_STATUS = "42578";

t_log * logger;
t_log * logger_operaciones;
t_list * lista_instancias;
t_list * lista_claves;

int * rta1; //la hago global para poder hacer el free
int id_counter = 0;

int indice_actual_lista; //que item de la lista fue el ultimo al que se asigno trabajo
t_instancia PROCESO_PLANIFICADOR;
int total_hilos = 0; //borrable
int hay_instancias = 0; //No se porque si uso lista.element_count tira segmentation fault. que mierda pasa la concha de la lora.
int GLOBAL_SEGUIR = 1;
//***

void logger_coordinador(int tipo_esc, int tipo_log, const char* mensaje, ...);

#include "Utilidades.c"
#include "FuncionesCoordinador.c"
#endif /* SRC_UTILIDADES_H_ */
