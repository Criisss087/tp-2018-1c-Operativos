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

void logger_coordinador(int tipo_esc, int tipo_log, const char* mensaje, ...){

	//Colores (reset,vacio,vacio,cian,verde,vacio,amarillo,rojo)
	static char *log_colors[8] = {"\x1b[0m","","","\x1b[36m", "\x1b[32m", "", "\x1b[33m", "\x1b[31m" };
	char *console_buffer=NULL;

	char *msj_salida = malloc(sizeof(char) * 256);

	//Captura los argumentos en una lista
	va_list args;
	va_start(args, mensaje);

	//Arma el mensaje formateado con sus argumentos en msj_salida.
	vsprintf(msj_salida, mensaje, args);

	//ESCRIBE POR PANTALLA
	if((tipo_esc == escribir) || (tipo_esc == escribir_loguear)){
		//printf("%s",msj_salida);
		//printf("\n");

		console_buffer = string_from_format("%s%s%s",log_colors[tipo_log],msj_salida, log_colors[0]);
		printf("%s",console_buffer);
		printf("\n");
		fflush(stdout);
		free(console_buffer);
	}

	//LOGUEA
	if((tipo_esc == loguear) || (tipo_esc == escribir_loguear)){

		if(tipo_log == l_info){
			log_info(logger, msj_salida);
		}
		else if(tipo_log == l_warning){
			log_warning(logger, msj_salida);
		}
		else if(tipo_log == l_error){
			log_error(logger, msj_salida);
		}
		else if(tipo_log == l_debug){
			log_debug(logger, msj_salida);
		}
		else if(tipo_log == l_trace){
			log_trace(logger, msj_salida);
		}
		else if(tipo_log == l_esi){
			log_info(logger_operaciones, msj_salida);
		}
	}

	va_end(args);
	free(msj_salida);

	return;
}


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
	int tiene_clave(t_clave * clObj){
		return (strcmp(sentencia->clave, clObj->clave)==0);
	}

	t_list * instancias_con_clave = list_filter(lista_claves,(void*)tiene_clave);
	if (list_size(instancias_con_clave) > 1){
		logger_coordinador(escribir_loguear,l_error,"Más de una instancia tiene asignada la clave %s\n",sentencia->clave);
	}
	else{
		if (list_size(instancias_con_clave ) == 1){
			//Existe
			t_clave * instanciaDuena = list_get(instancias_con_clave ,0);
			proxima =  instanciaDuena->instancia;
			logger_coordinador(loguear,l_warning,"instDuena clave %s - instancia null: %d \n", instanciaDuena->clave, instanciaDuena->instancia == NULL);
		}
		else{
			proxima = siguienteInstanciaSegunAlgoritmo(sentencia->clave, ASIGNAR);
			t_clave * instanciaDuena = list_get(instancias_con_clave ,0);
			instanciaDuena->instancia = proxima;

		}
	}

	logger_coordinador(escribir_loguear, l_info, "Enviando a instancia: %s %s %s\n",proxima->nombre,sentencia->clave, sentencia->valor);

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

	logger_coordinador(loguear, l_warning,"Codigos enviados en header a instancia: %d %d %d %d \n",coordinador,instancia,COORDINADOR_INSTANCIA_SENTENCIA, sizeof(t_content_header));

	t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_SENTENCIA, sizeof(t_content_header));
	t_esi_operacion_sin_puntero * s_sin_p = armar_esi_operacion_sin_puntero(sentencia);

	int instancia_conectada = chequearConectividadProceso(proxima);

	logger_coordinador(escribir_loguear, l_info,"Instancia conectada: %d \n", instancia_conectada == CONECTADO);

	if (instancia_conectada==DESCONECTADO){
		rta.cod = ERROR_I;
		rta.instancia = NULL;
		rta.valor = NULL;
		return rta;
		}
	else{
		int header_envio = send(proxima->socket,header,sizeof(t_content_header),0);
		if(header_envio < 0){
			logger_coordinador(escribir_loguear, l_error,"Error en el send del header de enviar sentencia a instancia\n");
		}

		int sentencia_envio = send(proxima->socket, s_sin_p, sizeof(t_esi_operacion_sin_puntero),0);

		if(sentencia_envio < 0){
			logger_coordinador(escribir_loguear, l_error,"Error en el send de la sentencia sin punteros a instancia\n");
		}

		if (sentencia->keyword == SET_){
			int valor_envio = send(proxima->socket,sentencia->valor,strlen(sentencia->valor),0);

			if(valor_envio < 0){
				logger_coordinador(escribir_loguear, l_error,"Error en el send del valor del set en enviar sentencia a instancia\n");
			}
		}

	logger_coordinador(escribir_loguear, l_info,"\nEnviada sentencia a instancia\n");
	logger_coordinador(escribir_loguear, l_info,"Esperando respuesta de instancia...\n");

	//Espero rta de Instancia
	int header_rta_instancia = recv(proxima->socket,header,sizeof(t_content_header), 0);

	if(header_rta_instancia < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el recv del header de la rta de instancia \n");
	}

	logger_coordinador(loguear, l_info,"Rta Instancia Header: - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d\n",header->proceso_origen,header->proceso_receptor,header->operacion,header->cantidad_a_leer);

	t_respuesta_instancia * rta_instancia = malloc(sizeof(t_respuesta_instancia));
	header_rta_instancia = recv(proxima->socket,rta_instancia,sizeof(t_respuesta_instancia), 0);

	if(header_rta_instancia < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el recv de la respuesta de instancia \n");
	}

	actualizarEntradasLibres(proxima->nombre,rta_instancia->entradas_libres);//TODO debería ser lo mismo actualizar el campo de "proxima"

	rta.cod= rta_instancia->rdo_operacion;
	logger_coordinador(escribir_loguear, l_info, "Respuesta de instancia nombre %s, respuesta: - %d - %d - entradas libres: %d \n",proxima->nombre,rta.cod,rta_instancia->rdo_operacion,rta_instancia->entradas_libres);

	//Si estoy pidiendo el valor de la clave, recibo la clave:
	//GET - Sé que no tiene que ir a la instancia. Voy a usar este código cuando necesite sabes el valor de la clave
	if (sentencia->keyword == OBTENER_VALOR){
		char * valor = malloc(header->cantidad_a_leer);
		header_rta_instancia = recv(proxima->socket,valor,header->cantidad_a_leer, 0);

		if(header_rta_instancia < 0){
			logger_coordinador(escribir_loguear, l_error,"Error en el recv del valor de la clave de la respuesta de instancia \n");
		}

		rta.valor = strdup(valor);
		free(valor);
	}

	logger_coordinador(escribir_loguear,l_info,"Recibido valor de instancia %s: Clave %s - Valor %s \n",rta.instancia->nombre,sentencia->clave,sentencia->valor);

	//NO libero la instancia "proxima" porque apunnta a la lista de instancias. si la libero al estoy borrando de la lista.
	destruir_cabecera_mensaje(header);
	free(s_sin_p);

	return rta;
	}
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){

	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			char * nombre = malloc(hd->cantidad_a_leer);
			int status_recv = recv(socketInstancia, nombre, hd->cantidad_a_leer, 0);

			if(status_recv < 0){
				logger_coordinador(escribir_loguear, l_error,"Error en el recv del nombre de la instancia \n");
			}

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

	switch(hd->operacion){
	case PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION:
		PROCESO_PLANIFICADOR.id = nuevoIDInstancia();
		PROCESO_PLANIFICADOR.socket = socketCliente;
	}
}

t_clave * guardarClaveInternamente(char clave[40], int keyword){

	//Chequear que no exista internamente
	//Si existe, devolver la instancia que la tiene.
	//Si no existe,

	int tiene_clave(t_clave * clObj){
		return (strcmp(clave, clObj->clave)==0);
	}

	t_list * instancias_con_clave = list_filter(lista_claves,(void*)tiene_clave);

	if(list_size(instancias_con_clave ) > 1){
		logger_coordinador(escribir_loguear, l_error,"Más de una instancia tiene asignada la clave %s\n",clave);
	}else{
		if(list_size(instancias_con_clave ) == 1){
			//Existe
			t_clave * instanciaDuena = list_get(instancias_con_clave ,0);

			if (keyword != GET && instanciaDuena->instancia == NULL){
				//SET INSTANCIA
				t_clave * getClaveByName(char * nombre){
					int mismoNombre(t_clave * clave){return string_equals_ignore_case(clave->clave, nombre);}
					return list_find(lista_claves,(void*)mismoNombre);
				}

				instanciaDuena = getClaveByName(clave);
				instanciaDuena->instancia = siguienteInstanciaSegunAlgoritmo(clave,ASIGNAR);
			}
			return instanciaDuena;
		}
		else{
			//No existe
			logger_coordinador(escribir_loguear, l_warning,"No existía la clave\n");

			t_clave * claveObjeto = malloc(sizeof(t_clave));

			//Asigno la instancia la primera vez que envio a una

			logger_coordinador(loguear, l_warning,"\n KEYWORD %d\n", keyword);
			logger_coordinador(loguear, l_warning,"\n GET %d\n", GET);
			logger_coordinador(loguear, l_warning,"\n OBTENER_VALOR %d\n", OBTENER_VALOR);

			if (keyword != GET){
				logger_coordinador(escribir_loguear, l_warning,"Set o Store, asigno instancia...\n");
				claveObjeto->instancia = siguienteInstanciaSegunAlgoritmo(clave,  ASIGNAR);
			}else{
				claveObjeto->instancia = NULL;
			}

			strncpy(claveObjeto->clave,clave,40);

			list_add(lista_claves,claveObjeto);

			return claveObjeto;
		}
	}
}

int consultarPlanificador(t_sentencia * sentencia){

	logger_coordinador(escribir_loguear,l_info,"\nConsultando planificador...\n");

	t_consulta_bloqueo * consulta_a_planif = malloc(sizeof(t_consulta_bloqueo));

	strncpy(consulta_a_planif->clave,sentencia->clave,40);
	consulta_a_planif->pid = sentencia->pid;
	consulta_a_planif->sentencia = sentencia->keyword;

	logger_coordinador(escribir_loguear,l_info,"Le mandé: clave %s - pid %d - key %d\n",consulta_a_planif->clave, consulta_a_planif->pid, consulta_a_planif->sentencia);

	t_content_header * header = crear_cabecera_mensaje(coordinador, planificador, COORD_PLANIFICADOR_OPERACION_CONSULTA_CLAVE_COORD,0);
	int status_head = send(PROCESO_PLANIFICADOR.socket, header, sizeof(t_content_header), 0);

	if(status_head < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el send del header para consultar al planificador\n");
	}

	int status_pack = send(PROCESO_PLANIFICADOR.socket, consulta_a_planif, sizeof(t_consulta_bloqueo), 0);

	if(status_pack < 0){
		logger_coordinador(escribir_loguear, l_error,"Error al enviar send con el contenido de la consulta al planificador\n");
	}

	free(consulta_a_planif);

	//Recibo rta
	int status_recv = recv(PROCESO_PLANIFICADOR.socket, header, sizeof(t_content_header),0);

	if(status_recv < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el recv del header de la rta del planificador al consultar\n");
	}

	destruir_cabecera_mensaje(header);

	int * rta = malloc(sizeof(int));
	status_recv = recv(PROCESO_PLANIFICADOR.socket,rta, sizeof(int),0);

	if(status_recv < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el recv de la respuesta del planificador al consultar\n");
	}

	logger_coordinador(escribir_loguear, l_info,"Respuesta de Planificador en consultar: %d\n",*rta);

	return *rta;
}

void loopPlanificadorConsulta(){
	while(GLOBAL_SEGUIR){
		logger_coordinador(loguear, l_warning,"loopPlanificadorCOnsulta\n");

		pthread_mutex_lock(&consulta_planificador);

		logger_coordinador(loguear, l_warning,"entro en mutex\n");

		rdo_consulta_planificador = consultarPlanificador(sentencia_global);

		logger_coordinador(escribir_loguear,l_info,"Resultado del planificador en loop %d \n",rdo_consulta_planificador);

		pthread_mutex_unlock(&consulta_planificador_terminar);
	}
}

int puedoEjecutarSentencia(t_sentencia * sentencia){
	//	Si existe, verificar conexión de instancia.
	//	Si no está conectada abortar esi.
	// 	Preguntarle a Planificador si la clave no esta bloqueada.

	logger_coordinador(escribir_loguear, l_info,"\nChequeando si puedo ejecutar la sentencia...\n");

	if(list_size(lista_instancias) == 0){
		return ABORTAR;
	}
	t_clave * clave_obj = guardarClaveInternamente(sentencia->clave,sentencia->keyword);

	if (sentencia->keyword == SET_){
		if (clave_obj->instancia != NULL){
			if(chequearConectividadProceso(clave_obj->instancia) == DESCONECTADO){
				logger_coordinador(escribir_loguear,l_error, "Chequo conectividad proceso: desconectado\n");
				return ABORTAR;
			}
			else return CORRECTO;
		}
		else return CORRECTO;
	}

	pthread_mutex_lock(&lock_sentencia_global);

	sentencia_global = sentencia;

	logger_coordinador(loguear, l_warning,"unlock cons planif\n");

	pthread_mutex_unlock(&consulta_planificador);

	logger_coordinador(loguear, l_warning,"unlock cons planif term\n");

	pthread_mutex_lock(&consulta_planificador_terminar);

	logger_coordinador(loguear, l_warning,"lock cons planif\n");

	pthread_mutex_unlock(&lock_sentencia_global);

	logger_coordinador(escribir_loguear, l_info,"Consulta planificador\n", rdo_consulta_planificador);

	return rdo_consulta_planificador;
}

void devolverCodigoResultadoAESI(int socketCliente, int cod, int idEsi, int proceso){

	t_content_header * cabecera_rdo = crear_cabecera_mensaje(coordinador,esi, RESULTADO_EJECUCION_SENTENCIA,sizeof(t_content_header));
	int status_hd_error = send(socketCliente,cabecera_rdo,sizeof(t_content_header),0);

	if(status_hd_error < 0){
		logger_coordinador(escribir_loguear, l_error,"Error en el send del header de devolver codigo resultado a esi\n");
	}

	char * pr;
	char * pr_c;
	int bandera = 0;

	if(proceso == instancia){
		pr = strdup("instancia");
	} else pr = strdup("esi");

	logger_coordinador(escribir_loguear,l_info, "Rdo que llega a la funcion '%d', proceso: %d \n", cod, proceso);

	respuesta_coordinador * cod_error = malloc(sizeof(respuesta_coordinador));
	if (cod ==ERROR_I && proceso == instancia){
		cod_error->resultado_del_parseado = ABORTAR;
		pr_c = strdup("abortar");
		bandera = 1;
	}

	if (cod ==CLAVE_BLOQUEADA && proceso == esi){
		cod_error->resultado_del_parseado = CLAVE_BLOQUEADA;
		pr_c = strdup("clave bloqueada");
		bandera = 1;
	}

	if (((cod == EXITO_I) && (proceso == instancia)) || ((cod == CORRECTO) && (proceso == esi))){
		cod_error->resultado_del_parseado = CORRECTO;
		pr_c = strdup("correcto");
		bandera = 1;
	}

	if (cod == ABORTAR && proceso == esi){
		cod_error->resultado_del_parseado = ABORTAR;
		pr_c = strdup("abortar");
		bandera = 1;
	}

	if (bandera == 0){
		logger_coordinador(escribir_loguear,l_error, "BASURA DEVUELTA POR LA INSTANCIA, ENVIANDO CODIGO DE ABORTO %s %s", pr,pr_c);
		cod_error->resultado_del_parseado = ABORTAR;
		pr_c = strdup("abortar");
	}

	logger_coordinador(loguear,l_warning, "Proceso %s - cod %s\n", pr,pr_c);
	logger_coordinador(escribir_loguear,l_info, "Devolviendo resultado '%d' a ESI '%d'..\n", cod_error->resultado_del_parseado, idEsi);

	int status_hd_mensaje_error = send(socketCliente, cod_error, sizeof(respuesta_coordinador), 0);

	if(status_hd_mensaje_error < 0){
		logger_coordinador(escribir_loguear, l_error,"Error al enviar el codigo con el resultado del parseado al ESI\n");
	}

	logger_coordinador(escribir_loguear,l_info, "Resultado devuelto a %d\n", idEsi);

	free(cod_error);
	free(pr);
	free(pr_c);
	destruir_cabecera_mensaje(cabecera_rdo);
}

void devolverResultadoAESI(int socketEsi, rta_envio rta, int idEsi, int proceso){
	devolverCodigoResultadoAESI(socketEsi, rta.cod, idEsi,proceso);
}


void indicarCompactacionATodasLasInstancias(){
	//Uso varios para que un hilo instancia no entre dos veces y otro se quede sin compactar
	logger_coordinador(escribir_loguear,l_warning, "Indicando a las instancias que compacten.\n");

	bool tieneHiloActivo(t_instancia * i){
		return (i->flag_thread == 1);
	};

	int total = list_size(list_filter(lista_instancias, (void*)tieneHiloActivo));

	for (int i = 0; total >i; i++){
		sem_post(&semInstancias);
	}
	for (int i = 0; total >i; i++){
		sem_wait(&semInstanciasFin);
	}

}

void proseguirOperacionNormal(int socketCliente, t_sentencia * sentencia_con_punteros){

	switch(sentencia_con_punteros->keyword){
	case OBTENER_VALOR://get
		//guardarClaveInternamente(sentencia_con_punteros->clave); ya guardé cuando chequeé si podia ejecutar
		devolverCodigoResultadoAESI(socketCliente, CORRECTO, sentencia_con_punteros->pid , esi);
		//Ya pregunté anteriormente al planificador, y ya la bloqueó
		break;
	default:
		;

		logger_coordinador(escribir_loguear,l_info, "\nEnviando sentencia a instancia\n");

		//Tanto para set o store le pregunté al planificador si podía continuar. El planificador ya hizo chequeos necesarios/operaciones necesarias.
		rta_envio rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);

		//Reintenta hasta 3 veces si debe compactar.
		int contador = 3;

		while (rdo_ejecucion_instancia.cod == COMPACTAR && contador>0){
			indicarCompactacionATodasLasInstancias();
			logger_coordinador(escribir_loguear,l_info, "Reenviando última sentencia a Instancia %s...\n", rdo_ejecucion_instancia.instancia->nombre);
			rdo_ejecucion_instancia = enviarSentenciaInstancia(sentencia_con_punteros);
			contador--;
		}

		logger_coordinador(escribir_loguear,l_info, "Obtenido resultado\n");

		devolverResultadoAESI(socketCliente, rdo_ejecucion_instancia, sentencia_con_punteros->pid,instancia);
		break;
	}
}

void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	char * buffer = NULL;

	logger_coordinador(loguear,l_info, "Interpretando operación ESI - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d \n",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	//Retardo:
	usleep(RETARDO*1000);

	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
	{
		//Recibo de ESI sentencia parseada
		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		int cantleida = recv( socketCliente, sentencia, hd->cantidad_a_leer, 0);

		if(cantleida < 0){
			logger_coordinador(escribir_loguear, l_error,"Error en el recv de la cantidad a leer de la sentencia\n");
		}

		logger_coordinador(loguear, l_warning,"Esi sentencia recibida.\n");
		logger_coordinador(loguear, l_warning,"KEYWORD %d\n",sentencia->keyword);
		logger_coordinador(loguear, l_warning,"tam valor: %d\n", sentencia->tam_valor);

		buffer = calloc(sentencia->tam_valor,sizeof(char));

		if (sentencia->keyword == SET_){

			char valRec[sentencia->tam_valor];

			//Recibo el valor - El esi me lo manda "pelado", directamente el string, ningún struct
			int valor_status = recv(socketCliente, buffer, sentencia->tam_valor,0);

			//set char array
			strncpy(valRec,buffer,sentencia->tam_valor);
			valRec[sentencia->tam_valor] = '\0';

			//set char p
			free(buffer);
			buffer = strdup(valRec);

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

		logger_coordinador(escribir_loguear,l_info, "Puedo enviar? : %d \n",puedoEnviar);

		switch(puedoEnviar){
			case CORRECTO:
				proseguirOperacionNormal(socketCliente,sentencia_con_punteros);
				break;
			case CLAVE_BLOQUEADA:
				devolverCodigoResultadoAESI(socketCliente, CLAVE_BLOQUEADA, sentencia_con_punteros->pid,esi);

				logger_coordinador(escribir_loguear,l_info, "Devuelto CLAVE_BLOQUEADA \n");

				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
				break;
			case ABORTAR:
				devolverCodigoResultadoAESI(socketCliente, ABORTAR, sentencia_con_punteros->pid,esi);

				logger_coordinador(escribir_loguear,l_warning, "Devuelto ABORTAR al ESI %d \n", sentencia->pid);

				log_error_operacion_esi(sentencia_con_punteros, puedoEnviar);
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
}

void interpretarHeader(t_content_header * hd, int socketCliente){

	logger_coordinador(loguear,l_info, "Interpretando header- Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d \n",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case esi:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case instancia:
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case planificador:
		interpretarOperacionPlanificador(hd,socketCliente);
		loopPlanificadorConsulta();
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    logger_coordinador(escribir_loguear,l_info, "Cliente conectado, esperando mensajes... \n");

    total_hilos++;

    logger_coordinador(loguear,l_info, "total hilos: %d \n",total_hilos);

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header > 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), 0);

        logger_coordinador(loguear,l_info, "Recv - Status Header: %d \n", status_header);

    	if (status_header == 0) {
    		logger_coordinador(escribir_loguear,l_info, "\nDesconectado \n");
    		total_hilos--;
    	}
    	else {
    		logger_coordinador(escribir_loguear,l_info, "Interpretando header... \n");
    		interpretarHeader(header, socketCliente);
    	};

   	}

    logger_coordinador(escribir_loguear, l_info, "\nCerrada \n");

    close(socketCliente);

	free(header);
}

int main(int argc, char **argv){

	seteosIniciales(argv[1]);

	pthread_mutex_init(&consulta_planificador, NULL);
	pthread_mutex_init(&lock_sentencia_global, NULL);
	pthread_mutex_init(&consulta_planificador_terminar,NULL);

	pthread_mutex_lock(&consulta_planificador);
	pthread_mutex_lock(&consulta_planificador_terminar);

	sem_init(&semInstancias, 0, 0);
	sem_init(&semInstanciasFin, 0, 0);
	sem_init(&semInstanciasTodasFin	, 0, 0);

	armar_hilo_planificador_status();

	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	logger_coordinador(loguear, l_info, "Socket de escucha creado %d \n", listenningSocket);

    // Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);

    logger_coordinador(loguear, l_info, "Socket de escucha bindeado \n");

    freeaddrinfo(serverInfo);

    logger_coordinador(escribir_loguear, l_info, "Escuchando... \n");

    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

	while (GLOBAL_SEGUIR){

		logger_coordinador(escribir_loguear, l_info, "\nEsperando conexiones...\n");

    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

    	logger_coordinador(escribir_loguear, l_info, "Conexión recibida - Accept: %d \n",socketCliente);

		crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);
	}

    close(listenningSocket);

    finalizar_coordinador();
    return 0;


}
