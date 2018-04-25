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
#include <sys/types.h>
#include <netdb.h> 			// Para getaddrinfo
#include <unistd.h> 		// Para close(socket)
#include <pthread.h>		// Manejo de hilos
#include <readline/readline.h> // Para usar readline
#include <readline/history.h> // Para usar readline
#include <commons/string.h> // Para manejo de strings


#define IP "127.0.0.1"
#define PORT "8080"

void *conectar_coordinador();
void *consola();
void *iniciar_servidor();

int obtener_key_comando(char* comando);
void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2);//Falta implementarla

//Enumeracion de los comandos de la consola
enum comandos { pausar, continuar, bloquear, desbloquear, listar, kill, status, deadlock, salir };

void pausar_consola(void);
void continuar_consola(void);
void bloquear_clave(char* clave , char* id);
void desbloquear_clave(char* clave, char* id);
void listar_recurso(char* recurso);
void kill_id(char* id);
void status_clave(char* clave);
void deadlock_consola(void);
