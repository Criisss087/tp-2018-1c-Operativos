/*
 * conexiones.h
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#ifndef UTILIDADES_H_
#define UTILIDADES_H_

#include <stdio.h> // Por dependencia de readline en algunas distros de linux :)
#include <string.h>
#include <stdlib.h> // Para malloc
#include <sys/socket.h> // Para crear sockets, enviar, recibir, etc
#include <netdb.h> // Para getaddrinfo
#include <unistd.h> // Para close
#include <readline/readline.h> // Para usar readline
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#define IP ""
#define PUERTO ""

//Para cuando usemos el logger
t_log * logger;

//Para el protocolo
typedef struct {
  int id;
  int len;
} __attribute__((packed)) ContentHeader;

#endif /* UTILIDADES_H_ */
