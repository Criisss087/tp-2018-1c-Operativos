/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include "Utilidades.h"

struct addrinfo* crear_addrinfo(ip, puerto){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	return serverInfo;
}


int main(int argc, char **argv){
	//proof of concept - conexion inicial con instancia
	logger = log_create("log_dummy_instancia.txt","dummy_instancia",true, LOG_LEVEL_INFO);
	struct addrinfo *serverInfoCoord = crear_addrinfo(IP, PUERTO);
	int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);

	int activado = 1;
	setsockopt(serverCoord, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int coord = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);
	freeaddrinfo(serverInfoCoord);
	printf("Conectado al servidor: %d \n",coord);
	printf("sizeof header %d \n", sizeof(t_content_header));

	char * nombre = "Prueba Nombre Instancia";

	//Envío header
	t_content_header * header = malloc(sizeof(t_content_header));
	header->cantidad_a_leer = sizeof(nombre);
	header->proceso_origen = INSTANCIA;
	header->proceso_receptor = COORDINADOR;
	header->operacion = INSTANCIA_COORDINADOR_CONEXION;

	int status_header = send(serverCoord,header,sizeof(t_content_header), NULL);
	printf("enviado header %d \n",status_header);

	//Envío nombre
	int status_nombre = send(serverCoord, nombre, sizeof(nombre), NULL);
	printf("enviado header %d \n",status_header);
	log_info(logger, "Enviado header y nombre");

	//Recibo header
	int status_recv_header = recv(serverCoord, header, sizeof(t_content_header), NULL);
	log_info(logger, "Recibido header");
	//Recibo config
	t_configTablaEntradas * config = malloc(sizeof(t_configTablaEntradas));
	int status_recv_config = recv(serverCoord, config, sizeof(t_configTablaEntradas),NULL);
	log_info(logger, "Recibida configuración");

	log_info(logger, "Configuracion recibida: Cantidad %d - Tamaño %d",config->cantTotalEntradas,config->tamanioEntradas);
	close(serverCoord);
	return 0;
}

