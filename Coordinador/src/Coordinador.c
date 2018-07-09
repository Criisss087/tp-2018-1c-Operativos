/*
 ============================================================================
 Name        : Coordinador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */
#include "Utilidades.h"
#include "FuncionesCoordinador.c"

rta_envio enviarSentenciaInstancia(t_sentencia * sentencia){

	rta_envio rta;
	rta.instancia = malloc(sizeof(t_instancia));

	t_instancia * proxima;
	/*
	switch(sentencia->keyword){
	case SET:
		//asignar instancia
		break;
	case STORE:
		break;
	default:
		//GET - Sé que no tiene que ir a la instancia. Voy a usar este código cuando necesite sabes el valor de la clave
		break;
	}*/

	//**
	log_warning(logger,"prueba matando instancia");
	int tiene_clave(t_clave * clObj){
					return (strcmp(sentencia->clave, clObj->clave)==0);
				}

		t_clave * instancias_con_clave = list_filter(lista_claves,(void*)tiene_clave);
		if (list_size(instancias_con_clave)>1){log_error(logger,"Más de una instancia tiene asignada la clave %s",sentencia->clave);log_warning(logger,"prueba matando instancia2");}
		else{
			if (list_size(instancias_con_clave )== 1){
				log_warning(logger,"prueba matando instancia3");
				//existe
				t_clave * instanciaDuena = list_get(instancias_con_clave ,0);
				proxima =  instanciaDuena->instancia;
				log_warning(logger,"prueba matando instancia3.1 instDuena clave %s - instancia null: %d ", instanciaDuena->clave, instanciaDuena->instancia == NULL);
			}
			else{
				log_warning(logger,"prueba matando instancia4");
				proxima = siguienteInstanciaSegunAlgoritmo(sentencia->clave, ASIGNAR);
				t_clave * instanciaDuena = list_get(instancias_con_clave ,0);
				instanciaDuena->instancia = proxima;

			}
		}
	//**
	log_warning(logger,"prueba matando instancia5");
	log_warning(logger,"prueba matando instancia6 prox nomb %s", proxima->nombre);
	log_warning(logger,"prueba matando instancia6 sent  clave %s", sentencia->clave);
	log_warning(logger,"prueba matando instancia6 sent valor %s", sentencia->valor);
	log_info(logger,"Enviando a instancia: %s %s %s",proxima->nombre,sentencia->clave, sentencia->valor);

//TODO Que siguienteInstanciaSegunAlgortimo devuelva null en vez de esto
	if (string_equals_ignore_case(proxima->nombre, "ERROR")){
		free(proxima->nombre);
		free(proxima);
		rta.instancia->nombre = strdup(proxima->nombre);
		rta.cod = ABORTAR;
		return rta;
	}

	rta.instancia->id = proxima->id;
	rta.instancia->nombre = strdup(proxima->nombre);
	rta.instancia->socket = proxima->socket;
	t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_SENTENCIA, sizeof(t_content_header));
	t_esi_operacion_sin_puntero * s_sin_p = armar_esi_operacion_sin_puntero(sentencia);

	int instancia_conectada = chequearConectividadProceso(proxima);
	log_error(logger,"instancia conectada: %d", instancia_conectada == CONECTADO);
	if (instancia_conectada==DESCONECTADO){
		rta.cod = ERROR_I;
		rta.instancia = NULL;
		rta.valor = NULL;
		return rta;
		}
	else{
		int header_envio = send(proxima->socket,header,sizeof(t_content_header),0);
		int sentencia_envio = send(proxima->socket, s_sin_p, sizeof(t_esi_operacion_sin_puntero),0);
		if (sentencia->keyword == SET_){
			int valor_envio = send(proxima->socket,sentencia->valor,strlen(sentencia->valor),0);
		}

		log_info(logger,"Enviada sentencia a instancia");
		log_info(logger, "Esperando rta de Instancia");
	//Espero rta de Instancia

	//	sem_wait(&semInstancias);
		int header_rta_instancia = recv(proxima->socket,header,sizeof(t_content_header), 0);
	//	sem_post(&semInstancias);

		log_info(logger, "Rta Instancia Header: - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",header->proceso_origen,header->proceso_receptor,header->operacion,header->cantidad_a_leer);

		//int * cod_rta = malloc(sizeof(int));
		t_respuesta_instancia * rta_instancia = malloc(sizeof(t_respuesta_instancia));

	//	sem_wait(&semInstancias);
		//header_rta_instancia = recv(proxima->socket,cod_rta,sizeof(int), 0);
		header_rta_instancia = recv(proxima->socket,rta_instancia,sizeof(t_respuesta_instancia), 0);

		actualizarEntradasLibres(proxima->nombre,rta_instancia->entradas_libres);//TODO debería ser lo mismo actualizar el campo de "proxima"

		//sem_post(&semInstancias);
		rta.cod= rta_instancia->rdo_operacion;
		log_info(logger, "Rta Instancia %srespuesta: - %d - %d - entradas libres: %d",proxima->nombre,rta.cod,rta_instancia->rdo_operacion,rta_instancia->entradas_libres);
		//Si estoy pidiendo el valor de la clave, recibo la clave:
		//GET - Sé que no tiene que ir a la instancia. Voy a usar este código cuando necesite sabes el valor de la clave
		if (sentencia->keyword == OBTENER_VALOR){
			char * valor = malloc(header->cantidad_a_leer);
			header_rta_instancia = recv(proxima->socket,valor,header->cantidad_a_leer, 0);
			rta.valor = strdup(valor);
		}

		log_info(logger,"Recibido valor de instancia %s: Clave %s - Valor %s",rta.instancia->nombre,sentencia->clave,sentencia->valor);

		//free(proxima->nombre);
		//free(proxima);
		//NO libero la instancia "proxima" porque apunnta a la lista de instancias. si la libero al estoy borrando de la lista.
		free(header);
		free(s_sin_p);

		return rta;
	}
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	//log_warning(logger,"aca - %d",hd->operacion);
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			char * nombre = malloc(hd->cantidad_a_leer);
			int status_recv = recv(socketInstancia, nombre, hd->cantidad_a_leer, 0);
			//log_info(logger,"Tamaño nombre: %d - Nombre: %s",strlen(nombre),nombre);
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia, nombre);
			t_instancia * inst_guardada = getInstanciaByName(nombre);
			if (inst_guardada->flag_thread != 1){
				loopInstancia(inst_guardada, nombre);
			}

			free(nombre);
			break;

		default:
			break;
	}
}

void interpretarOperacionPlanificador(t_content_header * hd, int socketCliente){
	//log_info(logger,"Interpetando operación planificador");
	switch(hd->operacion){
	case PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION:
		PROCESO_PLANIFICADOR.id = nuevoIDInstancia();
		PROCESO_PLANIFICADOR.socket = socketCliente;
	}
}

t_clave * guardarClaveInternamente(char clave[40], int keyword){

	int tiene_clave(t_clave * clObj){
				return (strcmp(clave, clObj->clave)==0);
			}
	//log_info(logger,"guardando clave - sin instancia");
	t_clave * instancias_con_clave = list_filter(lista_claves,(void*)tiene_clave);
	if (list_size(instancias_con_clave )>1){log_error(logger,"Más de una instancia tiene asignada la clave %s",clave);}
	else{
		if (list_size(instancias_con_clave )== 1){
			//existe
			//log_warning(logger,"existía la clavee");
			t_clave * instanciaDuena = list_get(instancias_con_clave ,0);
			if (keyword != GET && instanciaDuena->instancia == NULL){
				//SET INSTANCIA
				t_clave * getClaveByName(char * nombre){
					int mismoNombre(t_clave * clave){return string_equals_ignore_case(clave->clave, nombre);}
					return list_find(lista_claves,*mismoNombre);
				}
				instanciaDuena = getClaveByName(clave);
				instanciaDuena->instancia = siguienteInstanciaSegunAlgoritmo(clave,ASIGNAR);
			}
			return instanciaDuena;
		}
		else{
			//no existe
			log_warning(logger,"no existía la clave");
			t_clave * claveObjeto = malloc(sizeof(t_clave));
			//asigno la instancia la primera vez que envio a una
			log_warning(logger,"keyword: %d", keyword);
			log_warning(logger,"GET: %d", GET);
			log_warning(logger,"OBTENER_VALOR: %d", OBTENER_VALOR);
			if (keyword != GET){
				log_warning(logger,"set o store - asigno instancia");
				claveObjeto->instancia = siguienteInstanciaSegunAlgoritmo(clave,  ASIGNAR);
			}else claveObjeto->instancia = NULL;
			strncpy(claveObjeto->clave,clave,40);

			list_add(lista_claves,claveObjeto);

			return claveObjeto;
		}
	}

	//Chequear que no exista internamente
	//Si existe, devolver la instancia que la tiene.
	//Si no existe,

}

int chequearConectividadProceso(t_instancia * instancia){
	t_content_header * st_connect = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_CHEQUEO_CONEXION,0);
	int status_connect = send(instancia->socket,st_connect,sizeof(t_content_header),0);
	int status_recv = recv(instancia->socket,st_connect,sizeof(t_content_header),0);
	free(st_connect);
	log_error(logger, "cheqconectproce status_recv %d", status_recv);
	if (status_recv ==-1 || status_recv == 0){instancia->flag_thread = 0; return DESCONECTADO;}
	else return CONECTADO;
}

void loopPlanificadorConsulta(){
	while(1){
		log_warning(logger,"loopPlanificadorCOnsulta");
		pthread_mutex_lock(&consulta_planificador);
		log_warning(logger,"entro en mutex");
		rdo_consulta_planificador = consultarPlanificador(sentencia_global);
		log_error(logger,"Resultado del planificador en loop %d",rdo_consulta_planificador );
		pthread_mutex_unlock(&consulta_planificador_terminar);
	}
}

int consultarPlanificador(t_sentencia * sentencia){
	//TODO
	log_info(logger,"Consultando planificador..");

	t_consulta_bloqueo * consulta_a_planif = malloc(sizeof(t_consulta_bloqueo));

	strncpy(consulta_a_planif->clave,sentencia->clave,40);
	consulta_a_planif->pid = sentencia->pid;
	consulta_a_planif->sentencia = sentencia->keyword;
log_info(logger,"Le mandé: clave %s - pid %d - key %d", consulta_a_planif->clave, consulta_a_planif->pid, consulta_a_planif->sentencia);
	t_content_header * header = crear_cabecera_mensaje(coordinador, planificador, COORD_PLANIFICADOR_OPERACION_CONSULTA_CLAVE_COORD,0);
	int status_head = send(PROCESO_PLANIFICADOR.socket, header, sizeof(t_content_header), 0);

	int status_pack = send(PROCESO_PLANIFICADOR.socket, consulta_a_planif, sizeof(t_consulta_bloqueo), 0);
	//recibo rta
	int status_recv = recv(PROCESO_PLANIFICADOR.socket, header, sizeof(t_content_header),0);
	int * rta = malloc(sizeof(int));
	status_recv = recv(PROCESO_PLANIFICADOR.socket,rta, sizeof(int),0);
	log_error(logger,"Respuesta de Planificador en consultar: %d", *rta);
	return *rta;
}

int puedoEjecutarSentencia(t_sentencia * sentencia){
	//	Si existe, verificar conexión de instancia.
	//	Si no está conectada abortar esi.
	// 	Preguntarle a Planificador si la clave no esta bloqueada.
	log_info(logger,"Chequeando si puedo ejecutar la sentencia...");
	if(	list_size(lista_instancias)==0){return ABORTAR;}

	//TODO: que guardarclave... setee como null la instancia si la sentencia es get
	t_clave * clave_obj = guardarClaveInternamente(sentencia->clave,sentencia->keyword);

	if (sentencia->keyword == SET_){
		log_info(logger,"aca no rompe");
		if (clave_obj->instancia != NULL){
			if (chequearConectividadProceso(clave_obj->instancia)==DESCONECTADO){
				log_error(logger, "chequCOnectProc 0");
				return ABORTAR;
			}
			else return CORRECTO;
		}
		else return CORRECTO;
	}

	pthread_mutex_lock(&lock_sentencia_global);
	sentencia_global = sentencia;

	log_warning(logger,"unlock cons planif");
	pthread_mutex_unlock(&consulta_planificador);

	log_warning(logger,"lock cons planif term");
	pthread_mutex_lock(&consulta_planificador_terminar);

	log_warning(logger,"lock cons planif");
	//pthread_mutex_lock(&consulta_planificador);
	pthread_mutex_unlock(&lock_sentencia_global);

	log_info(logger,"consulta planificador: %d", rdo_consulta_planificador);
	//return CORRECTO;//consultarPlanificador(sentencia);
	return rdo_consulta_planificador;
}

devolverResultadoAESI(int socketEsi, rta_envio rta, int idEsi, int proceso){
	devolverCodigoResultadoAESI(socketEsi, rta.cod, idEsi,proceso);
}

void devolverCodigoResultadoAESI(int socketCliente, int cod, int idEsi, int proceso){

	t_content_header * cabecera_rdo = crear_cabecera_mensaje(coordinador,esi, RESULTADO_EJECUCION_SENTENCIA,sizeof(t_content_header));
	int status_hd_error = send(socketCliente,cabecera_rdo,sizeof(t_content_header),0);

	char * pr;
	char * pr_c;
	if (proceso == instancia){pr = strdup("instancia");} else pr = strdup("esi");
	log_info(logger, "Rdo que llega a la funcion'%d', proceso: %d", cod, proceso);

	respuesta_coordinador * cod_error = malloc(sizeof(respuesta_coordinador));
	if (cod ==ERROR_I && proceso == instancia){
		cod_error->resultado_del_parseado = ABORTAR;
		pr_c = strdup("abortar");
	}

	if (cod ==CLAVE_BLOQUEADA && proceso == esi){
		cod_error->resultado_del_parseado = CLAVE_BLOQUEADA;
		pr_c = strdup("clave bloqueada");
	}

	if (((cod == EXITO_I) && (proceso == instancia)) || ((cod == CORRECTO) && (proceso == esi))){
		cod_error->resultado_del_parseado = CORRECTO;
		pr_c = strdup("correcto");
	}

	if (cod == ABORTAR && proceso == esi){
		cod_error->resultado_del_parseado = ABORTAR;
		pr_c = strdup("abortar");
	}

	log_warning(logger, "proceso %s - cod %s", pr,pr_c);

	log_info(logger, "Devolviendo rdo '%d' a ESI '%d'..", cod_error->resultado_del_parseado, idEsi );
	//log_info(logger, "El codigo es CORRECTO? %d", CORRECTO == cod_error->resultado_del_parseado);
	int status_hd_mensaje_error = send(socketCliente, cod_error, sizeof(respuesta_coordinador), 0);

	log_info(logger, "Rdo devuelto a %d",idEsi);
}

void indicarCompactacionATodasLasInstancias(){
	//Uso varios para que un hilo instancia no entre dos veces y otro se quede sin compactar
	log_warning(logger,"Indicando a las instancias que compacten");
	for (int i = 0; list_size(lista_instancias) >i; i++){
		sem_post(&semInstancias);
	}
	for (int i = 0; list_size(lista_instancias) >i; i++){
		sem_wait(&semInstanciasFin);
	}
	/*
	for (int i = 0; list_size(lista_instancias) >i; i++){
		sem_wait(&semInstanciasTodasFin);
	}*/

}

void proseguirOperacionNormal(int socketCliente, t_sentencia * sentencia_con_punteros){
	//printf("adsf");
	switch(sentencia_con_punteros->keyword){
	case OBTENER_VALOR://get
		//guardarClaveInternamente(sentencia_con_punteros->clave); ya guardé cuando chequeé si podia ejecutar
		devolverCodigoResultadoAESI(socketCliente, CORRECTO, sentencia_con_punteros->pid , esi);
		//Ya pregunté anteriormente al planificador, y ya la bloqueó
		break;
	default:
		;
		log_info(logger,"Enviando sentencia a instancia");
		//Tanto para set o store le pregunté al planificador si podía continuar. El planificador ya hizo chequeos necesarios/operaciones necesarias.
		rta_envio rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);
		//Reintenta hasta 3 veces si debe compactar.
		int contador = 3;
		while (rdo_ejecucion_instancia.cod == COMPACTAR && contador>0){
			indicarCompactacionATodasLasInstancias();
			log_info(logger, "Reenviando última sentencia a Instancia %s...", rdo_ejecucion_instancia.instancia->nombre);
			rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);
			contador--;
		}
		log_info(logger,"Obtenido resultado");
		//TODO Que enviarSentenciaInstancia 
		//TODO Verificar si después de las 3 veces sigue devolviendo COMPACTAR. Si es asi ver que hacer. Abortar esi y mostrar log_error por consola?
		devolverResultadoAESI(socketCliente, rdo_ejecucion_instancia, sentencia_con_punteros->pid,instancia);
		break;
	}
}

void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	char * buffer = NULL;

	log_info(logger, "Interpretando operación ESI - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);
//Retardo:
	usleep(RETARDO*1000);
	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
	{
		//Recibo de ESI sentencia parseada
		//log_info(logger,"esi sentencia recibo");
		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		int cantleida = recv( socketCliente, sentencia, hd->cantidad_a_leer, 0);
		log_info(logger,"esi sentencia recibida");
		log_info(logger,"KEYWORD: %d", sentencia->keyword);
		log_info(logger,"tam valor: %d", sentencia->tam_valor);

		buffer = calloc(sentencia->tam_valor,sizeof(char));
		if (sentencia->keyword == SET_){

			char valRec[sentencia->tam_valor];

			//Recibo el valor - El esi me lo manda "pelado", directamente el string, ningún struct
			//memset(buffer,0,sentencia->tam_valor);
			printf("TAMANIO VALOR %d\n", sentencia->tam_valor);
			int valor_status = recv(socketCliente, buffer, sentencia->tam_valor,0);
			//buffer[sentencia->tam_valor] = '\0';

			//set char array
			strncpy(valRec,buffer,sentencia->tam_valor);
			valRec[sentencia->tam_valor] = '\0';

			//set char p
			free(buffer);
			buffer = strdup(valRec);

			printf("buffer: %s\n",buffer);
			printf("valrec: %s\n",valRec);

			//buffer[sentencia->tam_valor] = '\0';
			//log_info(logger,"esi valor recibido: %s", buffer);
		}
		else
		{
			free(buffer);
			buffer = strdup("");
		}
		//Armo una variable interna para manejar la sentencia
		t_sentencia * sentencia_con_punteros = armar_sentencia(sentencia, buffer);
		free(buffer);
		log_operacion_esi(sentencia_con_punteros, logger_operaciones);


		int puedoEnviar = puedoEjecutarSentencia(sentencia_con_punteros);
		log_info(logger, "PuedoEnviar %d", puedoEnviar);
		switch(puedoEnviar){
			case CORRECTO:

				proseguirOperacionNormal(socketCliente,sentencia_con_punteros);
				break;
			case CLAVE_BLOQUEADA:
				//log_info(logger,"Devolviendo error al ESI%d", sentencia->pid);
				devolverCodigoResultadoAESI(socketCliente, CLAVE_BLOQUEADA, sentencia_con_punteros->pid,esi);
				log_info(logger,"Devuelto CLAVE_BLOQUEADA");
				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
				break;
			case ABORTAR:
				//log_info(logger,"Devolviendo error al ESI %d", sentencia->pid);
				devolverCodigoResultadoAESI(socketCliente, ABORTAR, sentencia_con_punteros->pid,esi);
				log_warning(logger,"Devuelto ABORTAR al ESI %d", sentencia->pid);
				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
				//log_info(logger,"Fin abortar");
				break;
			default:
				break;
		}
		free(sentencia);
		break;
	}
	default:
		//TODO no se reconoció el tipo operación
		break;
	}
	//log_info(logger,"Fin interpretación");
}

void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case esi:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case instancia:
//		pthread_mutex_lock(&bloqueo_de_Instancias);
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case planificador:
		//if (leer_planificador_request){
			interpretarOperacionPlanificador(hd,socketCliente);
			loopPlanificadorConsulta();
		//}
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    log_info(logger, "Cliente conectado. Esperando mensajes:");
    total_hilos++;
    log_info(logger, "total hilos: %d",total_hilos);

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header > 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), 0);
    	log_info(logger, "Recv - Status Header: %d", status_header);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado"); total_hilos--;
    	}
    	else {
    		log_info(logger, "Interpretando header...");
    		interpretarHeader(header, socketCliente);
    	};

    	//if (header->proceso_origen == instancia /*|| header->proceso_origen == planificador*/){
    		//status_header =-1;
    		//log_warning(logger,	"Cerrando hilo para proceso no ESI");
    	//}
   	}

    log_warning(logger,"cerrada ");
	//if (header->proceso_origen == esi){
	    close(socketCliente);
	//}

    free(header);

}

int main(int argc, char **argv){

	seteosIniciales(argv[1]);

	pthread_mutex_init(&consulta_planificador, NULL);
	pthread_mutex_init(&lock_sentencia_global, NULL);
	pthread_mutex_init(&consulta_planificador_terminar,NULL);

	pthread_mutex_lock(&consulta_planificador);
	pthread_mutex_lock(&consulta_planificador_terminar);
	//pthread_mutex_init(&mutexInstancias, NULL);
	sem_init(&semInstancias, 0, 0);
	sem_init(&semInstanciasFin, 0, 0);
	sem_init(&semInstanciasTodasFin	, 0, 0);
	//pthread_mutex_init(&bloqueo_de_Instancias, NULL);
//	pthread_mutex_lock(&bloqueo_de_Instancias);
//	pthread_mutex_unlock(&bloqueo_de_Instancias;
	//sem_wait(&semInstancias);
//	sem_post(&semInstancias);

	armar_hilo_planificador_status();

	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(logger,"Socket de escucha creado %d", listenningSocket);

    // Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    log_info(logger, "Socket de escucha bindeado");
    freeaddrinfo(serverInfo);

    log_info(logger, "Escuchando...");
    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

	while (1){
    	log_info(logger, "Esperando conexiones...");
    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
		log_info(logger, "Conexión recibida - Accept: %d ",socketCliente);

		crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);
	}

    close(listenningSocket);
    log_destroy(logger);
    return 0;


}
