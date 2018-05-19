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
#include <pthread.h>		// Manejo de hilos
#include <readline/readline.h> // Para usar readline
#include <readline/history.h> // Para usar readline
#include <commons/string.h> // Para manejo de strings
#include <commons/collections/list.h> // Para manejo de strings
#include <errno.h>			//errorno
#include <fcntl.h>			// std no block


/**********************************************/
/* DEFINES									  */
/**********************************************/
#define IP_COORD "127.0.0.1"
#define PORT_COORD "8888"
#define PORT_ESCUCHA 8080
#define STDIN 0
#define TRUE 1
#define FALSE 0
#define MAX_CLIENTES 20
#define MAX_LINEA 255
#define NO_SOCKET -1
#define ESTIMACION_INICIAL 5
#define ALGORITMO_PLAN_FIFO "FIFO"
#define ALGORITMO_PLAN_SJFCD "SJF-CD"
#define ALGORITMO_PLAN_SJFSD "SJF-SD"
#define ALGORITMO_PLAN_HRRN "HRRN"

#define OPERACION_ESI_OK 1
#define OPERACION_ESI_OK_FINAL 2
#define OPERACION_ESI_BLOQUEADA -1

//Enumeracion de los comandos de la consola
enum comandos { pausar, continuar, bloquear, desbloquear, listar, kill, status, deadlock, salir,
				mostrar, ejecucion};
enum proceso_tipo { esi, instancia, planificador, coordinador };
enum estados { ready, exec, blocked, finished };


//TODO completar a medida que surjan operaciones
//enum operacion { };

/**********************************************/
/* STRUCTS									  */
/**********************************************/
struct conexion_esi {
  int socket;
  struct sockaddr_in addres;

};
typedef struct conexion_esi t_conexion_esi;

//TODO completar cuando sean necesarios nuevos campos, OJO tambien completar al crear el esi
struct pcb_esi {
	int pid;
	int estado;
	int estimacion;
	int estimacion_ant;
	int instruccion_actual;
	t_conexion_esi conexion;
};
typedef struct pcb_esi t_pcb_esi;

struct content_header {
	int proceso_tipo;
	int operacion;
	int cantidad_a_leer;
};
typedef struct __attribute__((packed)) content_header t_content_header  ;

struct config{
	char puerto_escucha[5];
	char algoritmo[6];
	int desalojo;
	int alfa;
	int estimacion_inicial;
	char* ip_coordinador;
	char puerto_coordinador[5];
	char ** claves_bloqueadas;
};

struct claves_bloqueadas{
	char * clave;
	int pid;
};
typedef struct clave_bloqueadas t_claves_bloqueadas;

/**********************************************/
/* DATOS GLOBALES							  */
/**********************************************/
t_conexion_esi conexiones_esi[MAX_CLIENTES];

t_list * l_listos;
t_list * l_bloqueados;
t_list * l_terminados;
t_list * claves_bloqueadas;
t_pcb_esi * l_ejecucion = NULL;

int esi_pid = 0;

struct config config;

/**********************************************/
/* FUNCIONES								  */
/**********************************************/
int conectar_coordinador(char * ip, char * port);
int iniciar_servidor(unsigned short port);
void stdin_no_bloqueante(void);
void crear_listas_planificador(void);
void terminar_planificador(void);
void planificar(void);
void obtener_proximo_ejec(void);
void desalojar_ejec(void);

//Utilidades para la consola
int comando_consola(char * buffer);
int obtener_key_comando(char* comando);
void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2);//Falta implementarla
int read_from_stdin(char *read_buffer, size_t max_len);
void *consola();

//Funciones de la consola
void pausar_consola(void);
void continuar_consola(void);
void bloquear_clave(char* clave , char* id);
void desbloquear_clave(char* clave, char* id);
void listar_recurso(char* recurso);
void kill_id(char* id);
void status_clave(char* clave);
void deadlock_consola(void);
void mostrar_lista(char* lista);
void mostrar_esi_en_ejecucion(void);

//Manejo de esi
void inicializar_conexiones_esi(void);
int atender_nuevo_esi(int serv_socket);
int recibir_mensaje_esi(int esi_socket);
int cerrar_conexion_esi(t_conexion_esi * esi);

t_pcb_esi * crear_esi(t_conexion_esi conexion);
int destruir_esi(t_pcb_esi * esi);
void mostrar_esi(t_pcb_esi * esi);

//Manejo de coordinador
int recibir_mensaje_coordinador(int coord_socket);
int cerrar_conexion_coord(int coord_socket);
