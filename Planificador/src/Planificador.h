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
#include <errno.h>			//errorno
#include <fcntl.h>			// std no block


/**********************************************/
/* DEFINES									  */
/**********************************************/
#define IP_COORD "127.0.0.1"
#define PORT_COORD "8080"
#define PORT_ESCUCHA 8080
#define STDIN 0
#define TRUE 1
#define FALSE 0
#define MAX_CLIENTES 20
#define MAX_LINEA 255
#define NO_SOCKET -1



/**********************************************/
/* STRUCTS									  */
/**********************************************/
typedef struct {
  int socket;
  struct sockaddr_in addres;

} t_conexion_esi;

/**********************************************/
/* DATOS GLOBALES							  */
/**********************************************/
t_conexion_esi conexiones_esi[MAX_CLIENTES];

//Enumeracion de los comandos de la consola
enum comandos { pausar, continuar, bloquear, desbloquear, listar, kill, status, deadlock, salir };


/**********************************************/
/* FUNCIONES								  */
/**********************************************/
int conectar_coordinador(char * ip, char * port);
void *consola();
int iniciar_servidor(unsigned short port);
int comando_consola(char * buffer);
void stdin_no_bloqueante(void);

int obtener_key_comando(char* comando);
void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2);//Falta implementarla
int read_from_stdin(char *read_buffer, size_t max_len);

void pausar_consola(void);
void continuar_consola(void);
void bloquear_clave(char* clave , char* id);
void desbloquear_clave(char* clave, char* id);
void listar_recurso(char* recurso);
void kill_id(char* id);
void status_clave(char* clave);
void deadlock_consola(void);

void inicializar_conexiones_esi(void);
int atender_nuevo_esi(int serv_socket);
int recibir_mensaje_esi(int esi_socket);
int cerrar_conexion_esi(t_conexion_esi * esi);
