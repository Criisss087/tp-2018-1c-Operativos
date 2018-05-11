/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include <Utilidades.h>
#include <Funciones.c>
t_esi_operacion_sin_puntero transformarSinPuntero(t_esi_operacion t);

struct addrinfo* crear_addrinfo(ip, puerto){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	return serverInfo;
}

typedef struct {
	int proceso_tipo;
	int operacion;
	int cantidad_a_leer;
} __attribute__((packed)) ContentHeader;



int main(int argc, char **argv){
	FILE * archivo_a_leer_por_el_ESI;
	char * linea_a_parsear = NULL;
	size_t direccion_de_la_linea_a_parsear = 0;
	ssize_t read;

	struct addrinfo *serverInfoCoord = crear_addrinfo(IP_COORDINADOR, PUERTO_COORDINADOR);
	struct addrinfo *serverInfoPlanif = crear_addrinfo(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);

	int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);
	int serverPlanif = socket(serverInfoPlanif->ai_family, serverInfoPlanif->ai_socktype, serverInfoPlanif->ai_protocol);


	/* Las siguientes dos lineas sirven para no lockear el address
		int activado = 1;
		setsockopt(serverSocket1, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));*/

	int coord = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);
	int planif = connect(serverPlanif, serverInfoPlanif->ai_addr, serverInfoPlanif->ai_addrlen);

	freeaddrinfo(serverInfoCoord);
	freeaddrinfo(serverInfoPlanif);

	printf("Conectado al servidor coordinador: %d \n",coord);
	printf("Conectado al servidor planificador: %d \n",planif);

	archivo_a_leer_por_el_ESI = fopen(argv[1], "r");

	char package[PACKAGESIZE];
	char rtaCoord[PACKAGESIZE];
	char ordenDeLectura[PACKAGESIZE];

	//recibo orden del planif

	ContentHeader * header_a_ESI_de_planif = malloc(sizeof(ContentHeader));
	recv(serverPlanif, &header_a_ESI_de_planif, sizeof(ContentHeader),0);

	if(header_a_ESI_de_planif->operacion == PLANIFICADOR_ENVIA_ORDEN_ESI){
		recv(serverPlanif, ordenDeLectura, sizeof(ordenDeLectura),0);
		//leo el archivo y parseo
		if ((read = getline(&linea_a_parsear, &direccion_de_la_linea_a_parsear, archivo_a_leer_por_el_ESI)) != -1) {
			     t_esi_operacion parsed = parse(linea_a_parsear);
                    //transformo el t_esi_operacion a un tipo que se pueda enviar correctamente
				    if(parsed.valido){
				    t_esi_operacion_sin_puntero * struct_de_operacion_sin_punteros = malloc(sizeof(t_esi_operacion_sin_puntero));
				    struct_de_operacion_sin_punteros = transformarSinPuntero(parsed);

				    //le envio al coordinador la linea parseada
                            ContentHeader * header_a_coord_de_ESI = malloc(sizeof(ContentHeader));
				    		header_a_coord_de_ESI->cantidad_a_leer = sizeof(t_esi_operacion_sin_puntero);
				    		header_a_coord_de_ESI->operacion = ESI_ENVIA_COORDINADOR_SENTENCIA;
				    		header_a_coord_de_ESI->proceso_tipo = 1;

				    		int resultado = send(serverCoord, header_a_coord_de_ESI, sizeof(ContentHeader), 0);
                            resultado = send(serverCoord, struct_de_operacion_sin_punteros, sizeof(t_esi_operacion_sin_puntero),0);


				    //recibo la rta del coord
				            ContentHeader * header_a_ESI_de_coord = malloc(sizeof(ContentHeader));
				            recv(serverCoord, &header_a_ESI_de_coord, sizeof(ContentHeader),0);
				            if(header_a_ESI_de_coord->operacion == COORDINADOR_ENVIA_ESI_RESULTADO_EJECUCION_SENTENCIA){
				            recv(serverCoord, rtaCoord, sizeof(rtaCoord),0);
				            }

				    //envio al planif lo que me mando el coord
				    		ContentHeader * header_a_planif_de_ESI = malloc(sizeof(ContentHeader));
				    		send(serverPlanif, &header_a_planif_de_ESI, sizeof(ContentHeader),0);
				    		if(header_a_planif_de_ESI->operacion == ESI_ENVIA_PLANIFICADOR_RESULTADO_EJECUCION_SENTENCIA){
				            send(serverPlanif, rtaCoord, sizeof(rtaCoord),0);
				         	}


				    destruir_operacion(parsed);
				    }
			}
	}

	fclose(archivo_a_leer_por_el_ESI);

    if (linea_a_parsear)
        free(linea_a_parsear);

	close(serverCoord);
	close(serverPlanif);
		return 0;
}

