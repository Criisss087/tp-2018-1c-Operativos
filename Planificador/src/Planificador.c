/*
 ============================================================================
 Name        : Planificador.c
 Author      : La Orden Del Socket
 Version     : 0.1
 Copyright   : Si nos copias nos desaprueban el coloquio
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
/*
	// Conectar al Coordinador
	pthread_attr_init(&t_cord_attr);
	pthread_create(&t_cord_id, &t_cord_attr, conectar_coordinador, NULL);
	pthread_join(t_cord_id, NULL);
*/
	// Iniciarse como servidor
	pthread_attr_init(&t_servidor_attr);
	pthread_create(&t_servidor_id, &t_servidor_attr, (void *)iniciar_servidor, NULL);
	pthread_join(t_servidor_id, NULL);

/*
	// Abrir la consola
	pthread_attr_init(&t_consola_attr);
	pthread_create(&t_consola_id, &t_consola_attr, (void *)consola, NULL);
	pthread_join(t_consola_id, NULL);
*/

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

	struct sockaddr_in dir_serv_pl;
	struct sockaddr_in dir_cli_pl;

	unsigned int dir_cli_size;

	dir_serv_pl.sin_family = AF_INET;
	dir_serv_pl.sin_addr.s_addr = INADDR_ANY;
	dir_serv_pl.sin_port = htons(8080);

	printf("Iniciando como servidor...\n");

	int serv_pl = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(serv_pl,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));


	if(bind(serv_pl, (void*)&dir_serv_pl, sizeof(dir_serv_pl)) != 0)
	{
		printf("Falló el bind del servidor\n");
		return 1;
	}


	printf("Servidor escuchando\n");
	listen(serv_pl, 10);

	int cliente = accept(serv_pl, (void*)&dir_cli_pl, &dir_cli_size);
	printf("Recibí una nueva coneccion (%d) \n", cliente);
	send(cliente, "Te conectaste al planificador!\n",32,0);

	for(;;);


	//close(serv_pl);
	pthread_exit(0);
	return EXIT_SUCCESS;
}



int *consola() {

	int comando_key;
	char *buffer = NULL;
	char *comando = NULL;
	char *parametro1 = NULL;
	char *parametro2 = NULL;

	printf("\nAbriendo consola...\n");

	do
	{
		printf("Ingrese un comando: ");

		buffer = leer_linea();
		//scanf("%s", buffer);

		//printf("buffer: %s\n",buffer);
		// Separa la linea de consola en comando y sus parametros
		obtener_parametros(buffer, &comando, &parametro1, &parametro2);
		//printf("comando: %s p1: %s p2: %s\n",comando,parametro1,parametro2);

		// Obtiene la clave del comando a ejecutar para el switch
		comando_key = obtener_key_comando(comando);

		switch(comando_key)
		{
			case pausar:
				pausar_consola();
				break;
			case continuar:
				continuar_consola();
				break;
			case bloquear:
				bloquear_clave(parametro1,parametro2);
				break;
			case desbloquear:
				desbloquear_clave(parametro1, parametro2);
				break;
			case listar:
				listar_recurso(parametro1);
				break;
			case kill:
				kill_id(parametro1);
				break;
			case status:
				status_clave(parametro1);
				break;
			case deadlock:
				deadlock_consola();
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
	}

	des_line = strtok(NULL, " \n");
	if(des_line != NULL)
	{
		*parametro2 = (char*)malloc(strlen(des_line)+1);
		(*parametro2)[strlen(des_line)]= '\n';
		strcpy(*parametro2, des_line);
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


void pausar_consola(void)
{
	printf("Estas intentando pausar...\n");
	return;
}

void continuar_consola(void)
{
	printf("Estas intentando continuar...\n");
	return;
}

void bloquear_clave(char* clave , char* id)
{
	if(clave == NULL || id == NULL)
		printf("Parametros incorrectos (bloquear <clave> <id>)\n");
	else
		printf("Bloquear clave: %s id: %s\n",clave, id);
	return;
}

void desbloquear_clave(char* clave, char* id)
{
	if(clave == NULL || id == NULL)
		printf("Parametros incorrectos (desbloquear <clave> <id>)\n");
	else
		printf("Desbloquear clave: %s id: %s\n",clave, id);

	return;

}

void listar_recurso(char* recurso)
{
	if(recurso == NULL)
		printf("Parametros incorrectos (listar <recurso>)\n");
	else
		printf("Listar recurso: %s\n",recurso);

	return;
}


void kill_id(char* id)
{
	if(id == NULL)
		printf("Parametros incorrectos (kill <id>)\n");
	else
		printf("KILL ID: %s\n",id);

	return;
}


void status_clave(char* clave)
{
	if(clave == NULL)
		printf("Parametros incorrectos (status <clave>)\n");
	else
		printf("Status clave: %s \n",clave);

	return;
}


void deadlock_consola(void)
{
	printf("Si tan solo supiera que es...\n");
	return;
}


