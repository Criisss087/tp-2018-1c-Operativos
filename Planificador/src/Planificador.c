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

	fd_set readset, writeset, exepset;
	int max_fd;
	char read_buffer[MAX_LINEA];

	//TODO Obtener datos del archivo de configuración

	//Creo el socket servidor para recibir ESIs (ya bindeado y escuchando)
	int serv_socket = iniciar_servidor(PORT_ESCUCHA);

	//Creo el socket cliente para conectarse al coordinador
	int coord_socket = conectar_coordinador(IP_COORD, PORT_COORD );

	crear_listas_planificador();
	inicializar_conexiones_esi();
	stdin_no_bloqueante();

	while(TRUE){
		//Inicializa los file descriptor
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_ZERO(&exepset);

		//Agrega el fd del socket servidor al set de lectura y excepciones
		FD_SET(serv_socket, &readset);
		FD_SET(serv_socket, &exepset);

		//Agrega el fd del socket coordinador al set de lectura
		FD_SET(coord_socket, &readset);
		//FD_SET(coord_socket, &writeset);
		FD_SET(coord_socket, &exepset);

		//Agrega el stdin para leer la consola
		FD_SET(STDIN_FILENO, &readset);
		//FD_SET(fileno(stdin), &exepset);

		/* Seteo el maximo file descriptor necesario para el select */
		max_fd = serv_socket;

		//Agrega los conexiones esi existentes
		for (int i = 0; i < MAX_CLIENTES; i++)
		{
			if (conexiones_esi[i].socket != NO_SOCKET)
			{
				FD_SET(conexiones_esi[i].socket, &readset);
				FD_SET(conexiones_esi[i].socket, &writeset);
				FD_SET(conexiones_esi[i].socket, &exepset);
			}

			if (conexiones_esi[i].socket > max_fd)
				max_fd = conexiones_esi[i].socket;

		}

		if(max_fd < coord_socket)
			max_fd = coord_socket;

		int result = select(max_fd+1, &readset, &writeset, &exepset, NULL); //&time);

		if(result == 0)
			printf("Select time out\n");
		else if(result < 0){
			printf("Error en select\n");
			exit(EXIT_FAILURE);
		}

		else if(result > 0) //Hubo un cambio en algun fd
		{

			//Aceptar nuevas conexiones de ESI
			if (FD_ISSET(serv_socket, &readset)) {
				atender_nuevo_esi(serv_socket);
				planificar();
			}

			//Atender al coordinador
			if(FD_ISSET(coord_socket, &readset))
			{
				if(recibir_mensaje_coordinador(coord_socket) == 0)
				{
					cerrar_conexion_coord(coord_socket);
				}
			}


			/*if(FD_ISSET(coord_socket, &writeset))
			{
				printf("Entro al isset del coord WRITE\n");
				recibir_mensaje_coordinador(coord_socket);
			}
			 */
			if(FD_ISSET(coord_socket, &exepset))
			{
				if(recibir_mensaje_coordinador(coord_socket) == 0)
				{
					cerrar_conexion_coord(coord_socket);
					terminar_planificador();
				}
			}

			//Se ingresó algo a la consola
			if(FD_ISSET(STDIN_FILENO, &readset))
			{

				read_from_stdin(read_buffer, MAX_LINEA);

				int res = comando_consola(read_buffer);
				if(res)
				{
					terminar_planificador();
					break;
				}
			}

			//Manejo de conexiones esi ya existentes
			for (int i = 0; i < MAX_CLIENTES; ++i) {
				if (conexiones_esi[i].socket != NO_SOCKET ){
					//Mensajes nuevos de algun esi
					if (FD_ISSET(conexiones_esi[i].socket, &readset)) {
						if(recibir_mensaje_esi(conexiones_esi[i].socket) == 0)
						{
							cerrar_conexion_esi(&conexiones_esi[i]);
							continue;
						}
					}

					//Excepciones del esi, para la desconexion
					if (FD_ISSET(conexiones_esi[i].socket, &exepset)) {
						if(recibir_mensaje_esi(conexiones_esi[i].socket) == 0)
						{
							cerrar_conexion_esi(&conexiones_esi[i]);
							continue;
						}
					}//if isset
				} // if NO_SOCKET
			} //for conexiones_esi
		} //if result select
	} //while

	return EXIT_SUCCESS;
}

int conectar_coordinador(char * ip, char * port) {

	int familiaProt = PF_INET;
	int tipo_socket = SOCK_STREAM;
	int protocolo = 0; //ROTO_TCP;

	struct addrinfo hints;
	struct addrinfo *coord_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;  // Indica que usaremos el protocolo TCP

	getaddrinfo(ip, port, &hints, &coord_info); // Carga en server_info los datos de la conexion


	printf("\nIniciando como cliente hacia el coordinador...\n");

	int coord_socket = socket(coord_info->ai_family, coord_info->ai_socktype, coord_info->ai_protocol);
	if (coord_socket != -1)
		printf("Socket creado correctamente!\n");
	else
		printf("Error al crear el socket\n");

	int activado = 1;
	setsockopt(coord_socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	int res_connect = connect(coord_socket, coord_info->ai_addr, coord_info->ai_addrlen);
	if (res_connect < 0)
	{
		printf("Error al intentar conectar al coordinador\n");
		close(coord_socket);
		exit(EXIT_FAILURE);
	}
	else
		printf("Conectado con el coordinador! (%d) \n",coord_socket);

	freeaddrinfo(coord_info);

	return coord_socket;

}


int iniciar_servidor(unsigned short port)
{

	struct sockaddr_in dir_serv_sock;

	unsigned int dir_cli_size;

	dir_serv_sock.sin_family = AF_INET;
	dir_serv_sock.sin_addr.s_addr = INADDR_ANY;
	dir_serv_sock.sin_port = htons(port);

	printf("Creando socket servidor...\n");

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));


	if(bind(server_socket, (void*)&dir_serv_sock, sizeof(dir_serv_sock)) != 0)
	{
		printf("Falló el bind del socket servidor\n");
		exit(1);
	}


	printf("Socket servidor (%d) escuchando\n", server_socket);
	listen(server_socket, MAX_CLIENTES);

	return server_socket;

}

/*	Consola preparada para funcionar en un hilo aparte
 * 	actualmente no se está usando ya que readline es una funcion bloqueante
 * 	y necesito que el select pueda leer de stdin mientras maneja las conexiones
 * 	Es código duplicado, pero la dejo porque está bonita :)
 */

void *consola() {

	int res=0;
	char *buffer = NULL;

	printf("\nAbriendo consola...\n");

	while(TRUE){

		//Trae la linea de consola
		buffer = readline(">");

		res = comando_consola(buffer);

		free(buffer);

		//Sale de la consola con exit
		if(res)
			break;

	}

	pthread_exit(0);

}

int obtener_key_comando(char* comando)
{
	int key = -1;

	if(comando == NULL)
		return key;

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

	if(!strcmp(comando, "mostrar"))
			key = mostrar;

	if(!strcmp(comando, "ejec"))
			key = ejecucion;

	if(!strcmp(comando, "exit"))
		key = salir;

	return key;
}

void obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2)
{
	char** comandos;
	int i,j=0;

	comandos = string_n_split(buffer,3," ");

	while(comandos[j])
	{
		switch(j)
		{
			case 0:
				*comando = comandos[j];
				break;
			case 1:
				*parametro1 = comandos[j];
				break;
			case 2:
				*parametro2 = comandos[j];
				break;
		}

		j++;
	}


	for(i=0;i>j;i++)
	{
		printf("parte %d: %s\n", j,comandos[j]);
		free(comandos[j]);
	}
	free(comandos);

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

void mostrar_lista(char * name)
{

	t_list * lista;

	if(name == NULL)
		printf("Parametros incorrectos (mostrar <lista>)\n");
	else
	{
		lista = list_create();

		if(!strcmp(name, "ready"))
		{
			printf("copiando a l_ready a lista\n");
			list_add_all(lista, l_listos);
		}

		else if(!strcmp(name, "block"))
		{
			list_add_all(lista,l_bloqueados);
		}

		else if(!strcmp(name, "term"))
		{
			list_add_all(lista, l_terminados);
		}
		else
		{
			printf("No existe la lista %s: \n",name);
			return;
		}

		printf("\nEstado actual de la lista de %s: \n\n",name);

		list_iterate(lista,(void*)mostrar_esi);

		printf("\nTamaño: %d \n",list_size(lista));

		list_destroy(lista);
		printf("\n");
	}

	return;


}

void mostrar_esi_en_ejecucion(void)
{

	if(l_ejecucion!=NULL)
		printf("\nPID Esi en ejecución actual: %d: \n",l_ejecucion->pid);
	else
		printf("No hay ningun esi en ejecucion\n");


	return;
}

int comando_consola(char * buffer){

	int comando_key;
	char *comando = NULL;
	char *parametro1 = NULL;
	char *parametro2 = NULL;
	int res = 0;

	//printf("string recibido: %s\n",buffer);

	// Separa la linea de consola en comando y sus parametros
	obtener_parametros(buffer, &comando, &parametro1, &parametro2);
	//printf("comando: %s p1: %s p2: %s\n",comando,parametro1,parametro2);

	// Obtiene la clave del comando a ejecutar para el switch
	comando_key = obtener_key_comando(comando);

	switch(comando_key){
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
			res = 1;
			printf("Saliendo de la consola\n");
			break;
		case mostrar:
			mostrar_lista(parametro1);
			break;
		case ejecucion:
			mostrar_esi_en_ejecucion();
			break;
		default:
			printf("No reconozco el comando vieja...\n");
			break;
	}

	//Limpio el parametro 1
	if(parametro1 != NULL)
	{
		free(parametro1);
		parametro1 = NULL;
	}

	//Limpio el parametro 2
	if(parametro2 != NULL)
	{
		free(parametro2);
		parametro2 = NULL;
	}


	free(comando);

	return res;

}

int atender_nuevo_esi(int serv_socket)
{
	struct sockaddr_in client_addr;

	//Setea la direccion en 0
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);

	//Acepta la nueva conexion
	int new_client_sock = accept(serv_socket, (struct sockaddr *)&client_addr, &client_len);
	if (new_client_sock < 0) {
	  perror("accept()");
	  return -1;
	}

	printf("Acepté al esi con el fd: %d.\n", new_client_sock);


	//Lo agrego a la lista de conexiones esi actuales
	for (int i = 0; i < MAX_CLIENTES; ++i) {

		if (conexiones_esi[i].socket == NO_SOCKET) {
			conexiones_esi[i].socket = new_client_sock;
			conexiones_esi[i].addres = client_addr;

			//Creo el nuevo esi con su conexion
			t_pcb_esi * nuevo_esi = crear_esi(conexiones_esi[i]);

			//Agrego el esi nuevo a la cola de listos
			list_add(l_listos, nuevo_esi);
			printf("Esi %d agregado a ready!\n",nuevo_esi->pid);
	        return 0;
	    }

	 }

	 // printf("Demasiadas conexiones. Cerrando nueva conexion %s:%d.\n", client_ipv4_str, client_addr.sin_port);
	 close(new_client_sock);

	 return -1;

}

void inicializar_conexiones_esi(void)
{
	for (int i = 0; i < MAX_CLIENTES; ++i)
	{
		conexiones_esi[i].socket = NO_SOCKET;
	}

}

int recibir_mensaje_esi(int esi_socket)
{
	int read_size;
	char client_message[2000];
	t_pcb_esi *esi_aux;

	t_content_header *content_header = malloc(sizeof(t_content_header));

	read_size = recv(esi_socket, content_header, sizeof(t_content_header), NULL);

	int content_info;

	recv( socket, content_info, content_header->cantidad_a_leer, NULL);

	if(content_header->operacion == 1302){
		if(content_info == OPERACION_ESI_OK){
			// TODO Ordenar ejecutar siguiente sentencia del ESI

			l_ejecucion->instruccion_actual++;

			//int res_send = send(esi_socket, client_message, sizeof(client_message), 0);


		}
		else if(content_info == OPERACION_ESI_OK_FINAL){
			l_ejecucion->estado = finished;
			l_ejecucion->instruccion_actual++;

			esi_aux = l_ejecucion;

			list_add(l_terminados, esi_aux);

			l_ejecucion = NULL;
		}
		else if(content_info == OPERACION_ESI_BLOQUEADA){
			l_ejecucion->estado = blocked;
			esi_aux = l_ejecucion;

			list_add(l_bloqueados, esi_aux);

			l_ejecucion = NULL;
		}
	}

	//--------------------//

	// TODO Eliminar el bloque de prueba
	read_size = recv(esi_socket , client_message , 2000 , 0);
	if(read_size > 0)
	{
		printf("Esi %d dice: %s\n",esi_socket,client_message);
		int res_send = send(esi_socket, client_message, sizeof(client_message), 0);
	}


	return read_size;
}

int cerrar_conexion_esi(t_conexion_esi * esi)
{

	printf("Conexion con esi %d cerrada\n",esi->socket);
	close(esi->socket);
	esi->socket = NO_SOCKET;

	return 0;
}

int read_from_stdin(char *read_buffer, size_t max_len)
{
	char c = '\0';
	int i = 0;

	memset(read_buffer, 0, max_len);

	ssize_t read_count = 0;
	ssize_t total_read = 0;

	do{
		read_count = read(STDIN_FILENO, &c, 1);
		if (read_count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		  perror("read()");
		  return -1;
		}
		else if (read_count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		  break;
		}
		else if (read_count > 0) {
		  total_read += read_count;

		  read_buffer[i] = c;
		  i++;

		  if (total_read > max_len) {
			//printf("Message too large and will be chopped. Please try to be shorter next time.\n");
			fflush(STDIN_FILENO);
			break;
		  }
		}
	}while ((read_count > 0) && (total_read < max_len) && (c != '\n'));

	size_t len = strlen(read_buffer);
	if (len > 0 && read_buffer[len - 1] == '\n')
		read_buffer[len - 1] = '\0';

	//printf("Read from stdin %zu bytes. Let's prepare message to send.\n", strlen(read_buffer));

	return 0;
}

void stdin_no_bloqueante(void)
{
	  /* Set nonblock for stdin. */
	  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
	  flag |= O_NONBLOCK;
	  fcntl(STDIN_FILENO, F_SETFL, flag);

}

void crear_listas_planificador(void)
{
	l_listos = list_create();
	l_bloqueados = list_create();
	l_terminados = list_create();
}

int recibir_mensaje_coordinador(int coord_socket)
{
	int read_size;
	char client_message[2000];



	//TODO Recibir mensaje de "Puedo bloquear esta clave desde este esi?" (punto 4.3)
	read_size = recv(coord_socket , client_message, 2000 , 0);

	//TODO Recibir mensaje de "Puedo des-bloquear esta clave desde este esi?" (Punto 5.1)


	read_size = recv(coord_socket , client_message , 2000 , 0);
	if(read_size > 0)
	{
		printf("Coordinador %d dice: %s\n",coord_socket,client_message);
		int res_send = send(coord_socket, client_message, sizeof(client_message), 0);
	}


	return read_size;

}

int cerrar_conexion_coord(int coord_socket)
{

	printf("Conexion con coordinador %d cerrada\n",coord_socket);
	close(coord_socket);

	return 0;
}

t_pcb_esi * crear_esi(t_conexion_esi conexion)
{
	t_pcb_esi * esi;

	esi = malloc(sizeof(t_pcb_esi));

	esi->pid = esi_pid;
	esi->conexion = conexion;
	esi->estado = ready;
	esi->estimacion = ESTIMACION_INICIAL;
	esi->estimacion_ant = ESTIMACION_INICIAL;
	esi->instruccion_actual = 0;
	esi_pid++;

	return esi;

}

int destruir_esi(t_pcb_esi * esi)
{
	esi->conexion.socket = NO_SOCKET;
	free(esi);
	return 0;

}
void mostrar_esi(t_pcb_esi * esi)
{

	printf("PID esi: %d\n", esi->pid);
	printf("Estado: %d\n", esi->estado);
	printf("\n");

	return;

}

void terminar_planificador(void)
{
	list_destroy_and_destroy_elements(l_listos,(void*)destruir_esi);
	list_destroy_and_destroy_elements(l_bloqueados,(void*)destruir_esi);
	list_destroy_and_destroy_elements(l_terminados,(void*)destruir_esi);

	if(l_ejecucion!=NULL)
		destruir_esi(l_ejecucion);
}

void planificar(void)
{

	//TODO Obtener el algoritmo de planificacion de la config
	printf("intento replanificar\n");
	if(l_ejecucion == NULL)
		obtener_proximo_ejec();

	else if(config.desalojo)
		desalojar_ejec();


}
void obtener_proximo_ejec(void)
{

	t_pcb_esi * ejec_ant;

	ejec_ant = l_ejecucion;
	printf("Intento obtener el siguiente\n");
	/*  TODO SJF debe copiar la lista de listos a una lista auxiliar,
	 * ordenarla por estimacion mas corta, tomar el primero, destruir la lista auxiliar.
	 * Eso para ambos casos
	 */

	if(!strcmp(config.algoritmo, "SJF-CD") )
	{

	}
	else if(!strcmp(config.algoritmo, "SJF-SD") )
	{

	}

	/* TODO HRRN: Similar al anterior, pero ordenar por ratio
	 * Revisar como es ese ordenamiento
	 */
	else if(!strcmp(config.algoritmo, "HRRN") )
	{

	}

	/* FIFO: Directamente saca el primer elemento de la lista y lo pone en ejecucion
	 * Por default tambien hace fifo... ya fue
	 */
	else if(!strcmp(config.algoritmo, "FIFO") )
	{
		l_ejecucion = list_remove(l_listos,0);
	}
	else
	{
		l_ejecucion = list_remove(l_listos,0);
	}

	//TODO Enviar confirmacion al esi
	// Punto 2
	//Si hubo un cambio en el esi en ejecucion, debo avisarle al nuevo esi en ejecucion que es su turno
	if(ejec_ant != l_ejecucion)
	{
		// TODO Enviar PID asignado al ESI
		printf("Aca le debo avisar al esi %d que es su turno\n",l_ejecucion->pid);
	}


	return;
}

void desalojar_ejec(void)
{

}
