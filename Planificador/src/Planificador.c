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


int main(int argc, char **argv) {

	fd_set readset, writeset, exepset;
	int max_fd;
	char read_buffer[MAX_LINEA];
	struct timeval tv = {0, 500};

	//Parametro booleano indica si muestra por pantalla el log o no.
	logger = log_create("Log_Planificador.txt", "Planificador", false, LOG_LEVEL_TRACE);
	logger_planificador(escribir_loguear, l_trace,"Iniciando Planificador...");


	configurar_signals();
	crear_listas_planificador();
	leer_configuracion_desde_archivo(argv[1]);
	inicializar_conexiones_esi();
	stdin_no_bloqueante();

	//TODO: Borrar al terminar TODAS las pruebas
	//dummy_genera_deadlock();

	//Crea el socket servidor para recibir ESIs (ya bindeado y escuchando)
	int serv_socket = iniciar_servidor(config.puerto_escucha);

	//Crea el socket cliente para conectarse al coordinador
	int coord_socket = conectar_coordinador(config.ip_coordinador, config.puerto_coordinador,handshake);

	//Crea el socket para atender el comando status
	coord_status_socket = conectar_coordinador(config.ip_coordinador,PUERTO_STATUS,no_handshake);

	while(TRUE){
		//Inicializa los file descriptor
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_ZERO(&exepset);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		//Agrega el fd del socket servidor al set de lectura y excepciones
		FD_SET(serv_socket, &readset);
		FD_SET(serv_socket, &exepset);

		//Agrega el fd del socket coordinador al set de lectura
		FD_SET(coord_socket, &readset);
		FD_SET(coord_socket, &exepset);

		//Agrega el fd del socket cmd status
		FD_SET(coord_status_socket, &readset);
		FD_SET(coord_status_socket, &exepset);

		//Agrega el stdin para leer la consola
		FD_SET(STDIN_FILENO, &readset);

		/* Seteo el maximo file descriptor necesario para el select */
		max_fd = serv_socket;

		//Agrega los conexiones esi existentes
		for (int i = 0; i < MAX_CLIENTES; i++)
		{
			if (conexiones_esi[i].socket != NO_SOCKET)
			{
				FD_SET(conexiones_esi[i].socket, &readset);
				FD_SET(conexiones_esi[i].socket, &exepset);
			}

			if (conexiones_esi[i].socket > max_fd)
				max_fd = conexiones_esi[i].socket;

		}

		if(max_fd < coord_socket){
			max_fd = coord_socket;
		}


		if(max_fd < coord_status_socket){
			max_fd = coord_status_socket;
		}

		int result = select(max_fd+1, &readset, &writeset, &exepset, &tv);
		logger_planificador(loguear, l_debug,"Resultado del select: %d\n",result); //Revisar rendimiento del CPU cuando select da > 1

		//if(result == 0)
		//	log_info(logger,"Select time out\n");
		//else
		if(result < 0 ) {
			logger_planificador(escribir_loguear, l_error,"Error en select\n");
			break;
		}
		else if(errno == EINTR) {
			logger_planificador(escribir_loguear, l_error,"Me mataron! salgo del select\n");
			break;
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
					break;
				}
			}


			if(FD_ISSET(coord_socket, &exepset))
			{
				if(recibir_mensaje_coordinador(coord_socket) == 0)
				{
					cerrar_conexion_coord(coord_socket);
					terminar_planificador();
					break;
				}
			}

			//Comando status
			if(FD_ISSET(coord_status_socket, &readset))
			{
				if(recibir_mensaje_coordinador(coord_status_socket) == 0)
				{
					cerrar_conexion_coord(coord_status_socket);
					terminar_planificador();
					break;
				}
			}


			if(FD_ISSET(coord_status_socket, &exepset))
			{
				if(recibir_mensaje_coordinador(coord_status_socket) == 0)
				{
					cerrar_conexion_coord(coord_status_socket);
					terminar_planificador();
					break;
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
						if(recibir_mensaje_esi(conexiones_esi[i]) == 0)
						{
							finalizar_esi(conexiones_esi[i].pid,liberar);
							planificar();
							continue;
						}
					}

					//Excepciones del esi, para la desconexion
					if (FD_ISSET(conexiones_esi[i].socket, &exepset)) {
						if(recibir_mensaje_esi(conexiones_esi[i]) == 0)
						{
							finalizar_esi(conexiones_esi[i].pid,liberar);
							planificar();
							continue;
						}
					}//if isset
				} // if NO_SOCKET
			} //for conexiones_esi
		} //if result select
	} //while

	//pthread_exit(0);
	return EXIT_SUCCESS;
}

//*************************//
//FUNCIONES DE COMUNICACION//
//*************************//

int conectar_coordinador(char * ip, char * port, int handshake) {

	int coord_socket = conectar_a_server(ip, port);
	if (coord_socket < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error al intentar conectar al coordinador\n");
		terminar_planificador();
		exit(EXIT_FAILURE);
	}
	else{
		logger_planificador(escribir_loguear,l_trace,"Conectado con el coordinador! (%d)",coord_socket);
	}

	if(handshake){
		/* Handshake necesario para que el coordinador identifique que la
		 * conexion recibida fue del planificador. Solo se envia el header con la operacion
		 */
		t_content_header * header = crear_cabecera_mensaje(planificador,coordinador,OPERACION_HANDSHAKE_COORD,sizeof(int));

		int res_send = send(coord_socket, header, sizeof(t_content_header), 0);
		if(res_send < 0)
		{
			logger_planificador(escribir_loguear,l_error,"Error send header handshake con el Coordinador :( \n");
			terminar_planificador();
			exit(EXIT_FAILURE);
		}
		else{
			logger_planificador(escribir_loguear,l_trace,"Handshake con Coordinador enviado correctamente");
		}

		destruir_cabecera_mensaje(header);
	}

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
		logger_planificador(escribir_loguear,l_error,"Error al aceptar un nuevo ESI :(\n");
	  return -1;
	}

	logger_planificador(escribir_loguear,l_trace,"\nSe aceptó un nuevo ESI, conexión (%d)", new_client_sock);

	//Lo agrego a la lista de conexiones esi actuales
	for (int i = 0; i < MAX_CLIENTES; ++i) {

		if (conexiones_esi[i].socket == NO_SOCKET) {
			conexiones_esi[i].socket = new_client_sock;
			conexiones_esi[i].addres = client_addr;

			//Creo el nuevo esi con su conexion
			t_pcb_esi * nuevo_esi = crear_esi(&conexiones_esi[i]);

			//Agrego el esi nuevo a la cola de listos
			list_add(esi_listos, nuevo_esi);
			logger_planificador(escribir_loguear,l_info,"Al nuevo ESI se le asignó el PID %d y fue agregado a ready!\n",nuevo_esi->pid);

			/*
			 * Si el algoritmo es con desalojo, (solo SJF) debo chequear si la estimacion del esi nuevo es
			 * menor a la rafaga pendiente del que esta en ejecucion, si lo es reemplazarlo por el nuevo.
			 */
			if(!strcmp(config.algoritmo, "SJF-CD"))
			{
				if((esi_en_ejecucion!=NULL) && (nuevo_esi->estimacion_real < esi_en_ejecucion->estimacion_actual))
				{
					logger_planificador(escribir_loguear,l_info,"El ESI nuevo de PID %d debe desalojar al ESI en ejecucion!",nuevo_esi->pid);
					desalojo_en_ejecucion++;
					esi_por_desalojar = nuevo_esi;
					logger_planificador(escribir_loguear,l_info,"Esperando a que ESI %d termine su sentencia\n",esi_en_ejecucion->pid);
				}
			}

	        return 0;
	    }
	 }

	logger_planificador(escribir_loguear,l_error,"Demasiadas conexiones. Cerrando nueva conexion\n");
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
	int resultado_consulta;

	t_claves_bloqueadas * clave_bloqueada=NULL;

	//Recepcion de mensaje comun de texto, con la cabecera (para no bloquear) Borrar mas adelante
	t_content_header *content_header = malloc(sizeof(t_content_header));

	logger_planificador(loguear,l_debug,"Llegó un nuevo mensaje desde el Coordinador!");

	read_size = recv(coord_socket, content_header, sizeof(t_content_header), 0);
	if(read_size < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error al recibir la cabecera de un mensaje con el Coordinador :(");
		destruir_cabecera_mensaje(content_header);
		terminar_planificador();
		exit(EXIT_FAILURE);
	}

	if(content_header->operacion == OPERACION_CONSULTA_CLAVE_COORD)
	{

		/* 4.1 El COORDINADOR le solicita al PLANIFICADOR aprobación para bloquear dicha
		 * clave [informándole Clave y PID del ESI]
		 * El PLANIFICADOR verifica internamente si la clave tiene un bloqueo vigente.
		 * SI existe un bloqueo, el PLANIFICADOR le indica al COORDINADOR que existe un
		 * bloqueo para que éste no avance con la ejecución de la sentencia.
		 * SI no existe un bloqueo, el PLANIFICADOR le indica al COORDINADOR que avance
		 * con la ejecución de la sentencia y registra la clave bloqueada en su lista.
		 */

		t_consulta_bloqueo * consulta_bloqueo = malloc(sizeof(t_consulta_bloqueo));

		read_size = recv(coord_socket, consulta_bloqueo , sizeof(t_consulta_bloqueo),0);
		if(read_size < 0)
		{
			logger_planificador(escribir_loguear,l_error,"Error al recibir consulta de bloqueo desde el Coordinador");
			free(consulta_bloqueo);
			terminar_planificador();
			exit(EXIT_FAILURE);
		}

		logger_planificador(escribir_loguear,l_info,"Consulta %s %s, para el ESI %d",sentencia_string(consulta_bloqueo->sentencia),consulta_bloqueo->clave,consulta_bloqueo->pid);

		switch(consulta_bloqueo->sentencia)
		{
			case GET:

				clave_bloqueada = NULL;
				clave_bloqueada = buscar_clave_bloqueada(consulta_bloqueo->clave);
				//Si la clave sesta bloqueada, el esi pasa a bloqueado
				if(clave_bloqueada){
					resultado_consulta = CLAVE_BLOQUEADA;
				}
				else{
					resultado_consulta = CORRECTO;
					bloqueo_por_get++;
				}


				clave_a_bloquear_por_get = malloc(sizeof(t_consulta_bloqueo));
				strcpy(clave_a_bloquear_por_get->clave,consulta_bloqueo->clave);
				clave_a_bloquear_por_get->pid = consulta_bloqueo->pid;

				break;

			case SET:

				/*  Si se hace set a una clave que no estaba bloqueada por ese esi, debe abortar */
				clave_bloqueada = NULL;
				clave_bloqueada = buscar_clave_bloqueada(consulta_bloqueo->clave);

				if(clave_bloqueada != NULL)
				{
					if(clave_bloqueada->pid == consulta_bloqueo->pid)
					{
						resultado_consulta = CORRECTO;
					}
					else
						resultado_consulta = ABORTAR;

				}
				else
					resultado_consulta = ABORTAR;

				break;

			case STORE:
				/*  Si se hace STORE a una clave que no estaba bloqueada por ese esi, debe abortar */
				clave_bloqueada = buscar_clave_bloqueada(consulta_bloqueo->clave);

				if(clave_bloqueada != NULL)
				{
					if(clave_bloqueada->pid == consulta_bloqueo->pid)
					{
						resultado_consulta = CORRECTO;
						desbloqueo_por_store++;
						clave_a_desbloquear_por_store = malloc(sizeof(t_consulta_bloqueo));
						strcpy(clave_a_desbloquear_por_store->clave,consulta_bloqueo->clave);
						clave_a_desbloquear_por_store->pid = consulta_bloqueo->pid;
						clave_a_desbloquear_por_store->sentencia = consulta_bloqueo->sentencia;
					}

					else
						resultado_consulta = ABORTAR;

				}
				else
					resultado_consulta = ABORTAR;

				break;
		}

		enviar_coordinador_resultado_consulta(coord_socket, resultado_consulta);

		free(consulta_bloqueo);
	}

	// RECIBE STATUS DE CLAVE
	if(content_header->operacion == PLANIFICADOR_COORDINADOR_CMD_STATUS){

		t_status_clave *st_clave = malloc(sizeof(t_status_clave));

		read_size = recv(coord_socket, st_clave, sizeof(t_status_clave), 0);
		if(read_size < 0)
		{
			logger_planificador(escribir_loguear, l_error, "Error al recibir el status de la clave desde el Coordinador");

			//TODO Evaluar qué hacer
			//terminar_planificador();
			//exit(EXIT_FAILURE);
		}
		else{
			status_clave(st_clave,clave_status);
			free(clave_status);
		}

		free(st_clave);
	}

	destruir_cabecera_mensaje(content_header);

	return read_size;
}

int recibir_mensaje_esi(t_conexion_esi conexion_esi)
{
	int read_size;
	t_pcb_esi *esi_aux=NULL;
	t_confirmacion_sentencia * confirmacion=NULL;

	t_content_header *content_header = malloc(sizeof(t_content_header));
	memset(content_header, 0, sizeof(t_content_header));

	logger_planificador(loguear,l_debug,"Llegó un nuevo mensaje desde el ESI %d!",conexion_esi.pid);

	read_size = recv(conexion_esi.socket, content_header, sizeof(t_content_header), 0);
	logger_planificador(loguear,l_warning,"resultado recv %d! errno %d",read_size,errno);
	if(read_size < 0)
	{
		free(content_header);
		logger_planificador(escribir_loguear,l_error,"Hubo un error en recv con esi %d",conexion_esi.pid);
		finalizar_esi(conexion_esi.pid,no_liberar);
		return read_size;
	}


	logger_planificador(loguear,l_debug,"Llego la operacion %d  se leerán %d bytes",content_header->operacion,content_header->cantidad_a_leer);

	if(content_header->operacion == OPERACION_RES_SENTENCIA)
	{
		confirmacion = malloc(content_header->cantidad_a_leer);
		memset(confirmacion, 0, content_header->cantidad_a_leer);

		read_size = recv(conexion_esi.socket, confirmacion, content_header->cantidad_a_leer,0);
		logger_planificador(loguear,l_warning,"resultado recv %d! errno %d",read_size,errno);
		if(read_size < 0)
		{
			free(confirmacion);
			logger_planificador(escribir_loguear,l_error,"Hubo un error en recv con esi %d",conexion_esi.pid);
			finalizar_esi(conexion_esi.pid,no_liberar);
			return read_size;
		}

		logger_planificador(loguear,l_trace,"Resultado de (%d) = %d",conexion_esi.pid,confirmacion->resultado);

		if(confirmacion->resultado == CORRECTO){

			esi_en_ejecucion->instruccion_actual++;
			esi_en_ejecucion->estimacion_actual--;
			esi_en_ejecucion->ejec_anterior = 0;

			logger_planificador(escribir_loguear,l_info,"La sentencia se ejecutó correctamente");

			//Si hay un bloqueo de clave pendiente para este esi en ejecucion, lo hago
			if(bloqueo_en_ejecucion)
			{
				confirmar_bloqueo_ejecucion();
			}

			if(desalojo_en_ejecucion)
			{
				confirmar_desalojo_ejecucion();
			}

			if(bloqueo_por_get)
			{
				confirmar_bloqueo_por_get();
			}

			if(desbloqueo_por_store)
			{
				confirmar_desbloqueo_por_store();
			}

			if(estado_pausa_por_consola == pausado){
				confirmar_pausa_por_consola();
			}

			if(kill_en_ejecucion){
				confirmar_kill_ejecucion();
			}

			// Ordenar ejecutar siguiente sentencia del ESI
			if(esi_en_ejecucion!=NULL)
			{
				enviar_esi_confirmacion_sentencia(esi_en_ejecucion);
			}

		}
		else if(confirmacion->resultado == LISTO){

			esi_aux = esi_en_ejecucion;
			finalizar_esi(esi_aux->pid,liberar);

		}
		else if(confirmacion->resultado == CLAVE_BLOQUEADA){

			if(confirmacion->pid == clave_a_bloquear_por_get->pid)
			{
				logger_planificador(loguear,l_warning,"Bloquea el esi por aca");

				logger_planificador(escribir_loguear,l_info,"El ESI %d intentó acceder a la clave %s, se pasa a bloqueados",confirmacion->pid,clave_a_bloquear_por_get->clave);
				esi_en_ejecucion->estado = bloqueado;
				esi_en_ejecucion->ejec_anterior = 1;
				esi_en_ejecucion->clave_bloqueo = strdup(clave_a_bloquear_por_get->clave);
				esi_aux = esi_en_ejecucion;

				list_add(esi_bloqueados, esi_aux);

				free(clave_a_bloquear_por_get);
				esi_en_ejecucion = NULL;


			}

		}
		else if(confirmacion->resultado == ABORTAR){

			esi_aux = esi_en_ejecucion;
			logger_planificador(escribir_loguear,l_info,"El ESI %d se abortará por orden el Coordinador",confirmacion->pid);
			finalizar_esi(esi_aux->pid,no_liberar);
		}

		free(confirmacion);

		//Cada vez que se ejecuta una sentencia, debo aumentar el tiempo de espera de todos los esi READY
		aumentar_tiempo_espera_ready();

	}

	printf("\n");

	destruir_cabecera_mensaje(content_header);

	planificar();

	return read_size;
}

int cerrar_conexion_coord(int coord_socket)
{
	logger_planificador(escribir_loguear,l_trace,"\nConexión con coordinador (%d) cerrada",coord_socket);
	close(coord_socket);

	return 0;
}

int cerrar_conexion_esi(t_conexion_esi * esi)
{
	logger_planificador(loguear,l_trace,"Conexión con ESI %d cerrada\n",esi->pid);
	close(esi->socket);
	esi->socket = NO_SOCKET;

	return 0;
}

int iniciar_servidor(char * port)
{
	int server_socket = crear_listen_socket(port,MAX_CLIENTES);

	if(server_socket < 0)
	{
		logger_planificador(escribir_loguear,l_error,"\nFalló la creación del socket servidor");
		terminar_planificador();
		exit(1);
	}
	else
	{
		logger_planificador(escribir_loguear,l_trace,"Socket servidor (%d) escuchando", server_socket);
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

	logger_planificador(escribir_loguear,l_info,"\nAbriendo consola...\n");

	while(TRUE){

		//Trae la linea de consola
		buffer = readline(">");
		//consola_leer_stdin(buffer, MAX_LINEA);

		res = consola_derivar_comando(buffer);

		free(buffer);

		//Sale de la consola con exit
		if(res)
			break;
	}

	pthread_exit(0);
	return 0;
}

int consola_derivar_comando(char * buffer){

	int comando_key;
	char *comando = NULL;
	char *parametro1 = NULL;
	char *parametro2 = NULL;
	int res = 0;

	logger_planificador(loguear,l_debug,"string recibido: %s\n",buffer);

	// Separa la linea de consola en comando y sus parametros
	consola_obtener_parametros(buffer, &comando, &parametro1, &parametro2);
	logger_planificador(loguear,l_debug,"comando: %s p1: %s p2: %s\n",comando,parametro1,parametro2);

	printf("\n");

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
		case ckill:
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
			logger_planificador(escribir_loguear,l_info,"Saliendo de la consola...");
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
			logger_planificador(escribir_loguear,l_warning,"No reconozco el comando vieja...");
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
		key = ckill;

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
		//log_info(logger,"parte %d: %s\n", j,comandos[j]);
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
			logger_planificador(escribir_loguear,l_error,"Error en read() desde STDIN");
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
			//log_info(logger,"Message too large and will be chopped. Please try to be shorter next time.\n");
			fflush(STDIN_FILENO);
			break;
		  }
		}

	}while ((read_count > 0) && (total_read < max_len) && (c != '\n'));

	size_t len = strlen(read_buffer);
	if (len > 0 && read_buffer[len - 1] == '\n'){
		read_buffer[len - 1] = '\0';
	}

	//log_info(logger,"Read from stdin %zu bytes. Let's prepare message to send.\n", strlen(read_buffer));

	return 0;
}

void consola_pausar(void)
{
	/* Pausar/Continuar planificación(^2): El Planificador no le dará nuevas órdenes de
	 * ejecución a ningún ESI mientras se encuentre pausado.
	 *
	 * ^2: Esto se puede lograr ejecutando una sycall bloqueante que espere la entrada de un humano.
	 */

	logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: pausar");

	estado_pausa_por_consola = pausado;
	logger_planificador(escribir_loguear,l_info,"Se pausó la ejecución.");

	return;
}

void consola_continuar(void)
{
	logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: continuar");

	estado_pausa_por_consola = no_pausado;
	planificar();
	logger_planificador(escribir_loguear,l_info,"Se reanudó la ejecución.");

	return;
}

void consola_bloquear_clave(char* clave , char* id){

	int pid;

	/* Debe simular GET CLAVE PID */
	if(clave == NULL || id == NULL)
	{
		logger_planificador(escribir_loguear,l_warning,"CONSOLA> Parametros incorrectos (bloquear <clave> <id>)");
	}
	else
	{
		pid = atoi(id);
		logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: Bloquear clave: %s id: %d",clave, pid);

		//Si al intentar bloquear la clave falla, bloqueo el esi
		if(bloquear_clave(clave, pid)){
			bloquear_esi_pid(clave,pid);
		}
	}

	return;
}

void consola_desbloquear_clave(char* clave){

	/* desbloquear clave: Se desbloqueara el primer proceso ESI bloquedo por la clave
	 * especificada.
	 *
	 */

	if(clave == NULL){
		logger_planificador(escribir_loguear,l_warning,"CONSOLA> Parametros incorrectos (desbloquear <clave>)");
	}
	else
	{
		logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: Desbloquear clave: %s",clave);
		desbloquear_clave(clave);
		planificar();
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
	/*
	if(recurso == NULL){
		logger_planificador(escribir_loguear,l_warning,"CONSOLA> Parametros incorrectos (listar <recurso>)");
	}
	else{
		t_list* lista_recursos;

		logger_planificador(escribir_loguear,l_info, "CONSOLA> COMANDO: Listar recurso encolados: %s",recurso);

		bool esta_bloqueado_por_clave(t_pcb_esi* esi){
			return !strcmp(recurso, esi->clave_bloqueo);
		}

		lista_recursos = list_filter(esi_bloqueados,(void*)esta_bloqueado_por_clave);

		void mostrar_esi_consola(t_pcb_esi* esi){
			logger_planificador(escribir_loguear,l_info, "Proceso ESI bloqueado: %i", esi->pid);
			return;
		}

		if(list_size(lista_recursos)>0){
			list_iterate(lista_recursos, (void*)mostrar_esi_consola);
		}
		else{
			logger_planificador(escribir_loguear,l_info,"NO se encontraron procesos ESI bloqueados por el recurso %s.", recurso);
		}
	 */

	if(recurso == NULL){
		log_warning(logger,"CONSOLA> Parametros incorrectos (listar <recurso>)");
	}
	else{
		log_info(logger, "CONSOLA> COMANDO: Listar recurso encolados: %s",recurso);

		t_list* lista_esis_bloqueados;

		lista_esis_bloqueados = esis_bloqueados_por_clave(recurso);
		mostrar_esis_consola(lista_esis_bloqueados);


		list_destroy(lista_esis_bloqueados);
	}

	return;
}

void consola_matar_proceso(char* id)
{
	/* kill ID: finaliza el proceso. Recordando la atomicidad mencionada en “bloquear”.
	 * Al momento de eliminar el ESI, se debloquearan las claves que tenga tomadas.
	 *
	 */

	int pid;
	t_pcb_esi * esi_aux = NULL;

	if(id == NULL){
		logger_planificador(escribir_loguear,l_warning,"CONSOLA> Parametros incorrectos (kill <id>)");
	}
	else{
		logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: kill ID: %s",id);

		pid = atoi(id);

		if(esi_en_ejecucion != NULL  && esi_en_ejecucion->pid == pid)
		{
			/* Si el esi esta en ejecucion, esperar a que termine la instrucción*/
			logger_planificador(escribir_loguear,l_info,"Esperando a que termine de ejecutar la sentencia\n");
			kill_en_ejecucion++;
			return;
		}

		kill_flag++;

		if (!finalizar_esi(pid,liberar))
		{
			logger_planificador(escribir_loguear,l_info,"ESI de ID %d Finalizado",pid);
		}
		else
		{
			logger_planificador(escribir_loguear,l_info,"No existe el ESI de ID %d en READY, BLOQUEADOS, ni en EJECUCION",pid);
		}

	}

	if(esi_en_ejecucion==NULL)
		planificar();

	return;
}

void consola_consultar_status_clave(char* clave_nombre)
{
	if(clave_nombre == NULL){
		log_warning(logger,"CONSOLA> Parametros incorrectos (status <clave>)");
	}
	else{
		log_info(logger,"CONSOLA> COMANDO Status para clave: %s.", clave_nombre);

		clave_status = strdup(clave_nombre);

		if(enviar_coordinador_clave_status(clave_nombre) < 0){
			logger_planificador(loguear, l_error, "ERROR al consultar status clave al Coordinador.");
		}
	}

	return;
}

void status_clave(t_status_clave* clave_st, char * clave)
{
	/*
	 status	clave: Con el objetivo de conoce el estado de una clave y de probar la correcta distribución de las mismas se deberan obtener los siguiente valores: (Este comando se utilizara para probar el sistema)
	-Valor, en caso de no poseer valor un mensaje quec lo indique.
	-Instancia actual en la cual se encuentra la clave. (En	caso de	que	la clave no	se encuentre en una instancia, no se debe mostrar este valor)
	-Instancia en la cual se guardaría actualmente la clave (Calcular este valor mediante el algoritmo de distribución(^4), pero sin afectar la distribución actual de las claves).
	-ESIs bloqueados a la espera de dicha clave.
	 */

	int res;
	char * nombre_instancia=NULL;
	char * valor=NULL;

	if(clave == NULL){
		logger_planificador(escribir_loguear,l_warning,"CONSOLA> Parametros incorrectos (status <clave>)");
	}
	else
	{
		logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: status clave: %s ",clave);


		if(clave_st->tamanio_instancia_nombre != -1){

			nombre_instancia = malloc(clave_st->tamanio_instancia_nombre);

			res = recv(coord_status_socket, nombre_instancia, clave_st->tamanio_instancia_nombre, 0);
			if(res < 0)
			{
				logger_planificador(escribir_loguear, l_error, "Error al recibir nombre instancia");

				//TODO Evaluar qué hacer
				//terminar_planificador();
				//exit(EXIT_FAILURE);
			}

		}

		//Valor de la clave
		if(clave_st->tamanio_valor != -1){

			valor = malloc(clave_st->tamanio_valor);

			res = recv(coord_status_socket, valor, clave_st->tamanio_valor, 0);
			if(res < 0)
			{
				logger_planificador(escribir_loguear, l_error, "Error al recibir el valor de la clave");

				//TODO Evaluar qué hacer
				//terminar_planificador();
				//exit(EXIT_FAILURE);
			}

		}

		switch(clave_st->cod)
		{
			//Buscar clave en lista interna de claves
				//1-Si no existe, devolver cod = 0
				//2-Si existe, mirar si tiene asociada una instancia
					//3-Si no tiene asociada instancia, simular, y devolver (no va a devolver valor)( cod =2)
					//4-Si tiene asociada instancia, consultarle a la misma
						//5-Si la inst esta caida, devolver 1 en cod
						//6-Si la inst no esta caida:
							//7-tiene valor: lo devuelve, devolver cod= 3
							//8-No tiene valor: devolver cod= 4

			case COORDINADOR_SIN_CLAVE:
				logger_planificador(escribir_loguear, l_info,"La clave %s no existe en el coordinador", clave);
				break;

			case INSTANCIA_CAIDA:
				logger_planificador(escribir_loguear, l_info,"La instancia donde se encotraba la clave %s era %s, está caida",clave,nombre_instancia);
				break;

			case INSTANCIA_SIMULADA:
				logger_planificador(escribir_loguear, l_info,"La instancia donde se encuentra la clave es %s, fue simulada",nombre_instancia);
				break;

			case CORRECTO_CONSULTA_VALOR:
				logger_planificador(escribir_loguear, l_info,"La instancia donde se encuentra la clave es %s",nombre_instancia);
				logger_planificador(escribir_loguear, l_info,"El valor de la clave es %s",valor);
				break;

			case INSTANCIA_SIN_CLAVE:
				logger_planificador(escribir_loguear, l_info,"La instancia donde se encuentra la clave es %s",nombre_instancia);
				logger_planificador(escribir_loguear, l_info,"El clave no tiene valor");
				break;

		}

		//ESIs bloqueados por la clave
		t_list* lista_esis_bloq_por_clave;

		lista_esis_bloq_por_clave = esis_bloqueados_por_clave(clave);

		logger_planificador(escribir_loguear, l_info,"Listado de ESIs bloqueados por clave %s: \n\n", clave);
		mostrar_esis_consola(lista_esis_bloq_por_clave);

		list_destroy(lista_esis_bloq_por_clave);

		if(valor!=NULL){
			free(valor);
		}

		if(nombre_instancia!=NULL){
			free(nombre_instancia);

		}

	}

	return;
}

void dummy_genera_deadlock(void)
{

	//TODO: Borrar funcion al terminar TODAS las pruebas

	t_pcb_esi * esi = NULL;
	t_claves_bloqueadas * clave_bloqueada = NULL;

	clave_bloqueada = malloc(sizeof(t_claves_bloqueadas));
	memset(clave_bloqueada, 0, sizeof(t_claves_bloqueadas));

	clave_bloqueada->clave = strdup("clave0");
	clave_bloqueada->pid = 0;
	list_add(claves_bloqueadas, clave_bloqueada);

	clave_bloqueada = malloc(sizeof(t_claves_bloqueadas));
	memset(clave_bloqueada, 0, sizeof(t_claves_bloqueadas));

	clave_bloqueada->clave = strdup("clave1");
	clave_bloqueada->pid = 1;
	list_add(claves_bloqueadas, clave_bloqueada);

	clave_bloqueada = malloc(sizeof(t_claves_bloqueadas));
	memset(clave_bloqueada, 0, sizeof(t_claves_bloqueadas));

	clave_bloqueada->clave = strdup("clave2");
	clave_bloqueada->pid = 2;
	list_add(claves_bloqueadas, clave_bloqueada);


	esi = malloc(sizeof(t_pcb_esi));
	memset(esi,0,sizeof(t_pcb_esi));

	esi->pid = 0;

	t_conexion_esi * conexion = malloc(sizeof(t_conexion_esi));
	esi->conexion = conexion;
	esi->estado = bloqueado;
	esi->instruccion_actual = 0;
	esi->tiempo_espera = 0;
	esi->ejec_anterior = 0;
	esi->clave_bloqueo = strdup("clave1");

	list_add(esi_bloqueados,esi);

	esi = malloc(sizeof(t_pcb_esi));
	memset(esi,0,sizeof(t_pcb_esi));
	esi->pid = 1;

	conexion = malloc(sizeof(t_conexion_esi));
	esi->conexion = conexion;

	esi->estado = bloqueado;
	esi->instruccion_actual = 0;
	esi->tiempo_espera = 0;
	esi->ejec_anterior = 0;
	esi->clave_bloqueo = strdup("clave2");

	list_add(esi_bloqueados,esi);

	esi = malloc(sizeof(t_pcb_esi));
	memset(esi,0,sizeof(t_pcb_esi));
	esi->pid = 2;

	conexion = malloc(sizeof(t_conexion_esi));
	esi->conexion = conexion;

	esi->estado = bloqueado;
	esi->instruccion_actual = 0;
	esi->tiempo_espera = 0;
	esi->ejec_anterior = 0;
	esi->clave_bloqueo = strdup("clave0");

	list_add(esi_bloqueados,esi);

}

void consola_consultar_deadlock(void)
{
	/* deadlock: Esta consola también permitirá analizar los deadlocks que existan en el
	 * sistema y a que ESI están asociados.
	 * Pudiendo resolverlos manualmente con la sentencia de kill previamente descrita.
	 */

	int tam_block = list_size(esi_bloqueados);
	int analizar= 0;
	t_pcb_esi * esi_aux = NULL;
	t_pcb_esi * esi_analizado = NULL;
	t_list * claves_bloqueadas_aux;
	t_claves_bloqueadas * clave_analizada;
	t_claves_bloqueadas * clave_aux;

	t_deadlock * esi_inicial;
	t_deadlock * esi_aux_deadlock;

	t_list * deadlock;
	t_list * lista_deadlocks;

	int seguir_buscando = 0;


	logger_planificador(escribir_loguear,l_info,"CONSOLA> COMANDO: deadlock\n");

	lista_deadlocks = list_create();

	for(int i=0;i<tam_block;i++)
	{
		esi_analizado = list_get(esi_bloqueados,i);
		esi_aux = esi_analizado;

		//Si el esi ya se encuentra en un deadlock, no lo analizo
		analizar = buscar_esi_en_deadlock(lista_deadlocks,esi_analizado->pid);
		if(analizar){
			logger_planificador(loguear,l_debug,"No se analiza el esi %d porque ya esta en un deadlock",esi_analizado->pid);
			continue;
		}


		deadlock = list_create();

		esi_inicial = malloc(sizeof(t_deadlock));
		memset(esi_inicial,0,sizeof(t_deadlock));
		esi_inicial->pid = esi_aux->pid;
		esi_inicial->clave_pedida = strdup(esi_aux->clave_bloqueo);

		list_add(deadlock,esi_inicial);

		logger_planificador(loguear,l_debug,"Analizando al esi %d",esi_analizado->pid);
		seguir_buscando++;

		while(seguir_buscando)
		{

			logger_planificador(loguear,l_debug,"busco la clave %s\n",esi_aux->clave_bloqueo);
			clave_aux = buscar_clave_bloqueada(esi_aux->clave_bloqueo);
			if(clave_aux)
			{
				logger_planificador(loguear,l_debug,"Encontre la clave bloqueada %s\n",clave_aux->clave);

				esi_aux_deadlock = malloc(sizeof(t_deadlock));
				memset(esi_aux_deadlock,0,sizeof(t_deadlock));
				esi_aux_deadlock->pid = clave_aux->pid;
				esi_aux_deadlock->clave_tomada = strdup(clave_aux->clave);

				list_add(deadlock,esi_aux_deadlock);

				logger_planificador(loguear,l_debug,"busco esi %d que tiene %s\n",clave_aux->pid,clave_aux->clave);
				esi_aux = buscar_esi_en_lista_pid(esi_bloqueados,clave_aux->pid);
				if(esi_aux)
				{
					logger_planificador(loguear,l_debug,"el esi %d fue bloqueado por %s\n",esi_aux->pid,esi_aux->clave_bloqueo);
					esi_aux_deadlock->clave_pedida = strdup(esi_aux->clave_bloqueo);


					logger_planificador(loguear,l_debug,"busco la clave bloqueada %s\n",esi_aux->clave_bloqueo);
					clave_aux = buscar_clave_bloqueada(esi_aux->clave_bloqueo);
					if(clave_aux)
					{
						logger_planificador(loguear,l_debug,"La clave bloqueada %s la tiene el esi %d\n",clave_aux->clave,clave_aux->pid);

						logger_planificador(loguear,l_debug,"Comparo al esi %d que tiene la clave %s con el esi analizado %d\n",clave_aux->pid,clave_aux->clave,esi_analizado->pid);
						if(clave_aux->pid == esi_analizado->pid)
						{
							esi_inicial->clave_tomada = strdup(clave_aux->clave);
							logger_planificador(loguear,l_debug,"Se encontró un deadlock analizando al esi %d\n",esi_analizado->pid);

							seguir_buscando = 0;

							t_list* deadlock_aux = list_create();
							list_add_all(deadlock_aux,deadlock);

							list_add(lista_deadlocks,deadlock_aux);
							list_clean(deadlock);

						}
					}
					else{
						logger_planificador(loguear,l_debug,"No hay deadlock para %d\n",esi_aux->pid);
						seguir_buscando = 0;
					}
				}
				else{
					logger_planificador(loguear,l_debug,"No hay deadlock para %d\n",esi_aux->pid);
					seguir_buscando = 0;
				}
			}
			else{
				logger_planificador(loguear,l_debug,"No hay deadlock para %d\n",esi_aux->pid);
				seguir_buscando = 0;
			}

		}//End while

		list_destroy(deadlock);

	}//End for

	if(list_size(lista_deadlocks))
	{
		list_iterate(lista_deadlocks,(void*)mostrar_deadlock);
	}
	else{
		logger_planificador(escribir_loguear,l_info,"No se encontraron deadlocks en el estado actual del sistema\n");
	}


	destruir_lista_deadlocks(lista_deadlocks);
	return;
}

int buscar_esi_en_deadlock(t_list* lista_deadlocks,int pid){

	bool deadlock_tiene_pid(t_list * deadlock)
	{

		bool is_pid_en_deadlock(t_deadlock * esi_en_deadlock){
			return (esi_en_deadlock->pid==pid);
		}

		return(list_find(deadlock,(void*)is_pid_en_deadlock));
	}

	t_list * aux = NULL;

	if(list_size(lista_deadlocks)){

		aux = list_find(lista_deadlocks,(void*)deadlock_tiene_pid);
			if(aux!=NULL)
				return 1;
			else
				return 0;

	}
	else{
		return 0;
	}



}

void destruir_lista_deadlocks(t_list * lista_deadlocks){

	void destruir_deadlock(t_list * deadlock)
	{

		void destruir_esi_en_deadlock(t_deadlock * esi_en_deadlock )
		{
			if(esi_en_deadlock->clave_pedida!=NULL)
			{
				free(esi_en_deadlock->clave_pedida);
				esi_en_deadlock->clave_pedida = NULL;
			}

			if(esi_en_deadlock->clave_tomada!=NULL)
			{
				free(esi_en_deadlock->clave_tomada);
				esi_en_deadlock->clave_tomada = NULL;
			}

			free(esi_en_deadlock);
		}


		list_destroy_and_destroy_elements(deadlock,(void*)destruir_esi_en_deadlock);
	}


	list_destroy_and_destroy_elements(lista_deadlocks,(void*)destruir_deadlock);
}

void mostrar_deadlock(t_list * deadlock)
{
	void mostrar_esi_en_deadlock(t_deadlock * esi)
	{
		logger_planificador(escribir_loguear,l_info,"PID esi: %d\n", esi->pid);
		logger_planificador(escribir_loguear,l_info,"Clave pedida: %s\n", esi->clave_pedida);
		logger_planificador(escribir_loguear,l_info,"Clave tomada: %s\n", esi->clave_tomada);

		printf("\n");

		return;
	}

	printf("Deadlock encontrado:\n\n");

	list_iterate(deadlock,(void*)mostrar_esi_en_deadlock);
}

void mostrar_lista(char * name)
{
	t_list * lista;

	if(name == NULL)
		logger_planificador(escribir,l_warning,"Parametros incorrectos (mostrar <lista>)");
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
			logger_planificador(escribir,l_warning,"No existe la lista %s: ",name);
			list_destroy(lista);
			return;
		}

		logger_planificador(escribir,l_info,"\n\nEstado actual de la lista de %s: \n\n",name);

		list_iterate(lista,(void*)mostrar_esi);

		logger_planificador(escribir,l_info,"\nTamaño: %d \n",list_size(lista));

		list_destroy(lista);
		printf("\n");
	}

	return;
}

void mostrar_esi_en_ejecucion(void)
{
	if(esi_en_ejecucion!=NULL)
	{
		printf("\n");
		logger_planificador(escribir_loguear,l_info,"PID ESI en ejecución actual: %d",esi_en_ejecucion->pid);
		logger_planificador(escribir_loguear,l_info,"Estado: %d: ",esi_en_ejecucion->estado);
		logger_planificador(escribir_loguear,l_info,"Estimacion real: %f",esi_en_ejecucion->estimacion_real);
		logger_planificador(escribir_loguear,l_info,"Estimacion actual: %f",esi_en_ejecucion->estimacion_actual);
		logger_planificador(escribir_loguear,l_info,"Response Ratio: %f",esi_en_ejecucion->response_ratio);
		printf("\n");
	}
	else
		logger_planificador(escribir_loguear,l_warning,"No hay ningun ESI en ejecucion");

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

	return;
}

void mostrar_clave_bloqueada(t_claves_bloqueadas * clave_bloqueada)
{
	printf("Clave Bloqueada: %s\n", clave_bloqueada->clave);
	printf("PID ESI que la tiene asignada: %d\n", clave_bloqueada->pid);
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
	printf("\n");
	//logger_planificador(escribir,l_info,"Replanificando...\n");
	if(esi_en_ejecucion == NULL){
		//Si no está pausada la ejecución por consola
		if(estado_pausa_por_consola != pausado){
			obtener_proximo_ejecucion();
		}
	}

	return;
}

void obtener_proximo_ejecucion(void)
{
	t_pcb_esi * ejec_ant;
	t_list * lista_aux;

	ejec_ant = esi_en_ejecucion;

	/* SJF debe copiar la lista de listos a una lista auxiliar,
	 * ordenarla por estimacion mas corta, tomar el primero, destruir la lista auxiliar.
	 * Eso para ambos casos
	 */

	lista_aux = list_duplicate(esi_listos);
	logger_planificador(escribir_loguear,l_info,"Planificando por %s...",config.algoritmo);

	if( (!strcmp(config.algoritmo, "SJF-SD")) || (!strcmp(config.algoritmo, "SJF-CD")))
	{
		ordenar_lista_estimacion(lista_aux);
	}

	/* HRRN: Similar al anterior, pero ordenar por ratio
	 * Primero hay que calcularlo sobre todos los elementos de la lista ready
	 * Obtener el de response ratio mas alto
	 */
	else if(!strcmp(config.algoritmo, "HRRN") )
	{
		logger_planificador(escribir_loguear,l_info,"Calculando RR para toda la lista ready...");
		list_iterate(lista_aux,(void*)calcular_response_ratio);
		ordenar_lista_response_ratio(lista_aux);
	}

	/* FIFO: Directamente saca el primer elemento de la lista y lo pone en ejecucion
	 * Por default hace fifo
	 */
	esi_en_ejecucion = list_remove(lista_aux,0);
	if(!list_is_empty(esi_listos))
	{
		logger_planificador(escribir_loguear,l_info,"Saco de la lista de listos el próximo esi a ejecutar");
		sacar_esi_de_lista_pid(esi_listos,esi_en_ejecucion->pid);
		esi_en_ejecucion->estado = en_ejecucion;
	}
	else
	{
		esi_en_ejecucion = NULL;
		logger_planificador(escribir,l_info,"No hay ESIs para ejecutar! Todo en orden...");
	}

	list_destroy(lista_aux);

	//Si hubo un cambio en el esi en ejecucion, debo avisarle al nuevo esi en ejecucion que es su turno
	if((esi_en_ejecucion != NULL) && (ejec_ant != esi_en_ejecucion))
	{

		//log_info(logger,"Aca le debo avisar al esi %d que es su turno\n", esi_en_ejecucion->pid);
		int res = enviar_esi_confirmacion_sentencia(esi_en_ejecucion);
		if(!res)
		{
			//Validar resultado del envio
		}
	}

	return;
}

int enviar_esi_confirmacion_sentencia(t_pcb_esi * pcb_esi)
{

	t_content_header * header = crear_cabecera_mensaje(planificador,esi,OPERACION_CONF_SENTENCIA,sizeof(t_confirmacion_sentencia));

	t_confirmacion_sentencia * conf = NULL;

	conf = malloc(sizeof(t_confirmacion_sentencia));

	conf->pid 			= pcb_esi->pid;
	conf->ejec_anterior = pcb_esi->ejec_anterior;
	conf->resultado		= 0;

	logger_planificador(escribir_loguear,l_info,"Aviso al ESI %d que es su turno\n",pcb_esi->pid);

	int res_send = send(pcb_esi->conexion->socket, header, sizeof(t_content_header), 0);
	if(res_send < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error send header al ESI %d",pcb_esi->pid);
	}

	res_send = send(pcb_esi->conexion->socket, conf, sizeof(t_confirmacion_sentencia), 0);
	if(res_send < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error send ejec al ESI %d",pcb_esi->pid);
	}

	free(conf);
	destruir_cabecera_mensaje(header);
	return res_send;
}

void enviar_esi_confirmacion_kill(int pid)
{

	t_pcb_esi * esi_aux= NULL;

	if(esi_en_ejecucion!=NULL && pid == esi_en_ejecucion->pid)
	{
		esi_aux = esi_en_ejecucion;
	}
	else if(buscar_esi_en_lista_pid(esi_listos,pid))
	{
		esi_aux = buscar_esi_en_lista_pid(esi_listos,pid);
	}
	else if(buscar_esi_en_lista_pid(esi_bloqueados,pid))
	{
		esi_aux = buscar_esi_en_lista_pid(esi_bloqueados,pid);
	}

	if(esi_aux!=NULL)
	{

		t_content_header * header = crear_cabecera_mensaje(planificador,esi,OPERACION_CONF_KILL,sizeof(t_confirmacion_sentencia));

		logger_planificador(escribir_loguear,l_info,"Aviso al ESI %d que debe finalizar por KILL",esi_aux->pid);

		int res_send = send(esi_aux->conexion->socket, header, sizeof(t_content_header), 0);
		if(res_send < 0)
		{
			logger_planificador(escribir_loguear,l_error,"Error send header al ESI %d",esi_aux->pid);
		}

		destruir_cabecera_mensaje(header);

	}

	return;
}

int enviar_coordinador_resultado_consulta(int socket, int resultado)
{
	t_content_header * header = crear_cabecera_mensaje(planificador,coordinador,OPERACION_RES_CLAVE_COORD,sizeof(int));

	int * res = 0;

	res = malloc(sizeof(int));

	*res = resultado;

	logger_planificador(loguear,l_debug,"Envío resultado (%d) de la consulta al coordinador",*res);

	int res_send = send(socket, header, sizeof(t_content_header), 0);
	if(res_send < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error send header resultado consulta coordinador");
	}

	res_send = send(socket, res, sizeof(int), 0);
	if(res_send < 0)
	{
		logger_planificador(escribir_loguear,l_error,"Error send resultado consulta coordinador\n");
	}

	free(res);
	destruir_cabecera_mensaje(header);
	return res_send;
}

int enviar_coordinador_clave_status(char* clave_nombre){

	t_content_header *header = crear_cabecera_mensaje(planificador, coordinador, PLANIFICADOR_COORDINADOR_CMD_STATUS, strlen(clave_nombre)+1);

	logger_planificador(loguear, l_info, "Envío consulta de status por clave %s.", clave_nombre);

	//Envía Header al Coordinador
	int res_send = send(coord_status_socket, header, sizeof(t_content_header), 0);
	if(res_send < 0){
		log_error(logger, "Error al enviar header al Coordinador.");
	}

	//Envía Body al Coordinador
	res_send = send(coord_status_socket , clave_nombre, strlen(clave_nombre)+1, 0);
	if(res_send < 0){
		log_error(logger, "Error en send body al Coordinador.");
	}

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

void desalojar_ejecucion(void){

	if(esi_en_ejecucion!=NULL)
	{
		estimar_esi(esi_en_ejecucion);
		esi_en_ejecucion = NULL;
	}

	return;
}

void terminar_planificador(void)
{
	logger_planificador(escribir_loguear,l_trace,"Terminando Planificador...\n");

	list_destroy_and_destroy_elements(esi_listos,(void*)destruir_esi);
	list_destroy_and_destroy_elements(esi_bloqueados,(void*)destruir_esi);
	list_destroy_and_destroy_elements(esi_terminados,(void*)destruir_esi);
	list_destroy_and_destroy_elements(claves_bloqueadas,(void*)destruir_clave_bloqueada);

	if(esi_en_ejecucion!=NULL){
		destruir_esi(esi_en_ejecucion);
	}

	log_destroy(logger);

	if(config.algoritmo!=NULL)
	{
		free(config.algoritmo);
		config.algoritmo = NULL;
	}

	if(config.ip_coordinador!=NULL)
	{
		free(config.ip_coordinador);
		config.ip_coordinador = NULL;
	}

	if(config.puerto_coordinador!=NULL)
	{
		free(config.puerto_coordinador);
		config.puerto_coordinador = NULL;
	}

	if(config.puerto_escucha!=NULL)
	{
		free(config.puerto_escucha);
		config.puerto_escucha = NULL;
	}

	return;
}

//***********************//
//FUNCIONES DE ESI		 //
//***********************//

t_pcb_esi * crear_esi(t_conexion_esi * conexion)
{
	t_pcb_esi * esi;

	esi = malloc(sizeof(t_pcb_esi));

	esi->pid = esi_seq_pid;
	conexion->pid = esi->pid;
	esi->conexion = conexion;
	esi->estado = listo;
	esi->estimacion_real = config.estimacion_inicial;
	esi->estimacion_actual = config.estimacion_inicial;
	esi->estimacion_anterior = config.estimacion_inicial;
	esi->instruccion_actual = 0;
	esi->tiempo_espera = 0;
	esi->ejec_anterior = 0;
	esi->clave_bloqueo = NULL;
	esi_seq_pid++;

	return esi;
}

//TODO Considerar eliminar esta función
void mostrar_esi(t_pcb_esi * esi)
{
	printf("PID esi: %d\n", esi->pid);
	printf("Estado: %d\n", esi->estado);
	printf("Estimacion faltante: %f\n", esi->estimacion_actual);
	printf("Estimacion Real: %f\n", esi->estimacion_real);
	printf("Estimacion anterior: %f\n", esi->estimacion_anterior);

	if(!strcmp(config.algoritmo, "HRRN"))
		printf("Response Ratio: %f\n", esi->response_ratio);

	if(esi->clave_bloqueo!=NULL)
		printf("Clave que lo bloqueó: %s\n", esi->clave_bloqueo);

	if(esi->estado == listo)
		printf("Tiempo de espera en ready: %d\n",esi->tiempo_espera);

	printf("\n");

	return;
}

int bloquear_esi_pid(char * clave,int pid)
{
	if(esi_en_ejecucion != NULL  && esi_en_ejecucion->pid == pid)
	{
		/* Si el esi esta en ejecucion, esperar a que termine la instrucción y desalojar */

		logger_planificador(escribir_loguear,l_info,"Esperando a que termine de ejecutar la sentencia\n");
		if(esi_en_ejecucion->clave_bloqueo!=NULL)
		{
			free(esi_en_ejecucion->clave_bloqueo);
			esi_en_ejecucion->clave_bloqueo = NULL;
		}

		esi_en_ejecucion->clave_bloqueo = strdup(clave);

		bloqueo_en_ejecucion++;
	}
	else if (buscar_esi_en_lista_pid(esi_listos, pid) )
	{
		// Si el esi está en la lista de listos, hay que pasarlo a bloqueado
		t_pcb_esi * esi_aux = sacar_esi_de_lista_pid(esi_listos,pid);

		esi_aux->clave_bloqueo = strdup(clave);
		esi_aux->estado = bloqueado;

		list_add(esi_bloqueados,esi_aux);

		logger_planificador(escribir_loguear,l_info,"El ESI %d estaba en listos, se pasó a bloqueados",pid);
	}
	else
	{
		logger_planificador(escribir_loguear,l_info,"No existe el ESI de ID %d en READY ni en EJECUCION",pid);
		return 1;
	}

	return 0;
}

int destruir_esi(t_pcb_esi * esi)
{
	esi->conexion->socket = NO_SOCKET;
	if(esi->clave_bloqueo!=NULL)
	{
		free(esi->clave_bloqueo);
		esi->clave_bloqueo = NULL;
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

void aumentar_tiempo_espera_ready(void){

	void aumentar_tiempo_espera_esi(t_pcb_esi * esi)
	{
		esi->tiempo_espera++;
	}

	list_iterate(esi_listos,(void*)aumentar_tiempo_espera_esi);

}

void ordenar_lista_estimacion(t_list * lista)
{
	bool is_estimacion_menor(t_pcb_esi * esi1, t_pcb_esi * esi2)
	{
		return ( (esi1->estimacion_real < esi2->estimacion_real) || (esi1->estimacion_real == esi2->estimacion_real) );
	}

	/*
	* El comparador devuelve si el primer parametro debe aparecer antes que el
	* segundo en la lista
	*/

	list_sort(lista, (void*)is_estimacion_menor);

	return;
}

void ordenar_lista_response_ratio(t_list* lista)
{
	bool is_response_ratio_mayor(t_pcb_esi * esi1, t_pcb_esi * esi2)
	{
		return ( (esi1->response_ratio > esi2->response_ratio ) || (esi1->response_ratio == esi2->response_ratio) );
	}

	list_sort(lista,(void*)is_response_ratio_mayor);
	return;
}

int estimar_esi(t_pcb_esi * esi){

	config.alfa = ALPHA;
	esi->estimacion_anterior = esi->estimacion_real;

	esi->estimacion_real = ( (config.alfa / 100) * esi->instruccion_actual ) +
					  ( ( 1 - (config.alfa / 100) ) * esi->estimacion_real );

	esi->estimacion_actual  = esi->estimacion_real;
	esi->instruccion_actual = 0;

	return 0;
}

void calcular_response_ratio(t_pcb_esi* esi)
{
	if(esi->estimacion_real != 0)
	{
		esi->response_ratio = (esi->tiempo_espera + esi->estimacion_real) / esi->estimacion_real;
	}

}

int confirmar_bloqueo_ejecucion(void)
{
	logger_planificador(escribir_loguear,l_info,"Sentencia terminada!");

	esi_en_ejecucion->estado = bloqueado;
	esi_en_ejecucion->ejec_anterior = 1;

	list_add(esi_bloqueados,esi_en_ejecucion);
	logger_planificador(escribir_loguear,l_info,"Desalojo la ejecucion...\n");
	esi_en_ejecucion = NULL;

	bloqueo_en_ejecucion = 0;

	return 0;
}

int confirmar_desalojo_ejecucion(void)
{
	if(esi_por_desalojar!=NULL)
	{
		esi_en_ejecucion->estado = listo;
		esi_en_ejecucion->tiempo_espera = 0;

		esi_por_desalojar->estado = en_ejecucion;

		list_add(esi_listos, esi_en_ejecucion);
		esi_en_ejecucion = NULL;

		esi_en_ejecucion  = sacar_esi_de_lista_pid(esi_listos,esi_por_desalojar->pid);

		esi_por_desalojar = NULL;
		desalojo_en_ejecucion = 0;

		return 0;
	}

	return 1;
}

int finalizar_esi(int pid_esi, int liberar)
{
	t_pcb_esi * esi_aux;

	//Si se finaliza el esi por KILL, antes, debe mandarle el aviso al esi
	if(kill_flag)
	{
		enviar_esi_confirmacion_kill(pid_esi);
		kill_flag = 0;
	}

	if(esi_en_ejecucion!=NULL && pid_esi == esi_en_ejecucion->pid)
	{
		esi_aux = esi_en_ejecucion;
		esi_en_ejecucion = NULL;
	}
	else if(buscar_esi_en_lista_pid(esi_listos,pid_esi))
	{
		esi_aux = sacar_esi_de_lista_pid(esi_listos,pid_esi);
	}
	else if(buscar_esi_en_lista_pid(esi_bloqueados,pid_esi))
	{
		esi_aux = sacar_esi_de_lista_pid(esi_bloqueados,pid_esi);
	}
	else
	{
		return 1;
	}

	if(esi_aux!=NULL)
	{
		/*
		 * Si el esi tenia recursos tomados, debo liberarlos todos
		 */
		if(liberar)
		{
			desbloquear_claves_bloqueadas_pid(esi_aux->pid);
		}


		if(esi_aux->clave_bloqueo!=NULL)
		{
			free(esi_aux->clave_bloqueo);
			esi_aux->clave_bloqueo = NULL;
		}

		esi_aux->estado = terminado;
		list_add(esi_terminados, esi_aux);
		logger_planificador(escribir_loguear,l_info,"ESI %d pasado a finalizados",esi_aux->pid);
		cerrar_conexion_esi(esi_aux->conexion);
	}

	return 0;
}

//*******************************//
//FUNCIONES DE CLAVES BLOQUEADAS //
//*******************************//

int bloquear_clave(char* clave , int pid)
{
	t_claves_bloqueadas * clave_bloqueada;

	if((clave_bloqueada = buscar_clave_bloqueada(clave)) == NULL)
	{
		clave_bloqueada = malloc(sizeof(t_claves_bloqueadas));

		memset(clave_bloqueada, 0, sizeof(t_claves_bloqueadas));

		clave_bloqueada->clave = strdup(clave);
		clave_bloqueada->pid = pid;

		list_add(claves_bloqueadas, clave_bloqueada);

		logger_planificador(escribir_loguear,l_info,"Se creó la clave bloqueada %s",clave);

		//Busco si existe el ESI en el sistema
		if((esi_en_ejecucion != NULL  && esi_en_ejecucion->pid == pid) || 	// EJEC
		  (buscar_esi_en_lista_pid(esi_listos, pid) ) )						// READY
		{
			logger_planificador(escribir_loguear,l_info,"Se asigna la clave al ESI %d\n",pid);
		}
		else
		{
			clave_bloqueada->pid = -1;
			if(pid>-1)
				logger_planificador(escribir_loguear,l_info,"No existe el ESI de ID %d en READY ni en EJECUCION, se asigna la clave a sistema\n",pid);
			else
				logger_planificador(escribir_loguear,l_info,"Se asigna la clave a sistema\n");
		}
	}
	else
	{
		logger_planificador(escribir_loguear,l_info,"La clave %s ya está bloqueada! No se agrega a la lista",clave);
		if(clave_bloqueada->pid == pid)
		{
			logger_planificador(escribir_loguear,l_info,"La clave %s ya está asignada al ESI %d\n",clave,pid);
			return 0;
		}
		else if(clave_bloqueada->pid > -1)
			logger_planificador(escribir_loguear,l_info,"La clave %s está asignada al ESI %d\n",clave,clave_bloqueada->pid);
		else
			logger_planificador(escribir_loguear,l_info,"La clave %s está asignada a sistema\n",clave);

		return 1;
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
		logger_planificador(escribir_loguear,l_info,"La clave %s no está bloqueada\n",clave);
		return 1;
	}

	//Busco el primer esi bloqueado por la clave a desbloquear
	t_pcb_esi * esi_a_desbloquear = buscar_esi_bloqueado_por_clave(clave);
	if(esi_a_desbloquear!=NULL)
	{
		//Si existe, lo remuevo de los bloqueados
		esi_a_desbloquear = sacar_esi_bloqueado_por_clave(clave);

		logger_planificador(escribir_loguear,l_info,"El ESI %d estaba bloqueado por la clave %s, se pasa a ready",esi_a_desbloquear->pid,clave);

		free(esi_a_desbloquear->clave_bloqueo);
		esi_a_desbloquear->clave_bloqueo = NULL;
		esi_a_desbloquear->estado = listo;
		esi_a_desbloquear->tiempo_espera = 0;
		estimar_esi(esi_a_desbloquear);

		//Lo agrego a Ready
		list_add(esi_listos,esi_a_desbloquear);

		//Busco si hay otro esi bloqueado por la misma clave
		//Si no existe otro esi bloqueado por esa clave, hay que eliminar la clave de la lista de claves bloqueadas
		if(buscar_esi_bloqueado_por_clave(clave)==NULL)
		{
			logger_planificador(escribir_loguear,l_info,"No hay otro ESI bloqueados por la clave %s, se elimina de la lista de claves bloqueadas\n",clave);
			list_remove_and_destroy_by_condition(claves_bloqueadas,(void*)is_clave_bloqueada,(void*)destruir_clave_bloqueada);
			return 2;
		}
	}
	else
	{
		//Si no hay esi bloqueado por esa clave, solamente lo elimino de las claves bloqueadas
		logger_planificador(escribir_loguear,l_info,"No hay ningun ESIs bloqueados por la clave %s, se elimina de la lista de claves bloqueadas\n",clave);
		list_remove_and_destroy_by_condition(claves_bloqueadas,(void*)is_clave_bloqueada,(void*)destruir_clave_bloqueada);
		return 2;
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

void desbloquear_claves_bloqueadas_pid(int pid)
{
	t_list * claves_bloqueadas_aux;

	bool is_bloqueado_pid(t_claves_bloqueadas * clave)
	{
		return(clave->pid==pid);
	}

	void desbloquear_recursos(t_claves_bloqueadas * clave)
	{
		//Si desbloqueo la clave, pero no la destruyó de la lista, detruyo el local
		if(!desbloquear_clave(clave->clave))
			destruir_clave_bloqueada(clave);
	}

	claves_bloqueadas_aux = list_filter(claves_bloqueadas, (void*)is_bloqueado_pid);

	if(!list_is_empty(claves_bloqueadas_aux))
	{
		logger_planificador(escribir_loguear,l_info,"El ESI %d tiene recursos tomados, liberando claves...",pid);
		list_iterate(claves_bloqueadas_aux,(void*)desbloquear_recursos);
	}

	list_destroy(claves_bloqueadas_aux);

	return;
}

void confirmar_bloqueo_por_get(void)
{
	bloquear_clave(clave_a_bloquear_por_get->clave, clave_a_bloquear_por_get->pid);
	bloqueo_por_get = 0;
	free(clave_a_bloquear_por_get);
	return;
}

void confirmar_desbloqueo_por_store(void)
{
	desbloquear_clave(clave_a_desbloquear_por_store->clave);
	desbloqueo_por_store = 0;
	free(clave_a_desbloquear_por_store);
	return;
}

int confirmar_pausa_por_consola(){

	logger_planificador(escribir_loguear,l_info,"Sentencia pausada.");

	esi_en_ejecucion->estado = listo;
	esi_en_ejecucion->tiempo_espera = 0;

	list_add(esi_listos, esi_en_ejecucion);
	esi_en_ejecucion = NULL;

	logger_planificador(escribir_loguear,l_info,"ESI en ejecución desalojado.\n");

	return 0;
}

int confirmar_kill_ejecucion(void){

	enviar_esi_confirmacion_kill(esi_en_ejecucion->pid);
	finalizar_esi(esi_en_ejecucion->pid,liberar);

	return 0;
}

void leer_configuracion_desde_archivo(char* path_archivo){

	arch_config = config_create(path_archivo);

	if(arch_config==NULL){
		logger_planificador(escribir_loguear,l_error,"ERROR. No se pudo obtener un archivo de configuración.");
		terminar_planificador();
		exit(EXIT_FAILURE);
	}

	char* atributo=NULL;
	char** claves_bloqueadas_config=NULL;

	logger_planificador(escribir_loguear,l_info,"Obteniendo configuraciones iniciales desde archivo %s",path_archivo);

	//IP_COORDINADOR
	atributo = strdup("IP_COORDINADOR");
	if(config_has_property(arch_config, atributo)){
		config.ip_coordinador = strdup(config_get_string_value(arch_config, atributo));
		logger_planificador(escribir_loguear,l_info,"%s: %s", atributo, config.ip_coordinador);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s, default: %s", atributo, IP_COORD);
		config.ip_coordinador = strdup(IP_COORD);
	}

	free(atributo);
	atributo = NULL;

	//PUERTO_COORDINADOR
	atributo = strdup("PUERTO_COORDINADOR");
	if(config_has_property(arch_config, atributo)){
		config.puerto_coordinador = strdup(config_get_string_value(arch_config, atributo));
		logger_planificador(escribir_loguear,l_info,"%s: %s", atributo, config.puerto_coordinador);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s, default: %s", atributo, PORT_COORD);
		config.puerto_coordinador = strdup(PORT_COORD);
	}

	free(atributo);
	atributo = NULL;

	//PUERTO_ESCUCHA
	atributo = strdup("PUERTO_ESCUCHA");
	if(config_has_property(arch_config, atributo)){
		config.puerto_escucha = strdup(config_get_string_value(arch_config, atributo));
		logger_planificador(escribir_loguear,l_info,"%s: %s", atributo, config.puerto_escucha);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s, default: %s", atributo, PORT_ESCUCHA);
		config.puerto_escucha = strdup(PORT_ESCUCHA);
	}

	free(atributo);
	atributo = NULL;

	//ALGORITMO
	atributo = strdup("ALGORITMO");
	if(config_has_property(arch_config, atributo)){
		config.algoritmo = strdup(config_get_string_value(arch_config, atributo));
		logger_planificador(escribir_loguear,l_info,"%s: %s", atributo, config.algoritmo);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s, default: %s", atributo, ALGORITMO_PLAN_SJFSD);
		config.algoritmo = strdup(ALGORITMO_PLAN_SJFSD);
	}

	free(atributo);
	atributo = NULL;

	//ALFA
	atributo = strdup("ALFA");
	if(config_has_property(arch_config, atributo)){
		int alfa = config_get_int_value(arch_config, atributo);
		config.alfa = alfa;
		logger_planificador(escribir_loguear,l_info,"%s: %f", atributo, config.alfa);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s. Default: %d", atributo, ALPHA);
		config.alfa = ALPHA;
	}

	free(atributo);
	atributo = NULL;

	//ESTIMACION_INICIAL
	atributo = strdup("ESTIMACION_INICIAL");
	if(config_has_property(arch_config, atributo)){
		config.estimacion_inicial = config_get_int_value(arch_config, atributo);
		logger_planificador(escribir_loguear,l_info,"%s: %d", atributo, config.estimacion_inicial);
	}
	else{
		logger_planificador(escribir_loguear,l_warning,"ERROR. No se pudo recuperar el atributo %s. Default: %d", atributo,ESTIMACION_INICIAL);
		config.estimacion_inicial = ESTIMACION_INICIAL;
	}

	free(atributo);
	atributo = NULL;

	//CLAVES_BLOQUEADAS
	atributo = strdup("CLAVES_BLOQUEADAS");
	claves_bloqueadas_config = config_get_array_value(arch_config, atributo);
	logger_planificador(escribir_loguear,l_info,"Bloqueando claves por archivo de config...");
	int i=0;

	while(claves_bloqueadas_config[i]!=NULL)
	{
		bloquear_clave(claves_bloqueadas_config[i],-1);
		free(claves_bloqueadas_config[i]);
		i++;
	}

	free(claves_bloqueadas_config);

	free(atributo);
	atributo = NULL;

	config_destroy(arch_config);

	return;
}

void configurar_signals(void)
{
	struct sigaction signal_struct;
	signal_struct.sa_handler = captura_sigpipe;
	signal_struct.sa_flags   = 0;

	sigemptyset(&signal_struct.sa_mask);

	sigaddset(&signal_struct.sa_mask, SIGPIPE);
    if (sigaction(SIGPIPE, &signal_struct, NULL) < 0)
    {
        fprintf(stderr, "sigaction error\n");
        exit(1);
    }

    sigaddset(&signal_struct.sa_mask, SIGINT);
    if (sigaction(SIGINT, &signal_struct, NULL) < 0)
    {
        fprintf(stderr, "sigaction error\n");
        exit(1);
    }

}

void captura_sigpipe(int signo)
{
    int i;

    if(signo == SIGINT)
    {

    	logger_planificador(escribir,l_error,"\nApretaste ctrl+c, por qué?, no hay porque\n");
    	terminar_planificador();
    	exit(EXIT_FAILURE);
    }
    else if(signo == SIGPIPE)
    {
    	logger_planificador(escribir,l_error,"\nSe perdió la conexión con el ESI, %d\n",esi_en_ejecucion->pid);
    }

}

t_list* esis_bloqueados_por_clave(char* recurso){

	bool is_esi_bloqueado_por_clave(t_pcb_esi* esi){
		return !strcmp(recurso, esi->clave_bloqueo);
	}

	return list_filter(esi_bloqueados, (void*) is_esi_bloqueado_por_clave);
}

void mostrar_esis_consola(t_list* lista_esi){

	void escribir_esi_bloqueado(t_pcb_esi* esi) {
		log_info(logger, "ESI bloqueado: %i", esi->pid);
		return;
	}

	if (list_size(lista_esi) > 0){
		list_iterate(lista_esi, (void*) escribir_esi_bloqueado);
	} else {
		log_info(logger, "NO se encontraron ESI bloqueados por la clave.");
	}

	return;
}

void logger_planificador(int tipo_esc, int tipo_log, const char* mensaje, ...){

	//Colores (reset,vacio,vacio,cian,verde,vacio,amarillo,rojo)
	static char *log_colors[8] = {"\x1b[0m","","","\x1b[36m", "\x1b[32m", "", "\x1b[33m", "\x1b[31m" };
	char *console_buffer=NULL;

	char *msj_salida = malloc(sizeof(char) * 256);

	//Captura los argumentos en una lista
	va_list args;
	va_start(args, mensaje);

	//Arma el mensaje formateado con sus argumentos en msj_salida.
	vsprintf(msj_salida, mensaje, args);

	//ESCRIBE POR PANTALLA
	if((tipo_esc == escribir) || (tipo_esc == escribir_loguear)){
		//printf("%s",msj_salida);
		//printf("\n");

		console_buffer = string_from_format("%s%s%s",log_colors[tipo_log],msj_salida, log_colors[0]);
		printf("%s",console_buffer);
		printf("\n");
		fflush(stdout);
		free(console_buffer);
	}

	//LOGUEA
	if((tipo_esc == loguear) || (tipo_esc == escribir_loguear)){

		if(tipo_log == l_info){
			log_info(logger, msj_salida);
		}
		else if(tipo_log == l_warning){
			log_warning(logger, msj_salida);
		}
		else if(tipo_log == l_error){
			log_error(logger, msj_salida);
		}
		else if(tipo_log == l_debug){
			log_debug(logger, msj_salida);
		}
		else if(tipo_log == l_trace){
			log_trace(logger, msj_salida);
		}
	}

	va_end(args);
	free(msj_salida);

	return;
}

char * sentencia_string(int sentencia)
{
	return sentencias[sentencia];
}
