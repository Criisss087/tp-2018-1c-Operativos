/*
 * Utilidades.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

void crear_hilo_conexion(int socket, void(*funcion_a_ejecutar)(int)){
	pthread_t hilo;
	pthread_create(&hilo,NULL,*funcion_a_ejecutar,socket);
	pthread_detach(&hilo);
}

void administrarSentencia(t_esi_operacion_sin_puntero *);
void interpretarOperacionInstancia(t_content_header *, int);
void interpretarOperacionPlanificador(t_content_header *, int);
void interpretarOperacionESI(t_content_header *, int);
void interpretarHeader(t_content_header * , int);
void *escucharMensajesEntrantes(int);

void seteosIniciales(){
	ALGORITMO = EQUITATIVE_LOAD;
	logger = log_create("log_coordinador.txt","Coordinador",true, LOG_LEVEL_INFO);
	t_list * lista_instancias = list_create();
	indice_actual_lista = -1;
	t_instancia inst;

}
