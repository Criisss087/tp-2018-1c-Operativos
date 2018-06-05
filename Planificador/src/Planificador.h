/*
 * Planificador.h
 *
 *  Created on: 22 abr. 2018
 *      Author: utnso
 */

#ifndef SRC_PLANIFICADOR_H_
#define SRC_PLANIFICADOR_H_
#endif /* SRC_PLANIFICADOR_H_ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>			// funciones de strings
#include <sys/socket.h>		// Para crear los sockets
#include <sys/select.h>		// Select
#include <sys/types.h>
#include <sys/time.h>		//timeval en select
#include <netdb.h> 			// Para getaddrinfo
#include <unistd.h> 		// Para close(socket)
#include <readline/readline.h> // Para usar readline
#include <readline/history.h> // Para usar readline
#include <commons/string.h> // Para manejo de strings
#include <commons/log.h> // Para Logger
#include <commons/collections/list.h> // Para manejo de listas
#include <errno.h>			//errorno
#include <fcntl.h>			// std no block
#include <redis_lib.h>		// Commons para el TP
#include <semaphore.h>		// Semaforos de pthread
#include <pthread.h>		// Hilos
#include <signal.h>			// Señales

/**********************************************/
/* DEFINES									  */
/**********************************************/
#define IP_COORD "127.0.0.1"
#define PORT_COORD "8888"
#define PORT_ESCUCHA "8080"
#define STDIN 0
#define TRUE 1
#define FALSE 0
#define MAX_CLIENTES 20
#define MAX_LINEA 255
#define NO_SOCKET -1
#define ESTIMACION_INICIAL 5
#define ALPHA 50

#define ALGORITMO_PLAN_FIFO "FIFO"
#define ALGORITMO_PLAN_SJFCD "SJF-CD"
#define ALGORITMO_PLAN_SJFSD "SJF-SD"
#define ALGORITMO_PLAN_HRRN "HRRN"

#define RESULTADO_ESI_OK_SIG 1
#define RESULTADO_ESI_OK_FINAL 2
#define RESULTADO_ESI_ABORTADO 3
#define RESULTADO_ESI_BLOQUEADA -1
#define OPERACION_CONF_SENTENCIA 1
#define OPERACION_RES_SENTENCIA 2
#define OPERACION_HANDSHAKE_COORD 1
#define OPERACION_CONSULTA_CLAVE_COORD 2
#define OPERACION_RES_CLAVE_COORD 3


//Enumeracion de los comandos de la consola
enum comandos { pausar, continuar, bloquear, desbloquear, listar, ckill, status, deadlock, salir,
				mostrar, ejecucion, bloqueos};
//enum procesos { esi, instancia, planificador, coordinador };
enum estados { nuevo, listo, en_ejecucion, bloqueado, terminado };

//Sentencias recibidas desde el Coordinador
enum sentencias { GET, SET, STORE };

//Resultado a enviar al coordinador cuando consulta clave
enum resultado_consulta_bloqueo { CORRECTO, CLAVE_BLOQUEADA, ABORTAR};

//Estado de la ejecución de ESI según comando de la consola (Pausa/Continuar)
enum estado_pausa_ejec { no_pausado, pausado };

/**********************************************/
/* ESTUCTURAS								  */
/**********************************************/
struct conexion_esi {
	int pid;
	int socket;
	struct sockaddr_in addres;
};
typedef struct conexion_esi t_conexion_esi;

struct pcb_esi {
	int pid;
	int estado;
	float estimacion_real;
	float estimacion_actual;
	float estimacion_anterior;
	int instruccion_actual;
	int ejec_anterior;			// 1 Si en la siguiente corrida debe ejectar denuevo la ultima instruccion
	t_conexion_esi * conexion;
	char * clave_bloqueo;
};
typedef struct pcb_esi t_pcb_esi;

struct config{
	char puerto_escucha[5];
	char algoritmo[7];
	int desalojo;
	float alfa;
	int estimacion_inicial;
	char* ip_coordinador;
	char puerto_coordinador[5];
	char ** claves_bloqueadas;
};

struct claves_bloqueadas{
	int pid;
	char * clave;
};
typedef struct claves_bloqueadas t_claves_bloqueadas;

struct confirmacion_sentencia{
	int pid;
	int ejec_anterior;
	int resultado;
};
typedef struct confirmacion_sentencia t_confirmacion_sentencia;

struct consulta_bloqueo{
	int pid;
	char clave[40];
	int sentencia;
};
typedef struct consulta_bloqueo t_consulta_bloqueo;

/**********************************************/
/* DATOS GLOBALES							  */
/**********************************************/
t_conexion_esi conexiones_esi[MAX_CLIENTES];

t_log * logger;
t_list * esi_listos;
t_list * esi_bloqueados;
t_list * esi_terminados;
t_list * claves_bloqueadas;
t_pcb_esi * esi_en_ejecucion = NULL;
t_pcb_esi * esi_por_desalojar = NULL;
t_consulta_bloqueo * clave_a_bloquear_por_set = NULL;
t_consulta_bloqueo * clave_a_desbloquear_por_store = NULL;

int esi_seq_pid = 0;

// Globales para capturar eventos al recibir el resultado de la sentencia del ESI
int bloqueo_en_ejecucion = 0;		// Bloquear al ESI en ejecucion
int desalojo_en_ejecucion = 0;		// Desalojar al ESI en ejecucion por SJF-CD
int bloqueo_por_set = 0;			// Bloquear clave por resultado positivo de SET
int desbloqueo_por_store = 0;		// Desbloquear clave por resultado positivo de STORE

int estado_pausa_por_consola = no_pausado; // Pausa la ejecución de ESI y desaloja al proceso

struct config config;

/*
sem_t sem_ejecucion_esi;
sem_t sem_bloqueo_esi_ejec;
pthread_mutex_t mutex_esi_en_ejecucion;
*/

/**********************************************/
/* FUNCIONES								  */
/**********************************************/
int* conexiones(void);
int conectar_coordinador(char * ip, char * port);
int iniciar_servidor(char *port);
void *consola();
void stdin_no_bloqueante(void);
void crear_listas_planificador(void);
void terminar_planificador(void);
void planificar(void);
void obtener_proximo_ejecucion(void);
void desalojar_ejecucion(void);

//Utilidades para la consola
int consola_derivar_comando(char * buffer);
int consola_obtener_key_comando(char* comando);
void consola_obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2);//Falta implementarla
int consola_leer_stdin(char *read_buffer, size_t max_len);

//Funciones de la consola
void consola_pausar(void);
void consola_continuar(void);
void consola_bloquear_clave(char* clave , char* id);
void consola_desbloquear_clave(char* clave);
void consola_listar_recurso(char* recurso);
void consola_matar_proceso(char* id);
void consola_consultar_status_clave(char* clave);
void consola_consultar_deadlock(void);
void mostrar_lista(char* lista);
void mostrar_esi_en_ejecucion(void);
void mostrar_bloqueos(void);

//Manejo de ESI
void inicializar_conexiones_esi(void);
int atender_nuevo_esi(int serv_socket);
int recibir_mensaje_esi(t_conexion_esi esi);
int cerrar_conexion_esi(t_conexion_esi * esi);
int enviar_confirmacion_sentencia(t_pcb_esi * pcb_esi);
t_pcb_esi * crear_esi(t_conexion_esi * conexion);
int destruir_esi(t_pcb_esi * esi);
void mostrar_esi(t_pcb_esi * esi);
int bloquear_esi_pid(char * clave,int pid);
t_pcb_esi * buscar_esi_en_lista_pid(t_list *lista,int pid);
t_pcb_esi * sacar_esi_de_lista_pid(t_list *lista,int pid);
t_pcb_esi * buscar_esi_bloqueado_por_clave(char* clave);
t_pcb_esi * sacar_esi_bloqueado_por_clave(char* clave);
void ordenar_lista_estimacion(t_list * lista);
int estimar_esi(t_pcb_esi * esi);
int confirmar_bloqueo_ejecucion(void);
int confirmar_desalojo_ejecucion(void);
int confirmar_pausa_por_consola(void);
int finalizar_esi(int pid_esi);

//Manejo de Coordinador
int recibir_mensaje_coordinador(int coord_socket);
int cerrar_conexion_coord(int coord_socket);
int enviar_resultado_consulta(int socket, int resultado);

//Manejo de claves
int bloquear_clave(char* clave , int pid);
int desbloquear_clave(char* clave);
void mostrar_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada);
int destruir_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada);
t_claves_bloqueadas * buscar_clave_bloqueada(char* clave); //, int pid);

void desbloquear_claves_bloqueadas_pid(int pid);
void confirmar_bloqueo_por_set(void);
void confirmar_desbloqueo_por_store(void);

// TODO Eliminar esta función
void crear_claves_bloqueadas_dummy();
