/*
 * ESI.h
 *
 *  Created on: 23 jun. 2018
 *      Author: utnso
 */

#ifndef SRC_ESI_H_
#define SRC_ESI_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <parsi/parser.h>
#include <redis_lib.h>
#include <commons/config.h>

//Codigos de las operaciones:
#define RECIBIR_ORDEN_EJECUCION 1
#define ENVIAR_RESULTADO_PLANIF 2
#define RECIBIR_KILL_PLANIF 3
#define ENVIAR_SENTENCIA_COORD 1
#define RECIBIR_RESULTADO_SENTENCIA_COORD 2

//Nombres para el archivo de configuracion
#define ARCH_CONFIG_PUERTO_COORD "PUERTO COORDINADOR"
#define ARCH_CONFIG_PUERTO_PLANIF "PUERTO PLANIFICADOR"
#define ARCH_CONFIG_IP_COORD "IP COORDINADOR"
#define ARCH_CONFIG_IP_PLANIF "IP PLANIFICADOR"

//Structs
typedef struct{
	int resultado_del_parseado;
} respuesta_coordinador;

struct confirmacion_sentencia{
	int pid;
	int ejec_anterior;
	int resultado;
};
typedef struct confirmacion_sentencia t_confirmacion_sentencia;

//Globales
FILE * archivo_a_leer_por_el_ESI;
int serverCoord;
int serverPlanif;
t_confirmacion_sentencia * confirmacion;
char * linea_a_parsear;
t_esi_operacion parsed;

//Configuracion
char * IP_COORDINADOR;
char * PUERTO_COORDINADOR;
char * IP_PLANIFICADOR;
char * PUERTO_PLANIFICADOR;

//Funciones
struct addrinfo* crear_addrinfo(char *, char *);
int conectar_coordinador(char *, char *);
int conectar_planificador(char *, char *);
void finalizar_esi(void);
void mostrar_sentencia(t_esi_operacion_sin_puntero * sentencia, char*valor);
void mostrar_header(t_content_header * content_header);
t_esi_operacion_sin_puntero  *transformarSinPunteroYagregarpID(t_esi_operacion t, int id);
void cargar_archivo_de_config(char *path);
void recibir_orden_planif_para_comenzar(t_content_header * header);
void abrir_script(char *path);
void enviar_linea_parseada_coordinador(t_content_header * header, t_esi_operacion parsed);
void recibir_respuesta_coordinador(t_content_header * header);
void enviar_al_planificador_la_rta_del_coordinador(t_content_header * header);
void esperar_orden_planificador_para_finalizar(void);
void abortar_esi(void);

#endif /* SRC_ESI_H_ */
