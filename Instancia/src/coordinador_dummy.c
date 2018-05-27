/*
 * coordinador_dummy.c
 *
 *  Created on: 12 may. 2018
 *      Author: utnso
 */




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

int total_hilos = 0;

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	return serverInfo;
}


int main()
{
	struct addrinfo *serverInfo = crear_addrinfo();
	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	printf("socket creado \n");

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

    //while (1){
    	printf("Esperando mensaje\n");
    	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
		printf("Escuchando? %d \n",socketCliente);

		//crear_hilo_conexion(socketCliente, escucharMensajesEntrantes);


		/////////////////////////////////////
		//TODO
		//esta parte es para prueba. seguir con el dummyclient hasta que llegue a administrarSentencia. ahi borrar toda esta preuba pedorra y armar como se debe

		int status_header = 1;		// Estructura que manjea el status de los recieve.
		printf("Cliente conectado. Esperando mensajes:\n");

		// Envio de sentencia
			t_esi_operacion_sin_puntero * t = malloc(sizeof(t_esi_operacion_sin_puntero));
			char *c = "deportes:messi";
			char *v = "Lionel";

			memcpy(t->clave,c, strlen(c));
			t->clave[strlen(c)] = '\0';
			memcpy(t->valor,v, strlen(v));
			t->valor[strlen(v)] = '\0';
			t->keyword = 1;

		   // le envio al coordinador la linea parseada
			ContentHeader * header_a_coord_de_ESI = malloc(sizeof(ContentHeader));
			header_a_coord_de_ESI->cantidad_a_leer = sizeof(t_esi_operacion_sin_puntero);
			header_a_coord_de_ESI->operacion = 4201;
			header_a_coord_de_ESI->proceso_tipo = 2;
			printf("mandando header..: \n");
			printf("op %d \n",header_a_coord_de_ESI->operacion);
			printf("p tipo: %d \n",header_a_coord_de_ESI->proceso_tipo);
			printf("cant a leer: %d \n",header_a_coord_de_ESI->cantidad_a_leer);

			int resultado = send(socketCliente, header_a_coord_de_ESI, sizeof(ContentHeader), 0);

			printf("header: %d \n",resultado);

			printf("mandando sentencia..: \n");
			resultado = send(socketCliente, t, sizeof(t_esi_operacion_sin_puntero),0);
			printf("sentencia: %d \n",resultado);

    close(listenningSocket);
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
