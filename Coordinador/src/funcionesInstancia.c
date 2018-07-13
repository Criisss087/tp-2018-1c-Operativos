/*
 * funcionesInstancia.c
 *
 *  Created on: 1 jun. 2018
 *      Author: utnso
 */

char * cual_es_el_string(int caracter){
	switch(caracter){
		case 97:
			return "a";
			break;
		case 98:
			return "b";
			break;
		case 99:
			return "c";
			break;
		case 100:
			return "d";
			break;
		case 101:
			return "e";
			break;
		case 102:
			return "f";
			break;
		case 103:
			return "g";
			break;
		case 104:
			return "h";
			break;
		case 105:
			return "i";
			break;
		case 106:
			return "j";
			break;
		case 107:
			return "k";
			break;
		case 108:
			return "l";
			break;
		case 109:
			return "m";
			break;
		case 110:
			return "n";
			break;
		case 111:
			return "o";
			break;
		case 112:
			return "p";
			break;
		case 113:
			return "q";
			break;
		case 114:
			return "r";
			break;
		case 115:
			return "s";
			break;
		case 116:
			return "t";
			break;
		case 117:
			return "u";
			break;
		case 118:
			return "v";
			break;
		case 119:
			return "w";
			break;
		case 120:
			return "x";
			break;
		case 121:
			return "y";
			break;
		case 122:
			return "z";
			break;
	}
}


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

t_instancia * getInstanciaByName(char * nombre){
	int mismoNombre(t_instancia * instancia){return string_equals_ignore_case(instancia->nombre, nombre);}
	return list_find(lista_instancias,*mismoNombre);
}

void actualizarEntradasLibres(char * nombre, int entradas_libres){
	int actualizarEntradasLibresCondicional(t_instancia * t_i){
		if (string_equals_ignore_case(t_i->nombre, nombre)){
			t_i->entradas_libres = entradas_libres;
		}
	}
	log_warning(logger,"cantidad en instancia: %d",getInstanciaByName(nombre)->entradas_libres);
	list_map(lista_instancias,*actualizarEntradasLibresCondicional);
	log_warning(logger,"cantidad en instancia despuesde actualizar: %d",getInstanciaByName(nombre)->entradas_libres);
}


void actualizarSocketInstancia(char * nombre, int socket){
	int actualizarSocketInstanciaCondicional(t_instancia * t_i){
		if (string_equals_ignore_case(t_i->nombre, nombre)){
			t_i->socket = socket;
		}
	}
	log_warning(logger,"socket de instancia: %d",getInstanciaByName(nombre)->socket);
	list_map(lista_instancias,*actualizarSocketInstanciaCondicional);
	log_warning(logger,"sokcet en instancia despuesde actualizar: %d",getInstanciaByName(nombre)->socket);
}


int existe(char *nombre){
	int mismoNombre(t_instancia * instancia){return string_equals_ignore_case(instancia->nombre, nombre);}
	return list_any_satisfy(lista_instancias, *mismoNombre);
}

t_list * getClavesAsignadas(char *nombre){
	int asignada(t_clave * clave){
		if (clave->instancia!=NULL){
			return string_equals_ignore_case(clave->instancia->nombre,nombre);
		}
		return 0;
	}
	return list_filter(lista_claves,*asignada);
}

void guardarEnListaDeInstancias(int socketInstancia, char *nombre){
	if (existe(nombre)){
		log_info(logger,"existia instancia");
		actualizarSocketInstancia(nombre,socketInstancia);
		log_error(logger,"prueba reincorporar isntancia con clave1");
		//Enviar lista de claves asignadas a la instancia
		t_list * claves_asignadas =  getClavesAsignadas(nombre);
		log_error(logger,"prueba reincorporar isntancia con clave2");
		t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_CLAVES, list_size(claves_asignadas));
		//char array_claves[list_size(claves_asignadas)][40] = calloc(list_size(claves_asignadas),40);
		char array_claves[list_size(claves_asignadas)][40];
		log_error(logger,"prueba reincorporar isntancia con clave3");
		//for (int i=0;list_size(claves_asignadas)>i; i++){array_claves[i] = NULL;}
		int contador = 0;
		log_error(logger,"prueba reincorporar isntancia con clave4");
		void addToArray(t_clave * clave){
			strncpy(array_claves[contador],clave->clave,40);
			contador++;
		}
		log_error(logger,"prueba reincorporar isntancia con clave5");
		list_iterate(claves_asignadas, *addToArray);
		log_info(logger,"despues de armar array claves--");
		for(int i = 0; list_size(claves_asignadas)> i;i++){log_warning(logger,"claves de antes en instancia: %s",array_claves[i]);}

		int status_h = send(socketInstancia, header, sizeof(t_content_header),0);
		//int status_claves = send(socketInstancia, array_claves,40*list_size(claves_asignadas),NULL);

		char * cl = malloc(40);
		log_info(logger,"dpss malloc - list_size claves %d",list_size(claves_asignadas) );
		int i = 0;
		int d = list_size(claves_asignadas);
		log_info(logger,"i:%d, d:%d",i,d);
		while( i < d){
			printf("enviando '%s'", array_claves[i]);
			memset(cl,0,40);
			strncpy(cl,array_claves[i],40);
			send(socketInstancia,cl,40,0);
			i++;
		}

		//espero respuesta de instancia para ver cuanto espacio libre tiene y actualizar
		t_content_header * head_inst = malloc(sizeof(t_content_header));
		int status_recv_header = recv(socketInstancia, head_inst, sizeof(t_content_header),0);

		t_respuesta_instancia * rta_inst = malloc(sizeof(t_respuesta_instancia));

		int status_recv_rta_instancia = recv(socketInstancia, rta_inst, sizeof(t_content_header),0);

		actualizarEntradasLibres(nombre,rta_inst->entradas_libres);
		/*
		for (int i = 0; list_size(claves_asignadas)>i;i++){
			printf("enviando '%s'", array_claves[i]);
			memset(cl,0,40);
			strncpy(cl,array_claves[i],40);
			send(socketInstancia,cl,40,0);
		}*/
		log_info(logger,"dpss for" );
		free(cl);

		free(claves_asignadas);
	}else{
		//Agregar instancia en lista de instancias
		hay_instancias++;
		t_instancia * nueva = malloc(sizeof(t_instancia));
		nueva->id= nuevoIDInstancia();
		nueva->socket = socketInstancia;
		nueva->nombre = strdup(nombre);
		nueva->entradas_libres = CANT_MAX_ENTRADAS;
		nueva->flag_thread = 0;
		list_add(lista_instancias, nueva);
		log_info(logger,"Guardada Instancia: %s", nombre);
	}

}

t_instancia * siguienteEqLoad(int cod){
	//Quizá convenga hacer que recorra la lista de atrás para adelante,
	//para no estar siempre con las nuevas dejando las viejas para lo último.
	//Como no va a haber la misma cantidad de conexiones con instancias como con esis por ejemplo, no creo que sea tan necesario.
	if (cod == ASIGNAR){
		int cant = list_size(lista_instancias);
		indice_actual_lista++;
		if (indice_actual_lista>cant){indice_actual_lista = indice_actual_lista - cant;}
		int siguiente = indice_actual_lista % cant;
		return list_get(lista_instancias, siguiente);
	}
	else{
		int cant = list_size(lista_instancias);
		int indice_actual_lista_aux = indice_actual_lista;
		indice_actual_lista_aux++;
		if (indice_actual_lista_aux>cant){indice_actual_lista_aux = indice_actual_lista_aux - cant;}
		int siguiente = indice_actual_lista_aux % cant;
		return list_get(lista_instancias, siguiente);
	}
}

t_instancia * siguienteLSU(){
	t_instancia * siguiente = list_get(lista_instancias, 0);
	int maximo = siguiente->entradas_libres;
	int indiceDelMaximo = 0;
	int indiceContador = 0;

	void getInstanciaMayorEspacioLibre(t_instancia * t){
		if (t->entradas_libres > maximo){
			maximo = t->entradas_libres;
			indiceDelMaximo = indiceContador;
		}
		indiceContador++;
	}

	list_iterate(lista_instancias,*getInstanciaMayorEspacioLibre);
	return list_get(lista_instancias,indiceDelMaximo);
}

t_instancia * siguienteKeyExplicit(char clave[40]){

	int cantidad_instancias = list_size(lista_instancias);
	int caracter = 97; // a
	char * palabra = strdup(clave);
	char * letra = cual_es_el_string(caracter);

	if(cantidad_instancias!=0){

		int rango_letras = CANTIDAD_LETRAS / cantidad_instancias;

		if((CANTIDAD_LETRAS % cantidad_instancias) > 0){
			rango_letras ++;
		}

		for(int j=0;j<cantidad_instancias;j++){

			for(int i=0; i<=rango_letras; i++){

				if(string_starts_with(palabra, letra)){
					return list_get(lista_instancias, j);
				}
/*
 * De agregarse nuevas instancias, las claves previamente almacenadas no se moverán de lugar;
 * pero nuevas claves usarán el nuevo número de instancias para calcular la distribución.
 */
				caracter++;
				letra = cual_es_el_string(caracter);
			}
		}
	}
}

t_instancia * siguienteInstanciaSegunAlgoritmo(char clave[40], int cod){
	if(	list_size(lista_instancias)==0){
			log_error(logger,"No hay Instancias conectadas");
			t_instancia * instancia_error = malloc(sizeof(t_instancia));
			strncpy(instancia_error->nombre,"ERROR",5);
			return instancia_error;
	}

	switch(ALGORITMO_DISTRIBUCION){
		case EQUITATIVE_LOAD:
			return siguienteEqLoad(cod);
			break;
		case LEAST_SPACE_USED:
			return siguienteLSU();
			break;
		case KEY_EXPLICIT:
			return siguienteKeyExplicit(clave);
			break;
		default:
			return siguienteEqLoad(cod);
		}
}

void loopInstancia(t_instancia * inst, char * nombre){
	//Para que quede el hilo esperando para compactar
	//wait instancia semaforo
	inst->flag_thread = 1;
	int status = 1;
	while (status && inst->flag_thread){
		sem_wait(&semInstancias);
		//enviar orden de compactación
		if (chequearConectividadProceso(inst) == CONECTADO){
			t_content_header * header = crear_cabecera_mensaje(coordinador, instancia, COORDINADOR_INSTANCIA_COMPACTACION,0);
			int status_head = send(inst->socket,header,sizeof(t_content_header),0);

			int header_rta_instancia = recv(inst->socket,header,sizeof(t_content_header), 0);
			t_respuesta_instancia * rta = malloc(sizeof(t_respuesta_instancia));
			int status_rta_instancia = recv(inst->socket, rta, sizeof(t_respuesta_instancia), 0);
			if (status_head == -1 || header_rta_instancia == -1 || status_rta_instancia == -1 || status_head == 0 || header_rta_instancia == 0 || status_rta_instancia == 0){status = -1;}
			else{log_info(logger, "instancia %s rdo compactacion: %d", nombre, rta->rdo_operacion);}

			sem_post(&semInstanciasFin);
			//sem_wait(&semInstanciasTodasFin);
			//signal instancia semaforo
		}
		else {
			status = 0;
			sem_post(&semInstanciasFin);
		}
	}
	inst->flag_thread = 0;
}
