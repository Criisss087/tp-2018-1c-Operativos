/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include "Utilidades.h"

int main(int argc, char **argv){

	FILE * archivo_a_leer_por_el_ESI;
	char * linea_a_parsear = NULL;
	size_t direccion_de_la_linea_a_parsear = 0;
	ssize_t read;
	ssize_t sentencia;
	ssize_t sentencia_actual = 1;
	ssize_t sentencia_anterior;

	//Me conecto al coordinador y al planificador
	printf("Iniciando conexion a servidores... \n");
	int serverCoord = conectar_coordinador(IP_COORDINADOR, PUERTO_COORDINADOR);
	int serverPlanif = conectar_planificador(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);

	printf("Abriendo archivo a leer...\n");
	archivo_a_leer_por_el_ESI = fopen(argv[1], "r");

	//Leo el archivo y parseo
	while(!feof(archivo_a_leer_por_el_ESI)){

		//Recibo orden del planificador
		printf("Esperando orden del planificador para comenzar...\n");

		t_content_header *content_header = malloc(sizeof(t_content_header));
		int read_size = recv(serverPlanif, content_header, sizeof(t_content_header), (int)NULL);

		t_confirmacion_sentencia *confirmacion = malloc(sizeof(t_confirmacion_sentencia));
		read_size = recv(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia), 0);

		if(content_header->operacion == RECIBIR_ORDEN_EJECUCION){
			printf("Orden recibida, comienzo el parseo \n");
		}

		free(content_header);

		if (sentencia_actual != -1){

			if(confirmacion->ejec_anterior == 0){
				read = getline(&linea_a_parsear, &direccion_de_la_linea_a_parsear, archivo_a_leer_por_el_ESI);
				sentencia = read;
				sentencia_actual = sentencia;
			}

			if(confirmacion->ejec_anterior == 1){
				sentencia_anterior = sentencia;
				sentencia_actual = sentencia_anterior;
			}

			t_esi_operacion parsed = parse(linea_a_parsear);

			if(parsed.valido){

				//Transformo el t_esi_operacion a un tipo que se pueda enviar correctamente
				t_esi_operacion_sin_puntero  *parse_sin_punteros = malloc(sizeof(t_esi_operacion_sin_puntero));
				parse_sin_punteros = transformarSinPunteroYagregarpID(parsed, confirmacion->pid);

				printf("Enviando linea parseada al coordinador... \n");
				content_header = crear_cabecera_mensaje(esi, coordinador, ENVIAR_SENTENCIA_COORD, sizeof(t_esi_operacion_sin_puntero));
				int resultado = send(serverCoord, content_header, sizeof(t_content_header), 0);
				resultado = send(serverCoord, parse_sin_punteros, sizeof(t_esi_operacion_sin_puntero),0);

				if(parse_sin_punteros->keyword == SET){
					printf("Enviando valor de la clave necesaria para el coordinador... \n");
					printf("La clave es: %s\n", parsed.argumentos.SET.valor);

					int envio_valor_clave = send(serverCoord, parsed.argumentos.SET.valor , strlen(parsed.argumentos.SET.valor),0);
				}

				destruir_cabecera_mensaje(content_header);


				//Recibo respuesta del coordinador
				printf("Recibiendo respuesta del coordinador...\n");
				respuesta_coordinador *respuesta_coordinador = malloc (sizeof(respuesta_coordinador));
				recv(serverCoord, respuesta_coordinador, sizeof(respuesta_coordinador),0);

				content_header = malloc(sizeof(t_content_header));
				recv(serverCoord, content_header, sizeof(t_content_header),0);

				//SI recibo orden de abortar
				if(content_header->operacion == RECIBIR_RESULTADO_SENTENCIA_COORD && respuesta_coordinador->resultado_del_parseado == ABORTAR){
					printf("Recibi orden de aborto, aviso al planificador y fin de ejecucion. \n");
					free(content_header);

					//Aviso al planificador que recibi orden de abortar
					content_header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF, sizeof(t_content_header));
					int enviar_aviso_abortar = send(serverPlanif, content_header, sizeof(t_content_header),0);
					confirmacion->resultado = ABORTAR;
					enviar_aviso_abortar = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

					//Cierro!
					free(respuesta_coordinador);
					destruir_cabecera_mensaje(content_header);
					free(confirmacion);
					destruir_operacion(parsed);

					if(linea_a_parsear){
						free(linea_a_parsear);
					}

					fclose(archivo_a_leer_por_el_ESI);
					close(serverCoord);
					close(serverPlanif);

					exit(EXIT_FAILURE);
				}


				if(content_header->operacion == RECIBIR_RESULTADO_SENTENCIA_COORD && respuesta_coordinador->resultado_del_parseado != ABORTAR){
					confirmacion->resultado = respuesta_coordinador->resultado_del_parseado;
					free(respuesta_coordinador);
				}

				free(content_header);


				//Envio al planificador lo que me mando el coordinador
				printf("Enviando al planificador la respuesta del coordinador...\n");
				content_header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF, sizeof(t_content_header));
				int enviar_rdo_planif = send(serverPlanif, content_header, sizeof(t_content_header),0);
				enviar_rdo_planif = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

				destruir_cabecera_mensaje(content_header);

				free(confirmacion);
				destruir_operacion(parsed);

			}//if parsed valido

		}//if read=getline...

	}//while...


	if(linea_a_parsear){
		free(linea_a_parsear);
	}


	printf("Esperando orden del planificador para finalizar...\n");
	t_content_header* content_header = malloc(sizeof(t_content_header));
	int read_size = recv(serverPlanif, content_header, sizeof(t_content_header), (int)NULL);

	t_confirmacion_sentencia * confirmacion = malloc(sizeof(t_confirmacion_sentencia));
	read_size = recv(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia), 0);

	if(content_header->operacion == RECIBIR_ORDEN_EJECUCION){
		printf("Orden recibida, finaliza el proceso \n");
		free(content_header);

		content_header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF , sizeof(t_confirmacion_sentencia));

		//Le aviso al planificador que termine de leer el archivo
		int finalice_lectura = send(serverPlanif, content_header, sizeof(t_content_header),0);
		confirmacion->resultado = LISTO;
		finalice_lectura = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

		destruir_cabecera_mensaje(content_header);
		printf("Fin de ejecucion por alcanzar el fin del archivo \n");
	}

	free(confirmacion);

	fclose(archivo_a_leer_por_el_ESI);
	close(serverCoord);
	close(serverPlanif);
	return 0;

}


/*
 * Funciones para comunicarse con el coordinador y el planificador
 */


struct addrinfo* crear_addrinfo(char * ip, char * puerto){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	return serverInfo;
}

int conectar_coordinador(char * ip, char * puerto){

	struct addrinfo *serverInfoCoord = crear_addrinfo(IP_COORDINADOR, PUERTO_COORDINADOR);

	int serverCoord = socket(serverInfoCoord->ai_family, serverInfoCoord->ai_socktype, serverInfoCoord->ai_protocol);

	if (serverCoord < 0){
		printf("Error al intentar conectar al coordinador\n");
		exit(EXIT_FAILURE);
	}

	int activado = 1;
	setsockopt(serverCoord, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int resultado_conexion_coordinador = connect(serverCoord, serverInfoCoord->ai_addr, serverInfoCoord->ai_addrlen);

	if (resultado_conexion_coordinador < 0){
		freeaddrinfo(serverInfoCoord);
		close(serverCoord);
		printf("Error al intentar conectar al coordinador\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(serverInfoCoord);

	printf("Conectado al servidor coordinador: %d \n",resultado_conexion_coordinador);

	return serverCoord;

}

int conectar_planificador(char * ip, char * puerto){

	struct addrinfo *serverInfoPlanif = crear_addrinfo(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);

	int serverPlanif = socket(serverInfoPlanif->ai_family, serverInfoPlanif->ai_socktype, serverInfoPlanif->ai_protocol);

	if (serverPlanif < 0){
		printf("Error al intentar conectar al planificador\n");
		exit(EXIT_FAILURE);
	}

	int activado = 1;
	setsockopt(serverPlanif, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int resultado_conexion_planificador = connect(serverPlanif, serverInfoPlanif->ai_addr, serverInfoPlanif->ai_addrlen);

	if (resultado_conexion_planificador < 0){
		freeaddrinfo(serverInfoPlanif);
		close(serverPlanif);
		printf("Error al intentar conectar al planificador\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(serverInfoPlanif);

	printf("Conectado al servidor planificador: %d \n",resultado_conexion_planificador);

	return serverPlanif;
}
