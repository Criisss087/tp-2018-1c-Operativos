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
	FILE * archivo_a_leer_por_el_ESI;
	char * linea_a_parsear = NULL;
	size_t direccion_de_la_linea_a_parsear = 0;
	ssize_t read;
	int rtaCoord;

	struct addrinfo *serverInfoCoord = crear_addrinfo(IP_COORDINADOR, PUERTO_COORDINADOR);
	struct addrinfo *serverInfoPlanif = crear_addrinfo(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);

    int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);
	int serverPlanif = socket(serverInfoPlanif->ai_family, serverInfoPlanif->ai_socktype, serverInfoPlanif->ai_protocol);

	int coord = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);
	int planif = connect(serverPlanif, serverInfoPlanif->ai_addr, serverInfoPlanif->ai_addrlen);

	freeaddrinfo(serverInfoCoord);
	freeaddrinfo(serverInfoPlanif);

	printf("Conectado al servidor coordinador: %d \n",coord);
	printf("Conectado al servidor planificador: %d \n",planif);

	archivo_a_leer_por_el_ESI = fopen(argv[1], "r");

	//leo el archivo y parseo
	while(read != -1){

			//recibo orden del planif
			t_content_header *content_header = malloc(sizeof(t_content_header));
			int read_size = recv(serverPlanif, content_header, sizeof(t_content_header), (int)NULL);
			t_confirmacion_sentencia *conf = malloc(sizeof(t_confirmacion_sentencia));
			read_size = recv(serverPlanif, conf, sizeof(t_confirmacion_sentencia), 0);

			if ((read = getline(&linea_a_parsear, &direccion_de_la_linea_a_parsear, archivo_a_leer_por_el_ESI)) != -1){
				t_esi_operacion parsed = parse(linea_a_parsear);

				//transformo el t_esi_operacion a un tipo que se pueda enviar correctamente
				if(parsed.valido){
					t_esi_operacion_sin_puntero  *parse_sin_punteros;
					parse_sin_punteros = transformarSinPunteroYagregarpID(parsed, conf->pid);

					content_header = crear_cabecera_header(/*TODO*/);
					/*content_header = malloc(sizeof(t_content_header));
					content_header->proceso_origen = esi;
					content_header->proceso_receptor = coordinador;
					content_header->operacion = 1;
					content_header->cantidad_a_leer = sizeof(t_esi_operacion_sin_puntero);
*/
					 int resultado = send(serverCoord, content_header, sizeof(t_content_header), 0);
					 resultado = send(serverCoord, parse_sin_punteros, sizeof(t_esi_operacion_sin_puntero),0);
					 int envio_valor_clave = send(serverCoord, parsed.argumentos.SET.valor, sizeof(t_esi_operacion_sin_puntero),0);

					 free(content_header);

					 //recibo la rta del coord
					 content_header = malloc(sizeof(t_content_header));
					 recv(serverCoord, content_header, sizeof(t_content_header),0);
					 if(content_header->operacion == RESULTADO_EJECUCION_SENTENCIA){
						 recv(serverCoord, rtaCoord, sizeof(rtaCoord),0);
						 conf->resultado = rtaCoord;
					 }

					 free(content_header);

					 //envio al planif lo que me mando el coord
					 content_header = malloc(sizeof(t_content_header));
					 send(serverPlanif, content_header, sizeof(t_content_header),0);
					 if(content_header->operacion == RESPUESTA_EJECUCION_SENTENCIA){
						 send(serverPlanif, conf, sizeof(t_confirmacion_sentencia),0);
					 }

					 free(content_header);
					 free(conf);
					 destruir_operacion(parsed);

				}//if parsed valido

			}// if read = getline ...

	}//while

	if(linea_a_parsear){
	         free(linea_a_parsear);
	     }

	fclose(archivo_a_leer_por_el_ESI);
	close(serverCoord);
	close(serverPlanif);
		return 0;
}

