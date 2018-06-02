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

void enviarSentenciaInstancia(t_sentencia * sentencia, socket){
	//t_instancia * proxima = siguienteInstanciaSegunAlgoritmo();

	if (string_equals_ignore_case(proxima->nombre, "ERROR")){
		free(proxima);
		return;
	}
	t_instancia * proxima = malloc(sizeof(t_instancia));
	proxima->socket = socket;
	t_content_header * header = crear_cabecera_mensaje(coordinador,instancia,COORDINADOR_INSTANCIA_SENTENCIA, sizeof(t_content_header));

	t_esi_operacion_sin_puntero * s_sin_p = armar_esi_operacion_sin_puntero(sentencia);

	/*t_esi_operacion_sin_puntero * s_sin_p = malloc(sizeof(t_esi_operacion_sin_puntero));
	strncpy(s_sin_p->clave, sentencia->clave,40);
	s_sin_p->keyword = sentencia->keyword;
	s_sin_p->tam_valor = sizeof(sentencia->valor);
	s_sin_p->pid = sentencia->pid;
*/
	int header_envio = send(proxima->socket,header,sizeof(t_content_header),NULL);
	int sentencia_envio = send(proxima->socket, s_sin_p, sizeof(t_esi_operacion_sin_puntero),NULL);
//	int valor_envio = send(proxima->socket,sentencia->valor,sizeof(sentencia->valor),NULL);

	if (sentencia->keyword == SET){
		int valor_envio = send(proxima->socket,sentencia->valor,strlen(sentencia->valor),NULL);
	}
	log_info(logger,"Enviada sentencia a instancia");
	free(header);
	free(s_sin_p);
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	log_info(logger,"%d",debug_var); debug_var++;
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			//TODO leer packete para obtener nombre.
			char * nombre = malloc(hd->cantidad_a_leer);
			int status_recv = recv(socketInstancia, nombre, hd->cantidad_a_leer, NULL);
			log_info(logger,"Tamaño nombre: %d - Nombre: %s",sizeof(nombre),nombre);
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia, nombre);

			t_sentencia * una_sentencia = malloc(sizeof(t_sentencia));
			una_sentencia->clave="prueba";
			una_sentencia->keyword=SET;
			una_sentencia->valor= "opopop";
			una_sentencia->pid= 2;

			enviarSentenciaInstancia(una_sentencia, socketInstancia);

			una_sentencia->clave="prueba2";
			una_sentencia->keyword=STORE;
			una_sentencia->valor= "";
			una_sentencia->pid= 2;

			enviarSentenciaInstancia(una_sentencia, socketInstancia);

			una_sentencia->clave="prueba2";
			una_sentencia->keyword=GET;
			una_sentencia->valor= "";
			una_sentencia->pid= 3;

			enviarSentenciaInstancia(una_sentencia, socketInstancia);

			free(nombre);
			break;
		default:
			break;
	}
}


int puedoEjecutarSentencia(t_sentencia * sentencia){

	//TODO Verificar existencia de clave en alguna instancia.
	//	   Si no existe, crearla (internamente?)
	//	   Si existe, verificar conexión de instancia.
	//	   Si no está conectada abortar esi.
	//TODO Preguntarle a Planificador si la clave no esta bloqueada.
	if(	hay_instancias != 0){return CORRECTO;}else return ABORTAR;
}


void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){
	case esi:
		interpretarOperacionESI(hd,socketCliente);
		break;
	case instancia:
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case planificador:
		interpretarOperacionPlanificador(hd,socketCliente);
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

    while (status_header != 0){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	log_info(logger, "Recv - Status Header: %d", status_header);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado"); total_hilos--;
    	}
    	else {
    		log_info(logger, "Interpretando header...");
    		interpretarHeader(header, socketCliente);
    	};
   	}
    free(header);
    close(socketCliente);
}

int main(int argc, char **argv){
	if (argv[1] != NULL){
		seteosIniciales(argv[1]);
		log_info(logger,"cargado arch por input");}
	else {
		seteosIniciales("config.txt");
		log_info(logger,"cargado arch por default");}

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
