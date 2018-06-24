/*
 * funcionesInstancia.c
 *
 *  Created on: 1 jun. 2018
 *      Author: utnso
 */


int nuevoIDInstancia(){
	id_counter++;
	return id_counter;
}

void enviarConfiguracionInicial(int socketInstancia){

	log_info(logger,"Enviando configuracion inicial a Instancia");

	t_configTablaEntradas * config = malloc(sizeof(t_configTablaEntradas));

	config->cantTotalEntradas = CANT_MAX_ENTRADAS;
	config->tamanioEntradas= TAMANIO_ENTRADAS;

	t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_CONFIG_INICIAL,sizeof(t_configTablaEntradas));

	int sent_header = send(socketInstancia, header, sizeof(t_content_header),0);
	int sent = send(socketInstancia, config, sizeof(t_configTablaEntradas),0);

	log_info(logger,"Enviada configuración inicial a Instancia - send: %d",sent);

	free(config);
	free(header);

}

void guardarEnListaDeInstancias(int socketInstancia, char *nombre){
	hay_instancias++;
	t_instancia * nueva = malloc(sizeof(t_instancia));
	nueva->id= nuevoIDInstancia();
	nueva->socket = socketInstancia;
	nueva->nombre = strdup(nombre);
	list_add(lista_instancias, nueva);
	log_info(logger,"Guardada Instancia: %s", nombre);

}

t_instancia * siguienteEqLoad(){
	//Quizá convenga hacer que recorra la lista de atrás para adelante,
	//para no estar siempre con las nuevas dejando las viejas para lo último.
	//Como no va a haber la misma cantidad de conexiones con instancias como con esis por ejemplo, no creo que sea tan necesario.
	int cant = list_size(lista_instancias);
	indice_actual_lista++;
	if (indice_actual_lista>cant){indice_actual_lista = indice_actual_lista - cant;}
	int siguiente = indice_actual_lista % cant;
	return list_get(lista_instancias, siguiente);
}

t_instancia * siguienteInstanciaSegunAlgoritmo(){
	//TODO usar la funcion list_size para ver si mostrar o no el error
	if(	list_size(lista_instancias)==0){
			log_error(logger,"No hay Instancias conectadas");
			t_instancia * instancia_error = malloc(sizeof(t_instancia));
			strncpy(instancia_error->nombre,"ERROR",5);
			return instancia_error;
	}

	switch(ALGORITMO_DISTRIBUCION){
		case EQUITATIVE_LOAD:
			return siguienteEqLoad();
			break;
		case LEAST_SPACE_USED:
			//TODO
			break;
		case KEY_EXPLICIT:
			//TODO
			break;
		default:
			return siguienteEqLoad();
		}
}
