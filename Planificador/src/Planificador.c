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
	int coord_socket = conectar_coordinador(IP_COORD, PORT_COORD);

	crear_listas_planificador();
	inicializar_conexiones_esi();
	stdin_no_bloqueante();

	//Inicializa semáforo
	sem_init(&sem_ejecucion_esi, 0, 1);

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
					terminar_planificador();
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

				consola_leer_stdin(read_buffer, MAX_LINEA);

				int res = consola_derivar_comando(read_buffer);
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

//*************************//
//FUNCIONES DE COMUNICACION//
//*************************//

int conectar_coordinador(char * ip, char * port) {

	int coord_socket = conectar_a_server(IP_COORD, PORT_COORD);
	if (coord_socket < 0)
	{
		printf("Error al intentar conectar al coordinador\n");
		exit(EXIT_FAILURE);
	}
	else
		printf("Conectado con el coordinador! (%d) \n",coord_socket);


	/* Handshake necesario para que el coordinador identifique que la
	 * conexion recibida fue del planificador. Solo se envia el header con la operacion
	 */
	t_content_header * header = crear_cabecera_mensaje(planificador,coordinador,OPERACION_HANDSHAKE_COORD,sizeof(int));

	int res_send = send(coord_socket, header, sizeof(t_content_header), 0);
	if(res_send < 0)
	{
		printf("Error send header \n");
	}

	destruir_cabecera_mensaje(header);

	return coord_socket;

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
			list_add(esi_listos, nuevo_esi);
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

int recibir_mensaje_coordinador(int coord_socket)
{
	int read_size;
	char client_message[2000];

	//Recepcion de mensaje comun de texto, con la cabecera (para no bloquear) Borrar mas adelante
	t_content_header *content_header = malloc(sizeof(t_content_header));

	read_size = recv(coord_socket, content_header, sizeof(t_content_header), 0);
	if(read_size < 0)
	{
		printf("Error recv header\n");
	}


	//TODO Recibir mensaje de "Puedo bloquear esta clave desde este esi?" (punto 4.3)
	if(content_header->operacion == OPERACION_BLOQUEO_COORD)
	{

		/* 4.1 El COORDINADOR le solicita al PLANIFICADOR aprobación para bloquear dicha
		 * clave [informándole Clave y PID del ESI]
		 * El PLANIFICADOR verifica internamente si la clave tiene un bloqueo vigente.
		 * SI existe un bloqueo, el PLANIFICADOR le indica al COORDINADOR que existe un
		 * bloqueo para que éste no avance con la ejecución de la sentencia.
		 * SI no existe un bloqueo, el PLANIFICADOR le indica al COORDINADOR que avance
		 * con la ejecución de la sentencia y registra la clave bloqueada en su lista.
		 */

		t_bloqueo_clave * bloqueo_clave = malloc(sizeof(t_bloqueo_clave));

		read_size = recv(coord_socket, bloqueo_clave, sizeof(t_bloqueo_clave),0);
		if(read_size < 0)
		{
			printf("Error recv coord 2 \n");
		}

	}
	//TODO Recibir mensaje de "Puedo des-bloquear esta clave desde este esi?" (Punto 5.1)
	else if (content_header->operacion == OPERACION_DESBLOQUEO_COORD)
	{
		/* Se debe sacar de la lista de bloqueados la clave recibida, y se debe pasar a ready
		 * el primer esi en lista de bloqueados que espere a la clave que se acaba
		 * de desbloquear
		 */

	}
	else
	{
		read_size = recv(coord_socket , client_message, content_header->cantidad_a_leer , 0);
		if(read_size > 0)
		{
			printf("Coordinador %d dice: %s\n",coord_socket,client_message);
			//int res_send = send(coord_socket, client_message, sizeof(client_message), (int)NULL);
		}
	}


	free(content_header);

	return read_size;

}

int recibir_mensaje_esi(int esi_socket)
{
	int read_size;
	char client_message[2000];
	t_pcb_esi *esi_aux;
	t_confirmacion_sentencia * confirmacion;

	t_content_header *content_header = malloc(sizeof(t_content_header));

	printf("Llego algo desde esi con fd %d! \n",esi_socket);

	read_size = recv(esi_socket, content_header, sizeof(t_content_header), (int)NULL);

	printf("Llego la operacion %d  debo leer %d bytes\n",content_header->operacion,content_header->cantidad_a_leer );

	if(content_header->operacion == OPERACION_RES_SENTENCIA){

		sem_post(&sem_ejecucion_esi);

		confirmacion = malloc(sizeof(t_confirmacion_sentencia));

		recv(esi_socket, confirmacion, content_header->cantidad_a_leer,(int) NULL);

		printf("Resultado de (%d) = %d\n",esi_socket,confirmacion->resultado);

		if(confirmacion->resultado == RESULTADO_ESI_OK_SIG){

			esi_en_ejecucion->instruccion_actual++;
			esi_en_ejecucion->ejec_anterior = 0;

			// Ordenar ejecutar siguiente sentencia del ESI
			enviar_confirmacion_sentencia(esi_en_ejecucion);

		}
		else if(confirmacion->resultado == RESULTADO_ESI_OK_FINAL){
			esi_en_ejecucion->estado = terminado;
			esi_en_ejecucion->instruccion_actual++;

			esi_aux = esi_en_ejecucion;

			list_add(esi_terminados, esi_aux);

			esi_en_ejecucion = NULL;
			planificar();
		}
		else if(confirmacion->resultado == RESULTADO_ESI_BLOQUEADA){
			esi_en_ejecucion->estado = bloqueado;
			esi_en_ejecucion->ejec_anterior = 1;
			esi_aux = esi_en_ejecucion;

			list_add(esi_bloqueados, esi_aux);

			esi_en_ejecucion = NULL;
			planificar();
		}

		free(confirmacion);
	}

	else if(content_header->operacion == 10){

		// TODO Eliminar el bloque de operacion de prueba
		read_size = recv(esi_socket , client_message , content_header->cantidad_a_leer, 0);
		if(read_size > 0)
		{
			printf("Esi %d dice: %s\n",esi_socket,client_message);
			//int res_send = send(esi_socket, client_message, sizeof(client_message), 0);
		}

	}

	destruir_cabecera_mensaje(content_header);
	//free(content_header);

	return read_size;
}

int cerrar_conexion_coord(int coord_socket)
{

	printf("Conexion con coordinador %d cerrada\n",coord_socket);
	close(coord_socket);

	return 0;
}

int cerrar_conexion_esi(t_conexion_esi * esi)
{

	printf("Conexion con esi %d cerrada\n",esi->socket);
	close(esi->socket);
	esi->socket = NO_SOCKET;

	return 0;
}

int iniciar_servidor(char * port)
{
	int server_socket = crear_listen_socket(port,MAX_CLIENTES);


	if(server_socket < 0)
	{
		printf("Falló la creacion del socket servidor\n");
		exit(1);
	}
	else
	{
		printf("Socket servidor (%d) escuchando\n", server_socket);
	}

	return server_socket;

}

//***********************//
//FUNCIONES DE LA CONSOLA//
//***********************//

/*	Consola preparada para funcionar en un hilo aparte
 * 	actualmente no se está usando ya que readline es una funcion bloqueante
 * 	y necesito que el select pueda leer de stdin mientras maneja las conexiones
 * 	Es código duplicado, pero la dejo porque está bonita :)
 */

void *consola() {

	int res = 0;
	char *buffer = NULL;

	printf("\nAbriendo consola...\n");

	while(TRUE){

		//Trae la linea de consola
		buffer = readline(">");

		res = consola_derivar_comando(buffer);

		free(buffer);

		//Sale de la consola con exit
		if(res)
			break;
	}

	//pthread_exit(0);
	return 0;
}

int consola_derivar_comando(char * buffer){

	int comando_key;
	char *comando = NULL;
	char *parametro1 = NULL;
	char *parametro2 = NULL;
	int res = 0;

	//printf("string recibido: %s\n",buffer);

	// Separa la linea de consola en comando y sus parametros
	consola_obtener_parametros(buffer, &comando, &parametro1, &parametro2);
	//printf("comando: %s p1: %s p2: %s\n",comando,parametro1,parametro2);

	// Obtiene la clave del comando a ejecutar para el switch
	comando_key = consola_obtener_key_comando(comando);

	switch(comando_key){
		case pausar:
			consola_pausar();
			break;
		case continuar:
			consola_continuar();
			break;
		case bloquear:
			consola_bloquear_clave(parametro1,parametro2);
			break;
		case desbloquear:
			consola_desbloquear_clave(parametro1);
			break;
		case listar:
			consola_listar_recurso(parametro1);
			break;
		case kill:
			consola_matar_proceso(parametro1);
			break;
		case status:
			consola_consultar_status_clave(parametro1);
			break;
		case deadlock:
			consola_consultar_deadlock();
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
		case bloqueos:
			mostrar_bloqueos();
			break;
		default:
			printf("No reconozco el comando...\n");
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

int consola_obtener_key_comando(char* comando)
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

	if(!strcmp(comando, "bloqueos"))
				key = bloqueos;

	if(!strcmp(comando, "exit"))
		key = salir;

	return key;
}

void consola_obtener_parametros(char* buffer, char** comando, char** parametro1, char** parametro2)
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

int consola_leer_stdin(char *read_buffer, size_t max_len)
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

void consola_pausar(void)
{
	/* Pausar/Continuar planificación(^2): El Planificador no le dará nuevas órdenes de
	 * ejecución a ningún ESI mientras se encuentre pausado.
	 *
	 * ^2: Esto se puede lograr ejecutando una sycall bloqueante que espere la entrada de un humano.
	 */

	printf("Estas intentando pausar...\n");
	return;
}

void consola_continuar(void)
{
	printf("Estas intentando continuar...\n");
	return;
}

void consola_bloquear_clave(char* clave , char* id){

	/* Se bloqueará el proceso ESI hasta ser desbloqueado (ver más adelante),
	 * especificado por dicho ID(^3) en la cola del recurso clave. Vale recordar
	 * que cada línea del script a ejecutar es atómica, y no podrá ser interrumpida;
	 * si no que se bloqueará en la próxima oportunidad posible. Solo se podrán bloquear
	 * de esta manera ESIs que estén en el estado de listo o ejecutando.
	 *
	 * ^3: El Planificador empezará con una serie de claves bloqueadas de esta manera.
	 *
	 * Entiendo que este comando debe pasar a bloqueado un esi, e indicar por cual clave llego
	 * a bloqueados en su pcb, o lista (como hagamos para listar recurso)
	 */

	if(clave == NULL || id == NULL){
		printf("Parametros incorrectos (bloquear <clave> <id>)\n");
	}
	else{
		printf("Bloquear clave: %s id: %s\n",clave, id);
		bloquear_clave(clave, id);
	}


	return;
}

void consola_desbloquear_clave(char* clave){

	/* desbloquear clave: Se desbloqueara el primer proceso ESI bloquedo por la clave
	 * especificada.
	 *
	 */

	if(clave == NULL)
		printf("Parametros incorrectos (desbloquear <clave>)\n");
	else
	{
		printf("Desbloquear clave: %s \n",clave);
		desbloquear_clave(clave);
	}


	return;
}

void consola_listar_recurso(char* recurso)
{
	/* listar recurso: Lista los procesos encolados esperando al recurso.
	 *
	 * 	Entiendo que un recurso es una clave, osea que deberiamos tener un control en alguna
	 * 	lista, cuando un esi intenta acceder a una clave bloqueada, ponerlo en esa lista
	 * 	para mostrarlos con este comando. O sino, agregar en el pcb, si tiene status blocked
	 * 	agregar un campo clave_block con la clave que lo llevó a estar bloqueado
	 *
	 */

	if(recurso == NULL)
		printf("Parametros incorrectos (listar <recurso>)\n");
	else
		printf("Listar recurso: %s\n",recurso);

	return;
}

void consola_matar_proceso(char* id)
{
	/* kill ID: finaliza el proceso. Recordando la atomicidad mencionada en “bloquear”.
	 * Al momento de eliminar el ESI, se debloquearan las claves que tenga tomadas.
	 *
	 */

	if(id == NULL)
		printf("Parametros incorrectos (kill <id>)\n");
	else
		printf("KILL ID: %s\n",id);

	return;
}

void consola_consultar_status_clave(char* clave)
{

	/* status clave: Con el objetivo de conocer el estado de una clave y de probar la
	 * correcta distribución de las mismas se deberan obtener los siguientes valores:
	 * (Este comando se utilizara para probar el sistema)

    -Valor, en caso de no poseer valor un mensaje que lo indique.
    -Instancia actual en la cual se encuentra la clave. (En caso de que la clave no exista,
    	la Instancia actual debería )
    -Instancia en la cual se guardaría actualmente la clave (Calcular este valor mediante
    	el algoritmo de distribución(^4), pero sin afectar la distribución actual de las claves).
    -ESIs bloqueados a la espera de dicha clave.

	 *
	 * ^4: Estos algoritmos se detallarán más adelante.
	 */

	if(clave == NULL)
		printf("Parametros incorrectos (status <clave>)\n");
	else
		printf("Status clave: %s \n",clave);

	return;
}

void consola_consultar_deadlock(void)
{
	/* deadlock: Esta consola también permitirá analizar los deadlocks que existan en el
	 * sistema y a que ESI están asociados.
	 * Pudiendo resolverlos manualmente con la sentencia de kill previamente descrita.
	 */

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
			list_add_all(lista, esi_listos);
		}

		else if(!strcmp(name, "block"))
		{
			list_add_all(lista,esi_bloqueados);
		}

		else if(!strcmp(name, "term"))
		{
			list_add_all(lista, esi_terminados);
		}
		else
		{
			printf("No existe la lista %s: \n",name);
			list_destroy(lista);
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

	if(esi_en_ejecucion!=NULL)
		printf("\nPID Esi en ejecución actual: %d: \n",esi_en_ejecucion->pid);
	else
		printf("No hay ningun esi en ejecucion\n");

	return;
}

void mostrar_bloqueos(void)
{

	t_list * lista;

	lista = list_create();
	list_add_all(lista,claves_bloqueadas);

	printf("\nEstado actual de la lista de claves bloqueadas: \n\n");

	list_iterate(lista,(void*)mostrar_clave_bloqueada);

	printf("\nTamaño: %d \n",list_size(lista));

	list_destroy(lista);
	printf("\n");

}

void mostrar_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada)
{
	printf("PID esi: %d\n", clave_bloqueada->pid);
	printf("Clave Bloqueada: %s\n", clave_bloqueada->clave);
	printf("\n");

	return;
}

void stdin_no_bloqueante(void)
{
	  /* Set nonblock for stdin. */
	  int flag = fcntl(STDIN_FILENO, F_GETFL, 0);
	  flag |= O_NONBLOCK;
	  fcntl(STDIN_FILENO, F_SETFL, flag);

}

void planificar(void)
{
	//TODO Obtener el algoritmo de planificacion de la config
	printf("intento replanificar\n\n");
	if(esi_en_ejecucion == NULL){
		obtener_proximo_ejecucion();
	}
	/*else if(config.desalojo){
		desalojar_ejecucion();
	}*/
}

void obtener_proximo_ejecucion(void)
{
	t_pcb_esi * ejec_ant;

	ejec_ant = esi_en_ejecucion;
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
		esi_en_ejecucion = list_remove(esi_listos,0);
	}
	else
	{
		esi_en_ejecucion = list_remove(esi_listos,0);
	}

	// Punto 2
	//Si hubo un cambio en el esi en ejecucion, debo avisarle al nuevo esi en ejecucion que es su turno
	if(ejec_ant != esi_en_ejecucion)
	{

		//printf("Aca le debo avisar al esi %d que es su turno\n", esi_en_ejecucion->pid);
		int res = enviar_confirmacion_sentencia(esi_en_ejecucion);
		if(!res)
		{
			//Validar resultado del envio
		}

	}

	return;
}

int enviar_confirmacion_sentencia(t_pcb_esi * pcb_esi)
{

	sem_wait(&sem_ejecucion_esi);

	t_content_header * header = crear_cabecera_mensaje(planificador,esi,OPERACION_CONF_SENTENCIA,sizeof(t_confirmacion_sentencia));

	t_confirmacion_sentencia * conf = NULL;

	conf = malloc(sizeof(t_confirmacion_sentencia));

	conf->pid 			= pcb_esi->pid;
	conf->ejec_anterior = pcb_esi->ejec_anterior;
	conf->resultado		= -1;

	printf("Aviso al esi %d que es su turno\n\n",pcb_esi->pid);

	int res_send = send(pcb_esi->conexion.socket, header, sizeof(t_content_header), 0);
	if(res_send < 0)
	{
		printf("Error send header \n");
	}

	res_send = send(pcb_esi->conexion.socket, conf, sizeof(t_confirmacion_sentencia), 0);
	if(res_send < 0)
	{
		printf("Error send ejec \n");
	}

	free(conf);
	destruir_cabecera_mensaje(header);
	return res_send;

}

void crear_listas_planificador(void)
{
	esi_listos = list_create();
	esi_bloqueados = list_create();
	esi_terminados = list_create();
	claves_bloqueadas = list_create();
}

// TODO Implementar
void desalojar_ejecucion(void){

	return;
}

void terminar_planificador(void)
{
	list_destroy_and_destroy_elements(esi_listos,(void*)destruir_esi);
	list_destroy_and_destroy_elements(esi_bloqueados,(void*)destruir_esi);
	list_destroy_and_destroy_elements(esi_terminados,(void*)destruir_esi);
	list_destroy_and_destroy_elements(claves_bloqueadas,(void*)destruir_clave_bloqueada);

	if(esi_en_ejecucion!=NULL)
		destruir_esi(esi_en_ejecucion);
}

//***********************//
//FUNCIONES DE ESI		 //
//***********************//

t_pcb_esi * crear_esi(t_conexion_esi conexion)
{
	t_pcb_esi * esi;

	esi = malloc(sizeof(t_pcb_esi));

	esi->pid = esi_seq_pid;
	esi->conexion = conexion;
	esi->estado = listo;
	esi->estimacion = ESTIMACION_INICIAL;
	esi->estimacion_ant = ESTIMACION_INICIAL;
	esi->instruccion_actual = 0;
	esi->ejec_anterior = 0;
	esi->clave_bloqueo = NULL;
	esi_seq_pid++;

	return esi;
}

void mostrar_esi(t_pcb_esi * esi)
{
	printf("PID esi: %d\n", esi->pid);
	printf("Estado: %d\n", esi->estado);

	if(esi->clave_bloqueo!=NULL)
		printf("Clave que lo bloqueó: %s\n", esi->clave_bloqueo);

	printf("\n");

	return;
}

int destruir_esi(t_pcb_esi * esi)
{
	esi->conexion.socket = NO_SOCKET;
	if(esi->clave_bloqueo!=NULL)
	{
		free(esi->clave_bloqueo);
	}
	free(esi);
	return 0;
}

t_pcb_esi * buscar_esi_en_lista_pid(t_list* lista,int pid)
{
	bool is_pid_esi(t_pcb_esi * esi)
	{
		return (esi->pid==pid);
	}

	return (list_find(lista,(void*)is_pid_esi));
}

t_pcb_esi * sacar_esi_de_lista_pid(t_list* lista,int pid)
{
	bool is_pid_esi(t_pcb_esi * esi)
	{
		return (esi->pid==pid);
	}

	return (list_remove_by_condition(lista,(void*)is_pid_esi));
}

t_pcb_esi * buscar_esi_bloqueado_por_clave(char* clave)
{
	bool is_esi_bloqueado(t_pcb_esi * esi)
	{
		return ((!strcmp(clave, esi->clave_bloqueo)));
	}

	return(list_find(esi_bloqueados,(void*)is_esi_bloqueado));

}

t_pcb_esi * sacar_esi_bloqueado_por_clave(char* clave)
{
	bool is_esi_bloqueado(t_pcb_esi * esi)
	{
		return ((!strcmp(clave, esi->clave_bloqueo)));
	}

	return(list_remove_by_condition(esi_bloqueados,(void*)is_esi_bloqueado));

}

//*******************************//
//FUNCIONES DE CLAVES BLOQUEADAS //
//*******************************//

int bloquear_clave(char* clave , char* id)
{

	int pid = -1;
	t_claves_bloqueadas * clave_bloqueada;

	pid = atoi(id);
	if(buscar_clave_bloqueada(clave) == NULL)
	{
		clave_bloqueada = malloc(sizeof(t_claves_bloqueadas));

		memset(clave_bloqueada, 0, sizeof(t_claves_bloqueadas));

		clave_bloqueada->clave = strdup(clave);
		clave_bloqueada->pid = pid;

		list_add(claves_bloqueadas, clave_bloqueada);

		printf("Se creó la clave bloqueada %s\n",clave);
	}
	else
	{
		printf("La clave %s ya está bloqueada! No se agrega a la lista\n",clave);

	}

	if(esi_en_ejecucion != NULL && esi_en_ejecucion->pid == pid)
	{
		/*
		 * TODO: Si el esi esta en ejecucion, esperar a que termine la instrucción y desalojar
		 * Seguramente hay que usar un semaforo
		 *
		 */

		sem_wait(&sem_ejecucion_esi);

		esi_en_ejecucion->clave_bloqueo = strdup(clave);
		esi_en_ejecucion->estado = bloqueado;

		list_add(esi_bloqueados,esi_en_ejecucion);

		esi_en_ejecucion = NULL;

		sem_post(&sem_ejecucion_esi);

		printf("El ESI %d estaba en ejecución, se pasó a bloqueados\n",pid);
	}
	else if (buscar_esi_en_lista_pid(esi_listos, pid))
	{
		// Si el esi está en la lista de listos, hay que pasarlo a bloqueado
		t_pcb_esi * esi_aux = sacar_esi_de_lista_pid(esi_listos,pid);

		esi_aux->clave_bloqueo = strdup(clave);
		esi_aux->estado = bloqueado;

		list_add(esi_bloqueados,esi_aux);

		printf("El ESI %d estaba en listos, se pasó a bloqueados\n",pid);

	}
	else
	{
		//Si el esi no existe, solo se crea la clave bloqueada
		if(clave_bloqueada!=NULL)
			clave_bloqueada->pid = -1;

		printf("No existe el ESI de ID %d, solo se crea la clave bloqueada\n",pid);
	}

	return 0;
}


int desbloquear_clave(char* clave)
{

	bool is_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada)
	{
		return ((!strcmp(clave, clave_bloqueada->clave)));
	}

	t_claves_bloqueadas * clave_bloqueada = buscar_clave_bloqueada(clave);

	if(clave_bloqueada == NULL)
	{
		printf("La clave %s no está bloqueada\n",clave);
		return 1;
	}

	//Busco el primer esi bloqueado por la clave a desbloquear
	t_pcb_esi * esi_a_desbloquear = buscar_esi_bloqueado_por_clave(clave);
	if(esi_a_desbloquear!=NULL)
	{

		//Si existe, lo remuevo de los bloqueados
		esi_a_desbloquear = sacar_esi_bloqueado_por_clave(clave);

		printf("El esi %d estaba bloqueado por la clave %s, se pasa a ready\n",esi_a_desbloquear->pid,clave);

		free(esi_a_desbloquear->clave_bloqueo);
		esi_a_desbloquear->clave_bloqueo = NULL;
		esi_a_desbloquear->estado = listo;

		//Lo agrego a Ready
		list_add(esi_listos,esi_a_desbloquear);

		//Busco si hay otro esi bloqueado por la misma clave
		//Si no existe otro esi bloqueado por esa clave, hay que eliminar la clave de la lista de claves bloqueadas
		if(buscar_esi_bloqueado_por_clave(clave)==NULL)
		{
			printf("Entro a borrar la clave bloqueada\n");
			list_remove_and_destroy_by_condition(claves_bloqueadas,(void*)is_clave_bloqueada,(void*)destruir_clave_bloqueada);
		}


	}
	else
	{
		//Si no hay esi bloqueado por esa clave, solamente lo elimino de las claves bloqueadas
		list_remove_and_destroy_by_condition(claves_bloqueadas,(void*)is_clave_bloqueada,(void*)destruir_clave_bloqueada);
	}


	return 0;
}

t_claves_bloqueadas * buscar_clave_bloqueada(char* clave)
{

	bool is_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada)
	{
		return ((!strcmp(clave, clave_bloqueada->clave)));
	}

	 return (list_find(claves_bloqueadas,(void*)is_clave_bloqueada));

}

int destruir_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada)
{
	free(clave_bloqueada->clave);
	free(clave_bloqueada);
	return 0;

}
