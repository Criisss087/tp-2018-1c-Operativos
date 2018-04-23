/*
 ============================================================================
 Name        : Planificador.c
 Author      : La Orden Del Socket
 Version     : 0.1
 Copyright   : Si nos copias nos desaprueban cel coloquio
 Description : Planificador
 ============================================================================
 */

#include "Planificador.h"

int main(void) {

	pthread_t t_cord_id;
	pthread_t t_servidor_id;
	pthread_t t_consola_id;

	pthread_attr_t t_cord_attr;
	pthread_attr_t t_servidor_attr;
	pthread_attr_t t_consola_attr;

	// Conectar al Coordinador
	pthread_attr_init(&t_cord_attr);
	pthread_create(&t_cord_id, &t_cord_attr, conectar_coordinador, NULL);

	// Iniciarse como servidor
	pthread_attr_init(&t_servidor_attr);
	pthread_create(&t_servidor_id, &t_servidor_attr, (void *)iniciar_servidor, NULL);


	pthread_join(t_cord_id, NULL);
	pthread_join(t_servidor_id, NULL);

	// Abrir la consola
	pthread_attr_init(&t_consola_attr);
	pthread_create(&t_consola_id, &t_consola_attr, (void *)consola, NULL);

	pthread_join(t_consola_id, NULL);


	return EXIT_SUCCESS;
}

void *conectar_coordinador() {

	int server_socket;
	int familiaProt = PF_INET;
	int tipo_socket = SOCK_STREAM;
	int protocolo = 0; //ROTO_TCP;

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PORT, &hints, &server_info); // Carga en server_info los datos de la conexion


	printf("\nIniciando como cliente hacia el coordinador...\n");

	server_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (server_socket != -1)
		printf("Socket creado correctamente!\n");
	else
		printf("Error al crear el socket\n");

	int res_connect = connect(server_socket, server_info->ai_addr, server_info->ai_addrlen);
	if (res_connect < 0)
	{
		printf("Error al intentar conectar al servidor\n");
		pthread_exit(0);
	}
	else
		printf("Conectando con el servidor...\n");

	freeaddrinfo(server_info);

	char *mensaje = malloc(1000);

	//Intento enviar mensaje al coordinador
	int res_send = send(server_socket, mensaje, sizeof(mensaje), 0);
	printf("Intentando mandar un mensaje vacío...\n");

	free(mensaje);

	int res_close = close(server_socket);
	printf("Cerrando el socket y saliendo el hilo...\n");

	pthread_exit(0);
}


int *iniciar_servidor() {

	printf("Iniciando como servidor...\n");

	pthread_exit(0);
	return EXIT_SUCCESS;
}



int *consola() {

	printf("\nAbriendo consola...\n");
	printf("Ingrese un comando...\n");

	int comando_key;
	char *buffer;
	char *comando;
	char *parametro1;
	char *parametro2;

	do
	{

		buffer = leer_linea();
		//scanf("%s", buffer);

		//printf("escribiste el string %s\n",buffer);
		obtener_parametros(buffer, &comando, &parametro1, &parametro2);
		//printf("Despues de obtenr par\n");
	//	printf("escribiste: %s %s %s\n",comando, parametro1, parametro2);
		//printf("escribiste: %s len %d \n",comando,strlen(comando) );


		//Por ahora no separo el comando de los parametros
		comando_key = obtener_key_comando(comando);

		switch(comando_key)
		{
			case pausar:
				printf("Estas intentando pausar...\n");
				break;
			case continuar:
				printf("Estas intentando continuar...\n");
				break;
			case bloquear:
				printf("Bloquear clave: %s id: %s\n",parametro1, parametro2);
				break;
			case desbloquear:
				printf("Desbloquear clave: %s id: %s\n",parametro1, parametro2);
				break;
			case listar:
				printf("Listar recurso: %s\n",parametro1);
				break;
			case kill:
				printf("KILL ID: %s\n",parametro1);
				break;
			case status:
				printf("Status clave: %s \n",parametro1);
				break;
			case deadlock:
				printf("Si tan solo supiera que es...\n");
				break;
			case salir:
				printf("Terminando consola...\n");
				break;
			default:
				printf("No reconozco el comando vieja...\n");
				break;
		}

		//free(parametro1);
		//free(parametro2);

	}while(strcmp(comando,"exit"));

	//free(comando);

	pthread_exit(0);
	return EXIT_SUCCESS;
}

int obtener_key_comando(char* comando)
{
	int key = -1;

	if(!strcmp(comando, "pausar"))
		key = pausar;

	if(!strcmp(comando, "continuar"))
		key = continuar;

	if(!strcmp(comando, "bloquear"))
		key = bloquear;

	if(!strcmp(comando, "desbloquear"))
		key = desbloquear;

	if(!strcmp(comando, "listar"))
		key = listar;

	if(!strcmp(comando, "kill"))
		key = kill;

	if(!strcmp(comando, "status"))
		key = status;

	if(!strcmp(comando, "deadlock"))
		key = deadlock;

	if(!strcmp(comando, "exit"))
		key = salir;

	return key;
}

void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2)
{
	char line[255];
	char *des_line;

	strcpy(line, buffer);

	des_line = strtok(line, " \n");
	//printf("parseado comando: %s",des_line);
	if(des_line != NULL)
	{
		*comando = (char*)malloc(strlen(des_line)+1);
		(*comando)[strlen(des_line)]= '\n';
		strcpy(*comando, des_line);
	}

	des_line = strtok(NULL, " \n");
	if(des_line != NULL)
	{
		*parametro1 = (char*)malloc(strlen(des_line)+1);
		(*parametro1)[strlen(des_line)]= '\n';
		strcpy(*parametro1, des_line);
		//printf("parseado comando: %s",des_line);
	}


	des_line = strtok(NULL, " \n");
	if(des_line != NULL)
	{
		*parametro2 = (char*)malloc(strlen(des_line)+1);
		(*parametro2)[strlen(des_line)]= '\n';
		strcpy(*parametro2, des_line);
	//	printf("parseado comando: %s",des_line);
	}

}

char * leer_linea(void)
{

	char * line = malloc(100), * linep = line;
	    size_t lenmax = 100, len = lenmax;
	    int c;

	    if(line == NULL)
	        return NULL;

	    for(;;) {
	        c = fgetc(stdin);
	        if(c == EOF)
	            break;

	        if(--len == 0) {
	            len = lenmax;
	            char * linen = realloc(linep, lenmax *= 2);

	            if(linen == NULL) {
	                free(linep);
	                return NULL;
	            }
	            line = linen + (line - linep);
	            linep = linen;
	        }

	        if((*line++ = c) == '\n')
	            break;
	    }
	    *line = '\0';
	    return linep;
}
