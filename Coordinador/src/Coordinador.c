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

#define TAMANIO_ENTRADAS 8
#define CANT_MAX_ENTRADAS 5

int total_hilos = 0;
//t_list * lista_instancias = list_create();

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	return serverInfo;
}

void enviarConfiguracionInicial(int socketInstancia){

	configuracion_inicial * config = malloc(sizeof(configuracion_inicial));
	config->cantidad = CANT_MAX_ENTRADAS;
	config->tamanio = TAMANIO_ENTRADAS;

	int sent = send(socketInstancia, config, sizeof(configuracion_inicial),NULL);
	log_info(logger,"enviada config inicial %d", sent);
}

void guardarEnListaDeInstancias(int socketInstancia){

}

t_instancia siguienteInstanciaSegunAlgoritmo(){

}

void enviarAInstancia(t_instancia proxima, t_esi_operacion_sin_puntero * sentencia){

}

void enviarSentenciaInstancia(t_esi_operacion_sin_puntero * sentencia){
	t_instancia proxima = siguienteInstanciaSegunAlgoritmo();
	enviarAInstancia(proxima,sentencia);
}

void interpretarOperacionInstancia(t_content_header * hd, int socketInstancia){
	switch(hd->operacion){
		case INSTANCIA_COORDINADOR_CONEXION:
			;
			enviarConfiguracionInicial(socketInstancia);
			guardarEnListaDeInstancias(socketInstancia);
			break;
		default:
			break;
	}
}

void interpretarOperacionPlanificador(t_content_header * hd, int socketCliente){

}

void interpretarOperacionESI(t_content_header * hd, int socketCliente){
	printf("interpretando op esi\n");

	int po = hd->operacion;
	printf("p op: %d",po);
	printf(" cant: %d \n",hd->cantidad_a_leer);
	switch(hd->operacion){
	case ESI_COORDINADOR_SENTENCIA:
		printf("interpretando op esi correcta\n");
		;
		//C no permite hacer declaraciones inmediatamente después de un label.. wtf
		//Recibo de ESI sentencia parseada
		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		int cantleida = recv( socket, sentencia, hd->cantidad_a_leer, NULL);
		printf("cant leida %d\n",cantleida);
		enviarSentenciaInstancia(sentencia);
		break;
	default:
		//TODO no se reconoció el tipo operación
		break;
	}
}

void interpretarHeader(t_content_header * hd, int socketCliente){
	printf("interpretando header\n");

	//TODO: log_operacion();

	int pt = hd->proceso_origen;
	printf("p tipo %d \n",pt);
	int po = hd->operacion;
	printf("p op: %d \n",po);

	switch(hd->proceso_origen){
	case 1:
		//ESI
		interpretarOperacionESI(hd,socketCliente);
		break;
	case 2:
		interpretarOperacionInstancia(hd,socketCliente);
		break;
	case 3:
		interpretarOperacionPlanificador(hd,socketCliente);
		break;
	default:
		//TODO no se reconoció el tipo proceso
		break;
	}
}


void *escucharMensajesEntrantes(int socketCliente){

    int status_header = 1;		// Estructura que manjea el status de los recieve.

    printf("Cliente conectado. Esperando mensajes:\n");
    total_hilos++;
    printf("total hilos: %d\n",total_hilos);

    t_content_header * header = malloc(sizeof(t_content_header));

    while (status_header != 0){
    	memset(&status_header, 0, sizeof(status_header));
    	status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);
    	printf("status header: %d \n", status_header);
    	if (status_header == 0) {
    		printf("desconectado\n"); total_hilos--;
    	}
    	else {
    		printf("llamando funcion interpretarHeader\n");
    		interpretarHeader(header, socketCliente);
    	};
   	}
    close(socketCliente);
}

int main()
{
	logger = log_create("log_coordinador.txt","Coordinador",true, LOG_LEVEL_INFO);
	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(logger,"socket creado %d", listenningSocket);

    // Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    printf("socket bindeado \n");
    freeaddrinfo(serverInfo);

    printf("escuchando \n");
    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

    while (1){
    	printf("Esperando mensaje\n");
    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
		printf("Escuchando? %d \n",socketCliente);

		//crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);


		/////////////////////////////////////
		//TODO
		//esta parte es para prueba. seguir con el dummyclient hasta que llegue a administrarSentencia. ahi borrar toda esta preuba pedorra y armar como se debe

		int status_header = 1;		// Estructura que manjea el status de los recieve.
		printf("Cliente conectado. Esperando mensajes:\n");

		t_content_header * header = malloc(sizeof(t_content_header));
		status_header = recv(socketCliente, header, sizeof(t_content_header), NULL);

		printf("status header: %d \n", status_header);
		int pt = header->proceso_origen;
		printf("p tipo %d \n",pt);
		int po = header->operacion;
		printf("p op: %d \n",po);

		t_esi_operacion_sin_puntero * sentencia = malloc(sizeof(t_esi_operacion_sin_puntero));
		sentencia->keyword = 9;

		status_header = recv(socketCliente, sentencia, sizeof(t_esi_operacion_sin_puntero), NULL);

		printf("status header: %d \n", status_header);
		printf("setnencia recibida: %i - %s \n",sentencia->keyword,sentencia->clave);
		////////////////////////////////////

    }
    close(listenningSocket);
    log_destroy(logger);
    return 0;


}



/*


 inicializar() ----> 1. leerArchivoDeConfiguracion();
                     2. inicializarEstructurasAdministrativas();

 registrarEjecucion(); en el log de operaciones

void *elegirYutilizarInstancia(void*); // o gestionarInstancia (algoritmo de distribucion)






 Para trabajar con hilos:
 ------------------------
 pthread_create(direccion de memoria del thread que se va a crear, NULL, nombre de la funcion que va a usar, direccion de memoria del argumento que va a recibir);
 pthread_join(thread que se creo, donde se guarda el resultado tras ejecutar la funcion);


 */
