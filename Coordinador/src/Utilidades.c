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

void atenderESI(){
}

void atenderInstancia(){
}

void atenderPlanificador(){
}


