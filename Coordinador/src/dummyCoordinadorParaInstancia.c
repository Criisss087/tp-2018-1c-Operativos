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

void guardarClaveInternamente(char clave[40]){
	log_warning(logger,"TODO - guardarClaveInternamente");
}

rta_envio enviarSentenciaInstancia(t_sentencia * sentencia){

	rta_envio rta;
	rta.instancia = malloc(sizeof(t_instancia));
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

	t_instancia * proxima = siguienteInstanciaSegunAlgoritmo();
	log_info(logger,"Enviando a instancia: '%s' %s %s",proxima->nombre,sentencia->clave, sentencia->valor);

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
	int header_envio = send(proxima->socket,header,sizeof(t_content_header),NULL);
	int sentencia_envio = send(proxima->socket, s_sin_p, sizeof(t_esi_operacion_sin_puntero),NULL);
	if (sentencia->keyword == SET_){
		int valor_envio = send(proxima->socket,sentencia->valor,strlen(sentencia->valor),NULL);
	}

	log_info(logger,"Enviada sentencia a instancia");
	log_info(logger, "Esperando rta de Instancia");
//Espero rta de Instancia

//	sem_wait(&semInstancias);
	int header_rta_instancia = recv(proxima->socket,header,sizeof(t_content_header), NULL);
//	sem_post(&semInstancias);

	log_info(logger, "Rta Instancia Header: - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",header->proceso_origen,header->proceso_receptor,header->operacion,header->cantidad_a_leer);

	int * cod_rta = malloc(sizeof(int));
//	sem_wait(&semInstancias);
	header_rta_instancia = recv(proxima->socket,cod_rta,sizeof(int), NULL);
	//sem_post(&semInstancias);
	rta.cod= *cod_rta;
	log_info(logger, "Rta Instancia '%s'respuesta: - %d ",proxima->nombre,*cod_rta);
	log_info(logger, "Rta Instancia '%s'respuesta: - %d ",proxima->nombre,rta.cod);
	//Si estoy pidiendo el valor de la clave, recibo la clave:
	//GET - Sé que no tiene que ir a la instancia. Voy a usar este código cuando necesite sabes el valor de la clave
	if (sentencia->keyword == GET_){
		char * valor = malloc(header->cantidad_a_leer);
		header_rta_instancia = recv(proxima->socket,valor,header->cantidad_a_leer, NULL);
		rta.valor = strdup(valor);
	}

	log_info(logger,"Recibido valor de instancia '%s': Clave '%s' - Valor '%s'",rta.instancia->nombre,sentencia->clave,sentencia->valor);

	//free(proxima->nombre);
	//free(proxima);
	//NO libero la instancia "proxima" porque apunnta a la lista de instancias. si la libero al estoy borrando de la lista.
	free(header);
	free(s_sin_p);

	return rta;
}

void enviarSentenciasDummy(int socketInstancia){
	char clave1[40] = "claveprueba1";
	char clave2[40] = "claveprueba2";
	char clave3[40] = "claveprueba3";
	char * valor1 = "valor1";
	char * valor2 = "valor2";
	char * valor3 = "valor3";


	t_sentencia * sentencia = malloc(sizeof(t_sentencia));
	sentencia->pid = 0;

	strncpy(sentencia->clave,clave1,40);
	sentencia->keyword = SET_;
	sentencia->valor = valor1;

	enviarSentenciaInstancia(sentencia);

	//strncpy(sentencia->clave,clave1,40);
	sentencia->keyword = SET_;
	sentencia->valor = valor2;

	enviarSentenciaInstancia(sentencia);


	//strncpy(sentencia->clave,clave1,40);
	sentencia->keyword = STORE_;
	sentencia->valor = valor3;

	enviarSentenciaInstancia(sentencia);

}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	log_warning(logger,"aca - %d",hd->operacion);
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			//TODO leer packete para obtener nombre.
			char * nombre = malloc(hd->cantidad_a_leer);
			int status_recv = recv(socketInstancia, nombre, hd->cantidad_a_leer, NULL);
			log_info(logger,"Tamaño nombre: %d - Nombre: %s",strlen(nombre),nombre);
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia, nombre);
			enviarSentenciasDummy(socketInstancia);
			free(nombre);
			break;

		default:
			break;
	}
}

void indicarCompactacionATodasLasInstancias(){
	//TODO
	log_warning(logger,"TODO: Indicar a las instancias que compacten");
}


void interpretarHeader(t_content_header * hd, int socketCliente){
	log_info(logger, "Interpretando header - Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d",hd->proceso_origen,hd->proceso_receptor,hd->operacion,hd->cantidad_a_leer);

	switch(hd->proceso_origen){

	case instancia:
//		pthread_mutex_lock(&bloqueo_de_Instancias);
		interpretarOperacionInstancia(hd,socketCliente);
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

    while (status_header != -1){

    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	log_info(logger, "Recv - Status Header: %d", status_header);
    	if (status_header == 0) {
    		log_info(logger, "Desconectado"); total_hilos--;
    	}
    	else {
    		log_info(logger, "Interpretando header...");
    		interpretarHeader(header, socketCliente);
    	};
    	//Si es instancia, solamente le mando la configuracion y cierro el hilo.
    	if (header->proceso_origen == instancia){
    		status_header =-1;
    		log_warning(logger,	"cerrando hilo para instancia");
    	}
   	}

	if (header->proceso_origen != instancia){
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
