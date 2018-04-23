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

	// TODO Hacerlo cliente
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


// TODO Armar la consola
int *consola() {

	printf("\nAbriendo consola...\n");
	printf("Ingrese un comando...\n");

	int comando_key;
	char buffer[255];
	char *comando;
	char *parametros;

	do
	{
		scanf("%s", buffer);

		//Por ahora no separo el comando de los parametros
		comando_key = obtener_key_comando(buffer);
		switch(comando_key)
		{
			case pausar:
				printf("Estas intentando pausar...\n");
				break;
			case continuar:
				printf("Estas intentando continuar...\n");
				break;
			case bloquear:
				printf("Estas intentando bloquear nada...\n");
				break;
			case desbloquear:
				printf("Estas intentando desbloqear nada...\n");
				break;
			case listar:
				printf("Nada que listar...\n");
				break;
			case kill:
				printf("Nada para matar...\n");
				break;
			case status:
				printf("Status de una clave...\n");
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

	}while(strcmp(buffer,"exit"));



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
