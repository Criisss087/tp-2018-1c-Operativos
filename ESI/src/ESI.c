/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include "ESI.h"

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
		finalizar_esi();
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
		finalizar_esi();
		printf("Error al intentar conectar al planificador\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(serverInfoPlanif);

	printf("Conectado al servidor planificador: %d \n",resultado_conexion_planificador);

	return serverPlanif;
}

void finalizar_esi(void)
{
	if(confirmacion != NULL){
		free(confirmacion);
	}


	if(linea_a_parsear){
		free(linea_a_parsear);
	}

	if(IP_COORDINADOR!=NULL)
	{
		free(IP_COORDINADOR);
		IP_COORDINADOR = NULL;
	}

	if(PUERTO_COORDINADOR!=NULL)
	{
		free(PUERTO_COORDINADOR);
		PUERTO_COORDINADOR = NULL;
	}


	if(IP_PLANIFICADOR!=NULL)
	{
		free(IP_PLANIFICADOR);
		IP_PLANIFICADOR = NULL;
	}


	if(PUERTO_PLANIFICADOR!=NULL)
	{
		free(PUERTO_PLANIFICADOR);
		PUERTO_PLANIFICADOR = NULL;
	}


	if(archivo_a_leer_por_el_ESI != NULL)
	{
		fclose(archivo_a_leer_por_el_ESI);
	}

	close(serverCoord);
	close(serverPlanif);

}

void mostrar_header(t_content_header * header){
	printf("El contenido del header es -> ");

	if(header->proceso_origen == 1 || header->proceso_origen == 2 || header->proceso_origen == 3 || header->proceso_origen == 4){
		printf("Origen: %d, Receptor: %d, Operación: %d, Cantidad: %d \n",header->proceso_origen,header->proceso_receptor,header->operacion,header->cantidad_a_leer);
	}else{
		printf("Recibi cualquier cosa, cierro por error desde el proceso origen\n");
		free(header);
		finalizar_esi();
		exit(EXIT_FAILURE);
	}

}

void mostrar_sentencia(t_esi_operacion_sin_puntero * sentencia, char * valor)
{
	char * keyword = NULL;

	switch(sentencia->keyword)
	{
		case GET:
			keyword = strdup("GET");
			break;
		case SET:
			keyword = strdup("SET");
			break;
		case STORE:
			keyword = strdup("STORE");
			break;
		default:break;

	}

	if(sentencia->keyword == SET)
	{
		printf("EJECUTAR: %s %s %s\n",keyword, sentencia->clave,valor);
	}
	else
	{
		printf("EJECUTAR: %s %s\n",keyword, sentencia->clave);
	}

	free(keyword);
	keyword = NULL;


}

t_esi_operacion_sin_puntero  *transformarSinPunteroYagregarpID(t_esi_operacion t, int id){
	char * valorp = NULL;
	char * clavep = NULL;
	char clave[40];

	t_esi_operacion_sin_puntero  *tsp = malloc(sizeof(t_esi_operacion_sin_puntero));
	memset(tsp, 0, sizeof(t_esi_operacion_sin_puntero));
	tsp->keyword = t.keyword;
	tsp->pid = id;

	int tam_valor;
	/*
	get 0
	set 1
	store 2
	*/

	switch(t.keyword){
	case 0:
		clavep = strdup(t.argumentos.GET.clave);
		tam_valor = 0;
		break;
	case 1:
		clavep = strdup(t.argumentos.SET.clave);
		valorp = strdup(t.argumentos.SET.valor);
		tsp->tam_valor = strlen(valorp);
		break;
	case 2:
		clavep = strdup(t.argumentos.STORE.clave);
		tam_valor = 0;
		break;
	default: break;
	}

	strncpy(tsp->clave, clavep, sizeof (clave) - 1);
	tsp->clave[strlen(clavep)] = '\0';

	if(clavep!=NULL)
	{
		free(clavep);
		clavep = NULL;
	}

	if(valorp!=NULL)
	{
		free(valorp);
		valorp = NULL;
	}


	return tsp;
}

void cargar_archivo_de_config(char *path){
	if (path != NULL){

		t_config * config_file = config_create(path);

		if (config_has_property(config_file,ARCH_CONFIG_PUERTO_COORD)){
			PUERTO_COORDINADOR = strdup(config_get_string_value(config_file, ARCH_CONFIG_PUERTO_COORD));
		}

		if (config_has_property(config_file,ARCH_CONFIG_PUERTO_PLANIF)){
			PUERTO_PLANIFICADOR = strdup(config_get_string_value(config_file, ARCH_CONFIG_PUERTO_PLANIF));
		}

		if (config_has_property(config_file,ARCH_CONFIG_IP_COORD)){
			IP_COORDINADOR = strdup(config_get_string_value(config_file, ARCH_CONFIG_IP_COORD));
		}

		if (config_has_property(config_file,ARCH_CONFIG_IP_PLANIF)){
			IP_PLANIFICADOR = strdup(config_get_string_value(config_file, ARCH_CONFIG_IP_PLANIF));
		}

		config_destroy(config_file);
	}
	else {
		printf("Error al cargar el archivo de configuracion \n");
		exit(1);
	}
}

void recibir_orden_planif_para_comenzar(t_content_header * header){

	header  = malloc(sizeof(t_content_header));
	int read_size = recv(serverPlanif, header, sizeof(t_content_header), (int)NULL);

	if(read_size < 0){
		printf("Error en el recv del header de la orden del planificador. \n");
		finalizar_esi();
		free(header);
		exit(EXIT_FAILURE);
	}

	//mostrar_header(header);

	confirmacion = malloc(sizeof(t_confirmacion_sentencia));
	read_size = recv(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia), 0);

	if(read_size < 0){
		printf("Error en el recv de la confirmacion del planificador. \n");
		printf("read size es %d = \n", read_size);
		finalizar_esi();
		free(header);
		exit(EXIT_FAILURE);
	}

	printf("Esperando orden del planificador para comenzar...\n");

	switch(header->operacion){
		case RECIBIR_ORDEN_EJECUCION:
			printf("Orden recibida, comienzo. \n");
			break;
		case RECIBIR_KILL_PLANIF:
			printf("Me mataron desde el planificador!. \n");
			finalizar_esi();
			free(header);
			exit(EXIT_FAILURE);
			break;
		default:
			break;
	}

	free(header);
}

void abrir_script(char *path){

	if(path != NULL)
	{
		archivo_a_leer_por_el_ESI = fopen(path, "r");
	}

	if(archivo_a_leer_por_el_ESI == NULL){
		printf("Error al intentar abrir el archivo a leer.\n");
		finalizar_esi();
		exit(EXIT_FAILURE);
	}
}

void enviar_linea_parseada_coordinador(t_content_header * header, t_esi_operacion parsed){

	t_esi_operacion_sin_puntero  *parse_sin_punteros = NULL;
	parse_sin_punteros = transformarSinPunteroYagregarpID(parsed, confirmacion->pid);

	printf("Enviando linea parseada al coordinador... \n");

	header = crear_cabecera_mensaje(esi, coordinador, ENVIAR_SENTENCIA_COORD, sizeof(t_esi_operacion_sin_puntero));
	int resultado = send(serverCoord, header, sizeof(t_content_header), 0);

	if(resultado < 0){
		printf("Error en el send del header al coordinador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	resultado = send(serverCoord, parse_sin_punteros, sizeof(t_esi_operacion_sin_puntero),0);

	if(resultado < 0){
		printf("Error en el send del parsed sin punteros al coordinador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	//mostrar_header(header);

	if(parse_sin_punteros->keyword == SET){
		printf("Enviando valor de la clave necesaria para el coordinador, la cual es:  %s\n", parsed.argumentos.SET.valor);
		int envio_valor_clave = send(serverCoord, parsed.argumentos.SET.valor , strlen(parsed.argumentos.SET.valor),0);

		if(envio_valor_clave < 0){
			printf("Error en el send del valor de la clave al coordinador. \n");
			finalizar_esi();
			destruir_cabecera_mensaje(header);
			exit(EXIT_FAILURE);
		}
	}

	mostrar_sentencia(parse_sin_punteros, parsed.argumentos.SET.valor);

	free(parse_sin_punteros);

	destruir_cabecera_mensaje(header);
}

void recibir_respuesta_coordinador(t_content_header * header){

	printf("Recibiendo respuesta del coordinador...\n");

	header = malloc(sizeof(t_content_header));
	int resultado = recv(serverCoord, header, sizeof(t_content_header),0);

	if(resultado < 0){
		printf("Error en el recv del header de la rta del coordinador. \n");
		free(header);
		finalizar_esi();
		exit(EXIT_FAILURE);
	}

	//mostrar_header(header);

	respuesta_coordinador *respuesta_coordinador = malloc (sizeof(respuesta_coordinador));
	resultado = recv(serverCoord, respuesta_coordinador, sizeof(respuesta_coordinador),0);

	if(resultado < 0){
		printf("Error en el recv con el contenido de la rta del coordinador. \n");
		free(respuesta_coordinador);
		free(header);
		finalizar_esi();
		exit(EXIT_FAILURE);
	}

	//SI recibo orden de abortar
	if(header->operacion == RECIBIR_RESULTADO_SENTENCIA_COORD && respuesta_coordinador->resultado_del_parseado == ABORTAR){

		printf("Recibi orden de aborto, aviso al planificador y fin de ejecucion. \n");
		free(header);
		free(respuesta_coordinador);
		abortar_esi();

	}

	if(respuesta_coordinador->resultado_del_parseado < 0){
		printf("El coordinador me envio basura, cierro por que si no se arrastra el error.\n");

		free(respuesta_coordinador);
		free(header);
		destruir_operacion(parsed);

		finalizar_esi();
		exit(EXIT_FAILURE);
	}

	printf("La respuesta que recibi del coordinador es: %d \n", respuesta_coordinador->resultado_del_parseado);

	if(header->operacion == RECIBIR_RESULTADO_SENTENCIA_COORD && respuesta_coordinador->resultado_del_parseado != ABORTAR){
		confirmacion->resultado = respuesta_coordinador->resultado_del_parseado;
		free(respuesta_coordinador);
	}

	free(header);
}

void enviar_al_planificador_la_rta_del_coordinador(t_content_header * header){

	printf("Lo que envío al planificador es: %d \n", confirmacion->resultado);
	printf("Enviando al planificador la respuesta del coordinador...\n");

	header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF, sizeof(t_content_header));

	//mostrar_header(header);

	int enviar_rdo_planif = send(serverPlanif, header, sizeof(t_content_header),0);

	if(enviar_rdo_planif < 0){
		printf("Error en el send del header de enviar_rdo_planif al coordinador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	enviar_rdo_planif = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

	if(enviar_rdo_planif < 0){
		printf("Error en el send del contenido de enviar_rdo_planif al coordinador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	destruir_cabecera_mensaje(header);

}

void esperar_orden_planificador_para_finalizar(int esperar){

	t_content_header* content_header;

	if(esperar){

		printf("Esperando orden del planificador para finalizar...\n");

		content_header = malloc(sizeof(t_content_header));
		int read_size = recv(serverPlanif, content_header, sizeof(t_content_header), (int)NULL);

		if(read_size < 0){
			printf("Error en el recv de la orden del planificador. \n");
			finalizar_esi();
			free(content_header);
			exit(EXIT_FAILURE);
		}


		confirmacion = malloc(sizeof(t_confirmacion_sentencia));
		read_size = recv(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia), 0);

		if(read_size < 0){
			printf("Error en el recv del contenido de la orden del planificador. \n");
			finalizar_esi();
			free(content_header);
			exit(EXIT_FAILURE);
		}

		if(content_header->operacion == RECIBIR_ORDEN_EJECUCION)
			esperar = 0;

		destruir_cabecera_mensaje(content_header);
	}

	if(!esperar){
		printf("Finaliza el proceso \n");

		content_header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF , sizeof(t_confirmacion_sentencia));
		//mostrar_header(content_header);

		//Le aviso al planificador que termine de leer el archivo
		int finalice_lectura = send(serverPlanif, content_header, sizeof(t_content_header),0);

		if(finalice_lectura < 0){
			printf("Error en el send del header de finalice_lectura al planificador. \n");
			finalizar_esi();
			exit(EXIT_FAILURE);
		}

		confirmacion->resultado = LISTO;

		finalice_lectura = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

		if(finalice_lectura < 0){
			printf("Error en el send del contenido de finalice_lectura al planificador. \n");
			finalizar_esi();
			exit(EXIT_FAILURE);
		}
		destruir_cabecera_mensaje(content_header);
		printf("Fin de ejecucion por alcanzar el fin del archivo \n");
	}

}

void abortar_esi(void)
{
	t_content_header * header = NULL;

	//Aviso al planificador que recibi orden de abortar
	header = crear_cabecera_mensaje(esi, planificador, ENVIAR_RESULTADO_PLANIF, sizeof(t_content_header));
	int enviar_aviso_abortar = send(serverPlanif, header, sizeof(t_content_header),0);

	if(enviar_aviso_abortar < 0){
		printf("Error en el send del header de enviar_aviso_abortar al planificador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	confirmacion->resultado = ABORTAR;
	enviar_aviso_abortar = send(serverPlanif, confirmacion, sizeof(t_confirmacion_sentencia),0);

	if(enviar_aviso_abortar < 0){
		printf("Error en el send del contenido de enviar_aviso_abortar al planificador. \n");
		finalizar_esi();
		destruir_cabecera_mensaje(header);
		exit(EXIT_FAILURE);
	}

	//Cierro!
	destruir_cabecera_mensaje(header);
	destruir_operacion(parsed);

	finalizar_esi();
	exit(EXIT_FAILURE);

	return;
}

void configurar_signals(void)
{
	struct sigaction signal_struct;
	signal_struct.sa_handler = captura_sigint;
	signal_struct.sa_flags   = 0;

	sigemptyset(&signal_struct.sa_mask);

    sigaddset(&signal_struct.sa_mask, SIGINT);
    if (sigaction(SIGINT, &signal_struct, NULL) < 0)
    {
        fprintf(stderr, "sigaction error\n");
        exit(1);
    }

}

void captura_sigint(int signo)
{
    if(signo == SIGINT)
    {

    	printf("\nApretaste ctrl+c, por qué?, no hay porque\n");
    	destruir_operacion(parsed);
    	finalizar_esi();
    	exit(EXIT_FAILURE);
    }

}

int main(int argc, char **argv){

	linea_a_parsear = NULL;
	size_t direccion_de_la_linea_a_parsear = 0;
	ssize_t read = 0;
	t_content_header *content_header;

	//Obtengo los datos del archivo de configuracion
	cargar_archivo_de_config(argv[1]);
	configurar_signals();

	printf("Iniciando conexion a servidores... \n");
	serverCoord = conectar_coordinador(IP_COORDINADOR, PUERTO_COORDINADOR);
	serverPlanif = conectar_planificador(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);
	printf("\n");

	abrir_script(argv[2]);

	//Leo el archivo y parseo
	while(!feof(archivo_a_leer_por_el_ESI)){

		recibir_orden_planif_para_comenzar(content_header);

		printf("\n");

		//Chequeo si debo ejecutar linea anterior o no
		if (read != -1){ //sentencia_actual != -1){

			//Si la confirmacion es distinto de 0, mantiene la linea aterior en linea_a_parsear
			if(confirmacion->ejec_anterior == 0){
				free(linea_a_parsear);
				linea_a_parsear = NULL;
				read = getline(&linea_a_parsear, &direccion_de_la_linea_a_parsear, archivo_a_leer_por_el_ESI);

			}

			if(read == -1){
				//Se llegó al fin del archivo,
				//hay que mandar solamente el mensaje de finalizar, sin esperar confirmacion
				fin_archivo++;
				break;
			}

			printf("Ejecutar línea anterior? : %d\n", confirmacion->ejec_anterior);
			printf("\n");

			parsed = parse(linea_a_parsear);

			if(parsed.valido){
				enviar_linea_parseada_coordinador(content_header,parsed);
				printf("\n");
				recibir_respuesta_coordinador(content_header);
				printf("\n");
				enviar_al_planificador_la_rta_del_coordinador(content_header);
				free(confirmacion);
				destruir_operacion(parsed);

				printf("\n");
				printf("FIN LINEA\n");
			}//if parsed valido
			else{
				printf("La linea parseada no es válida, se aborta el ESI\n");
				abortar_esi();
			}

		}//if sentencia_actual

	}//while...


	if(fin_archivo)
	{
		esperar = 0;
	}
	else
	{
		esperar++;
	}

	printf("\n");
	esperar_orden_planificador_para_finalizar(esperar);

	finalizar_esi();

	return 0;

}
