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


void interpretarOperacionPlanificador(t_content_header * hd, int socketCliente){
	switch(hd->operacion){
	case PLANIFICADOR_COORDINADOR_HEADER_IDENTIFICACION:
		PROCESO_PLANIFICADOR.id = nuevoIDInstancia();
		PROCESO_PLANIFICADOR.socket = socketCliente;
	}
}

void devolverCodigoResultadoAESI(int socketCliente, int cod, int idEsi){

	t_content_header * cabecera_rdo = crear_cabecera_mensaje(coordinador,esi, RESULTADO_EJECUCION_SENTENCIA,sizeof(t_content_header));
	int status_hd_error = send(socketCliente,cabecera_rdo,sizeof(t_content_header),NULL);

	respuesta_coordinador * cod_error = malloc(sizeof(respuesta_coordinador));
	if (cod ==ERROR_I){	cod_error->resultado_del_parseado = ABORTAR;}
	if (cod ==EXITO_I || cod == CORRECTO){	cod_error->resultado_del_parseado = CORRECTO;}
	if (cod == ABORTAR){	cod_error->resultado_del_parseado = ABORTAR;}

	int status_hd_mensaje_error = send(socketCliente, cod_error, sizeof(respuesta_coordinador), NULL);

}

void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando operación ESI - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);
//Retardo:
	usleep(RETARDO*1000);
	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
	{;
	t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
	int cantleida = recv( socketCliente, sentencia, hd->cantidad_a_leer, NULL);
	log_info(logger,"Sentencia recibida, KEYWORD: %d, tam valor: %d",sentencia->keyword,sentencia->tam_valor);

	char * valor;
	char val[sentencia->tam_valor];
	if (sentencia->keyword == SET_){
		valor = malloc(sentencia->tam_valor);
		memset(valor,0,sentencia->tam_valor);
		log_info(logger,"strlen valor seteado %d",strlen(valor));
		log_info(logger,"tam valor %d",sentencia->tam_valor);
		int valor_status = recv(socketCliente, valor, sentencia->tam_valor,NULL);
		log_info(logger,"valor_status %d",valor_status);
		valor[sentencia->tam_valor] = '\0';

		//set char array
		strncpy(val,valor,sentencia->tam_valor);
		val[sentencia->tam_valor] = '\0';

		//set char p
		//free(valor);
		valor = strdup(val);
		valor[sentencia->tam_valor] = '\0';
		log_info(logger,"SET -- TAMANIO VALOR %d, esi valor recibido: %s", sentencia->tam_valor, valor);
		log_info(logger,"tam recibido: %d, tam seteado: %d",sentencia->tam_valor,strlen(valor));

		log_info(logger,"valor3: \'%s\' tam recibido: %d, tam seteado: %d",val,sentencia->tam_valor,strlen(valor));
	}else valor = strdup("");

	//Armo una variable interna para manejar la sentencia
	t_sentencia * sentencia_con_punteros = armar_sentencia(sentencia, valor);
	devolverCodigoResultadoAESI(socketCliente, CORRECTO, sentencia_con_punteros->pid );
	free(valor);

	}

		break;
	default:
		//TODO no se reconoció el tipo operación
		break;
	}
}

void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case esi:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case planificador:
		//if (leer_planificador_request){
			interpretarOperacionPlanificador(hd,socketCliente);
		//}
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header > 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado");
    	}
    	else {
    		interpretarHeader(header, socketCliente);
    	};
    	//Si es instancia, solamente le mando la configuracion y cierro el hilo.
    	if (header->proceso_origen == instancia || header->proceso_origen == planificador){
    		status_header =-1;
    		log_warning(logger,	"Cerrando hilo para proceso no ESI");
    	}
   	}

	if (header->proceso_origen == esi){
	    close(socketCliente);
	}

    free(header);

}

int main(int argc, char **argv){

	seteosIniciales(argv[1]);

	pthread_mutex_init(&mutexInstancias, NULL);
	sem_init(&semInstancias, 0, 1);
	//pthread_mutex_init(&bloqueo_de_Instancias, NULL);
//	pthread_mutex_lock(&bloqueo_de_Instancias);
//	pthread_mutex_unlock(&bloqueo_de_Instancias;
	//sem_wait(&semInstancias);
//	sem_post(&semInstancias);


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
