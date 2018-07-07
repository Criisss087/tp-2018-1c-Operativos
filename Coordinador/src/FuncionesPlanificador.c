t_status_clave_interno * buscar_clave(char * nombre_clave){
	//Buscar clave en lista interna de claves
			//1-Si no existe, devolver cod = 0
			//2-Si existe, mirar si tiene asociada una instancia
				//3-Si no tiene asociada instancia, simular, y devolver (no va a devolver valor)( cod =2)
				//4-Si tiene asociada instancia, consultarle a la misma
					//5-Si la inst esta caida, devolver 1 en cod
					//6-Si la inst no esta caida:
						//7-tiene valor: lo devuelve, devolver cod= 3
						//8-No tiene valor: devolver cod= 4

	t_instancia * getClaveByName(char * nombre){
		int mismoNombre(t_clave * clave){return string_equals_ignore_case(clave->clave, nombre);}
		return list_find(lista_claves,*mismoNombre);
	}

	t_status_clave_interno * st = malloc(sizeof(t_status_clave_interno));

	t_clave * clave = getClaveByName(nombre_clave);
	if (clave == NULL){
		//1
		st->cod = COORDINADOR_SIN_CLAVE;
		st->tamanio_valor = -1;
		st->tamanio_instancia_nombre = -1;
		st->nombre_instancia= NULL;
		st->valor = NULL;
	}
	else {
		//2
		if (clave->instancia == NULL){
			//3
			t_instancia * simulada = siguienteInstanciaSegunAlgoritmo(clave->clave, SIMULAR);
			st->cod=INSTANCIA_SIMULADA;
			st->nombre_instancia = strdup(simulada->nombre);
			st->tamanio_instancia_nombre = strlen(simulada->nombre);
			st->tamanio_valor = -1;
			st->valor = NULL;
		}
		else{
			//4
			if (chequearConectividadProceso(clave->instancia)==DESCONECTADO){
				//5
				st->cod = INSTANCIA_CAIDA;
				st->nombre_instancia = NULL;
				st->tamanio_instancia_nombre =-1;
				st->tamanio_valor=-1;
				st->valor=NULL;
			}else{
				//6
				//Send header
				t_content_header * header_consulta_valor_instancia = crear_cabecera_mensaje(coordinador,instancia,OBTENER_VALOR,clave->clave);
				int send_header = send(clave->instancia->socket,header_consulta_valor_instancia,sizeof(header_consulta_valor_instancia),0);
				//Send Clave
				char * clave_nombre = strdup(clave->clave);
				int send_clave = send(clave->instancia->socket, clave_nombre, strlen(clave_nombre),0);

				//recv header
				t_content_header * header_rta_inst = malloc(sizeof(t_content_header));
				int status_recv_header = recv(clave->instancia->socket,header_rta_inst,sizeof(t_content_header),0);

				//recv valor
				char * valor;
				if (header_rta_inst->cantidad_a_leer == -1){
					//8
					st->cod = INSTANCIA_SIN_CLAVE;
					st->nombre_instancia = strdup(clave->instancia->nombre);
					st->tamanio_instancia_nombre = strlen(clave->instancia->nombre);
					st->valor = NULL;
					st->tamanio_valor = -1;
				}
				else{
					//7
					valor = malloc(header_rta_inst->cantidad_a_leer);
					int status_recv_valor = recv(clave->instancia->socket,valor, header_rta_inst->cantidad_a_leer,0);

					st->cod = CORRECTO_CONSULTA_VALOR;
					st->nombre_instancia = strdup(clave->instancia->nombre);
					st->tamanio_instancia_nombre = strlen(clave->instancia->nombre);
					st->valor = strdup(valor);
					st->tamanio_valor = strlen(valor);
				}
			}
		}
	}

	return st;
}

void atender_comando_status(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO_ESCUCHA_PETICION_STATUS, &hints, &serverInfo);

	int listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	log_info(logger,"Socket de escucha planificador status creado %d", listenningSocket);

	// Las siguientes dos lineas sirven para no lockear el address
	int activado = 1;
	setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	log_info(logger, "Socket de escucha planificador status bindeado");
	freeaddrinfo(serverInfo);

	log_info(logger, "Escuchando...");
	listen(listenningSocket, BACKLOG);

	struct sockaddr_in addr;// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	log_info(logger, "Esperando conexion para petición...");
	int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);
	log_info(logger, "Conexión recibida - Accept: %d ",socketCliente);

	int socket_planif = socketCliente;

	while(1){
		//recv header
		t_content_header * header_consulta_valor = malloc(sizeof(t_content_header));
		int estado_recv_consulta = recv(socket_planif,header_consulta_valor,sizeof(t_content_header),0);
		//En el campo "cantidad_a_leer" me indica la long del nombre de la clave

		//recv del nombre de la clave
		char * nombre_clave = malloc(header_consulta_valor->cantidad_a_leer);
		int status_recv_clave = recv(socket_planif, nombre_clave, header_consulta_valor->cantidad_a_leer,0);

		t_status_clave_interno * st_clave_interno = buscar_clave(nombre_clave);

		//Devolver rta:

		t_content_header * header_rta_consulta_status = crear_cabecera_mensaje(coordinador,planificador,PLANIFICADOR_COORDINADOR_CMD_STATUS,sizeof(t_status_clave));
		int rdo_send_h = send(socket_planif, header_rta_consulta_status, sizeof(header_rta_consulta_status),0);
		//Mandar t_status_clave
		t_status_clave * st_clave = malloc(sizeof(t_status_clave));
		st_clave->cod = st_clave_interno->cod;
		st_clave->tamanio_instancia_nombre = st_clave_interno->tamanio_instancia_nombre;
		st_clave->tamanio_valor = st_clave_interno->tamanio_valor;

		int rdo_send_st_clave = send(socket_planif, st_clave, sizeof(t_status_clave),0);

		//Ahora mando valor y nombre instancia, en ese orden.
		int rdo_send_valor = send(socket_planif, st_clave_interno->valor, st_clave_interno->tamanio_valor, 0);
		int rdo_send_instancia = send(socket_planif, st_clave_interno->nombre_instancia, st_clave_interno->tamanio_instancia_nombre, 0);

		free(st_clave);
		free(st_clave_interno);
		free(header_consulta_valor);
		destruir_cabecera_mensaje(header_rta_consulta_status);
		free(nombre_clave);

	}
	close(socket_planif);
	log_destroy(logger);


}

void armar_hilo_planificador_status(){

	pthread_t hilo;
	pthread_create(&hilo,NULL,*atender_comando_status, NULL);
	pthread_detach(&hilo);

}
