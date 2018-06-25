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

int existe(char *nombre){
	int mismoNombre(t_instancia * instancia){return string_equals_ignore_case(instancia->nombre, nombre);}
	return list_any_satisfy(lista_instancias, *mismoNombre);
}

t_list * getClavesAsignadas(char *nombre){
	int asignada(t_clave * clave){
		return string_equals_ignore_case(clave->instancia->nombre,nombre);
	}
	return list_filter(lista_claves,*asignada);
}

void guardarEnListaDeInstancias(int socketInstancia, char *nombre){
	if (existe(nombre)){
		log_info(logger,"existia instancia");
		//Enviar lista de claves asignadas a la instancia
		t_list * claves_asignadas =  getClavesAsignadas(nombre);
		t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_CLAVES, list_size(claves_asignadas));
		//char array_claves[list_size(claves_asignadas)][40] = calloc(list_size(claves_asignadas),40);
		char array_claves[list_size(claves_asignadas)][40];
		//for (int i=0;list_size(claves_asignadas)>i; i++){array_claves[i] = NULL;}
		int contador = 0;
		void addToArray(t_clave * clave){
			strncpy(array_claves[contador],clave->clave,40);
		}
		list_iterate(claves_asignadas, *addToArray);
		int status_claves = send(socketInstancia, array_claves,40*list_size(claves_asignadas),NULL);
		free(claves_asignadas);
	}else{
		//Agregar instancia en lista de instancias
		hay_instancias++;
		t_instancia * nueva = malloc(sizeof(t_instancia));
		nueva->id= nuevoIDInstancia();
		nueva->socket = socketInstancia;
		nueva->nombre = strdup(nombre);
		list_add(lista_instancias, nueva);
		log_info(logger,"Guardada Instancia: %s", nombre);
	}

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

void loopInstancia(int socketInstancia, char * nombre){
	//Para que quede el hilo esperando para compactar
	//wait instancia semaforo
	int status = 1;
	while (status){
		sem_wait(&semInstancias);
		//enviar orden de compactación
		t_content_header * header = crear_cabecera_mensaje(coordinador, instancia, COORDINADOR_INSTANCIA_COMPACTACION,0);
		int status_head = send(socketInstancia,header,sizeof(t_content_header),0);

		int header_rta_instancia = recv(socketInstancia,header,sizeof(t_content_header), 0);
		t_respuesta_instancia * rta = malloc(sizeof(t_respuesta_instancia));
		int status_rta_instancia = recv(socketInstancia, rta, sizeof(t_respuesta_instancia), 0);
		if (status_head == -1 || header_rta_instancia == -1 || status_rta_instancia == -1){status = -1;}
		else{log_info(logger, "instancia %s rdo compactacion: %d", nombre, rta->rdo_operacion);}

		sem_post(&semInstanciasFin);
		sem_wait(&semInstanciasTodasFin);
		//signal instancia semaforo
	}
}
