/*
 * Utilidades.h
 *
 *  Created on: 5 may. 2018
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
#include <parsi/parser.h>

#define PACKAGESIZE 1024
#define HEADER_LENGTH 10

#define IP_COORDINADOR "127.0.0.1"
#define PUERTO_COORDINADOR "8080"

#define IP_PLANIFICADOR "127.0.0.1"
#define PUERTO_PLANIFICADOR "8082"


#endif /* SRC_UTILIDADES_H_ */
