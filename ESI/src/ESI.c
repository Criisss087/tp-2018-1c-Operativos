/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include <Utilidades.h>
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

t_esi_operacion_sin_puntero transformarSinPuntero(t_esi_operacion t){
	t_esi_operacion_sin_puntero tsp;
	int keyword = t.keyword;
	char * valorp = NULL;
	char * clavep;
	char valor[40];
	char clave[40];

	/*
	get 0
	set 1
	store 2
	*/

	switch(keyword){
	case 0:
		clavep = t.argumentos.GET.clave;
		break;
	case 1:
		clavep = t.argumentos.SET.clave;
		valorp = t.argumentos.SET.valor;
		break;
	case 2:
		clavep = t.argumentos.STORE.clave;
		break;
	default: break;
	}

	tsp.keyword = keyword;

	strncpy(tsp.clave, clavep, sizeof clave - 1);
	tsp.clave[strlen(clavep)-1] = '\0';

	if (keyword == 1 ) {
		strncpy(tsp.valor, valorp, sizeof valor - 1);
		tsp.valor[strlen(valorp)-1] = '\0';
	};

	return tsp;
}

int main(int argc, char **argv){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
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

	printf("Conectado al servidor: %d \n",coord);
	printf("Conectado al servidor: %d \n",planif);

	fp = fopen(argv[1], "r");

	char package[PACKAGESIZE];
	char rtaCoord[PACKAGESIZE];
	char ordenDeLectura[PACKAGESIZE];

	//recibo orden del planif

	ContentHeader * header_a_ESI_de_planif = malloc(sizeof(ContentHeader));
	recv(serverPlanif, &header_a_ESI_de_planif, sizeof(ContentHeader),0);

	if(header_a_ESI_de_planif->operacion == 3101){
		recv(serverPlanif, ordenDeLectura, sizeof(ordenDeLectura),0);
		//leo el archivo y parseo
		if ((read = getline(&line, &len, fp)) != -1) {
			     t_esi_operacion parsed = parse(line);
                    //transformo el t_esi_operacion a un tipo que se pueda enviar correctamente
				    if(parsed.valido){
				    transformarSinPuntero(parsed);
				    t_esi_operacion_sin_puntero * t = malloc(sizeof(t_esi_operacion_sin_puntero));

				    //le envio al coordinador la linea parseada

				    	    ContentHeader * header_a_coord_de_ESI = malloc(sizeof(ContentHeader));
				    		header_a_coord_de_ESI->cantidad_a_leer = sizeof(t_esi_operacion_sin_puntero);
				    		header_a_coord_de_ESI->operacion = 1401;
				    		header_a_coord_de_ESI->proceso_tipo = 1;
				    		printf("mandando header..: \n");
				    		printf("op %d \n",header_a_coord_de_ESI->operacion);
				    		printf("p tipo: %d \n",header_a_coord_de_ESI->proceso_tipo);
				    		printf("cant: %d \n",header_a_coord_de_ESI->cantidad_a_leer);

				    		int resultado = send(serverCoord, header_a_coord_de_ESI, sizeof(ContentHeader), 0);

				    		printf("header: %d \n",resultado);

				    		printf("mandando sentencia..: \n");
				    		resultado = send(serverCoord, t, sizeof(t_esi_operacion_sin_puntero),0);
				    	    printf("sentencia: %d \n",resultado);

				    //recibo la rta del coord
				            ContentHeader * header_a_ESI_de_coord = malloc(sizeof(ContentHeader));
				            recv(serverCoord, &header_a_ESI_de_coord, sizeof(ContentHeader),0);
				            if(header_a_ESI_de_coord->operacion == 4102){
				            recv(serverCoord, rtaCoord, sizeof(rtaCoord),0);
				            }

				    //envio al planif lo que me mando el coord
				    		ContentHeader * header_a_planif_de_ESI = malloc(sizeof(ContentHeader));
				    		send(serverPlanif, &header_a_planif_de_ESI, sizeof(ContentHeader),0);
				    		if(header_a_planif_de_ESI->operacion == 1302){
				            send(serverPlanif, rtaCoord, sizeof(rtaCoord),0);
				         	}


				    destruir_operacion(parsed);
				    }
			}
	}

	fclose(fp);

    if (line)
        free(line);

	close(serverCoord);
	close(serverPlanif);
		return 0;
}

