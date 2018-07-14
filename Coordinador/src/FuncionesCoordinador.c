/*
 * Utilidades.c
 *
 *  Created on: 28 abr. 2018
 *      Author: utnso
 */

#include "funcionesInstancia.c"
#include "Utilidades.h"


void crear_hilo_conexion(int socket, void*funcion_a_ejecutar(int)){
	pthread_t hilo;
	pthread_create(&hilo,NULL,(void*)funcion_a_ejecutar,(void*)socket);
	pthread_detach(hilo);
}

void configurar_signals();

void cargarArchivoConfiguracion(char * path){
	logger_coordinador(escribir_loguear, l_info, "Cargando archivo de configuración...\n");

	t_config * config_file = config_create(path);
	char * algo_dist;
	logger_coordinador(escribir_loguear, l_info, "Se cargó archivo? : %d\n",config_file!=NULL);

	if (config_has_property(config_file,ARCH_CONFIG_ALGORITMO_DISTRIBUCION)){
		algo_dist = strdup(config_get_string_value(config_file, ARCH_CONFIG_ALGORITMO_DISTRIBUCION));
		if (string_equals_ignore_case(algo_dist, "EL")){
			//Equitative Load
			ALGORITMO_DISTRIBUCION = EQUITATIVE_LOAD;
			logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Algoritmo de distribución': %s - %d\n", algo_dist, ALGORITMO_DISTRIBUCION);
		}
		else if (string_equals_ignore_case(algo_dist, "KE")){
			//Key Explicit
			ALGORITMO_DISTRIBUCION = KEY_EXPLICIT;
			logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Algoritmo de distribución': %s - %d\n", algo_dist, ALGORITMO_DISTRIBUCION);
		}
		else if (string_equals_ignore_case(algo_dist, "LSU")){
			//Least Space Used
			ALGORITMO_DISTRIBUCION = LEAST_SPACE_USED;
			logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Algoritmo de distribución': %s - %d\n", algo_dist, ALGORITMO_DISTRIBUCION);
		}else {
			logger_coordinador(escribir_loguear, l_warning, "No se reconoció el algoritmo especificado en el archivo de configuración. Se setea por default EL.\n");
		}

	}else logger_coordinador(escribir_loguear, l_warning, "No se reconoció el algoritmo especificado en el archivo de configuración. Se setea por default EL.\n");

	if (config_has_property(config_file,ARCH_CONFIG_CANTIDAD_ENTRADAS)){
		CANT_MAX_ENTRADAS = config_get_int_value(config_file, ARCH_CONFIG_CANTIDAD_ENTRADAS);
		logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Cantidad de entradas': %d \n", CANT_MAX_ENTRADAS);
	}else logger_coordinador(escribir_loguear, l_warning, "No se encontró cantidad de entradas especificadas en el archivo de configuración. Se setea por default 50.\n");

	if (config_has_property(config_file,ARCH_CONFIG_PUERTO)){
		PUERTO = strdup(config_get_string_value(config_file, ARCH_CONFIG_PUERTO));
		logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Puerto': %s\n",PUERTO);
	}else logger_coordinador(escribir_loguear, l_warning, "No se encontró puerto especificado en el archivo de configuración. Se setea por default 8888.\n");

	if (config_has_property(config_file,ARCH_CONFIG_TAMANIO_ENTRADAS)){
		TAMANIO_ENTRADAS = config_get_int_value(config_file, ARCH_CONFIG_TAMANIO_ENTRADAS);
		logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Tamaño de entradas': %d\n",TAMANIO_ENTRADAS);
	}else logger_coordinador(escribir_loguear, l_warning, "No se encontró tamaño de entradas especificado en el archivo de configuración. Se setea por default 300.\n");

	if (config_has_property(config_file,ARCH_CONFIG_RETARDO)){
		RETARDO = config_get_int_value(config_file, ARCH_CONFIG_RETARDO);
		logger_coordinador(escribir_loguear, l_info, "Se obtuvo configuración 'Retardo': %d\n",RETARDO);
	}else logger_coordinador(escribir_loguear, l_warning, "No se encontró retardo especificado en el archivo de configuración. Se setea por default 0.\n");

	free(algo_dist);
	config_destroy(config_file);
}

void inicializarLogger(){
	logger = log_create("log_coordinador.txt","Coordinador",false, LOG_LEVEL_INFO);
	logger_operaciones = log_create("log_operaciones_coordinador.txt","Coordinador",false, LOG_LEVEL_INFO);
}
void seteosIniciales(char *path){
	inicializarLogger();

	if (path != NULL){
		cargarArchivoConfiguracion(path);
		}
	else {
		logger_coordinador(escribir_loguear, l_warning, "Configuraciones cargadas por defecto\n");
	}

	lista_instancias = list_create();
	lista_claves = list_create();
	indice_actual_lista = -1; //TODO usar la funcion list_size para ver si mostrar o no el error

	rta1 = malloc(sizeof(int));
	configurar_signals();
}

struct addrinfo* crear_addrinfo(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP
	getaddrinfo(IP, PUERTO, &hints, &serverInfo);
	return serverInfo;
}

int listenningSocket(char * puerto)
{
	struct sockaddr_in dir_sock;

	//Convierto el string a INT para htons
	unsigned int puerto_i = atoi(puerto);

	dir_sock.sin_family = AF_INET;
	dir_sock.sin_addr.s_addr = INADDR_ANY;
	dir_sock.sin_port = htons(puerto_i);

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		return -1;
	}

	int activado = 1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(server_socket, (void*)&dir_sock, sizeof(dir_sock)) != 0)
	{
		return -1;
	}


	listen(server_socket, BACKLOG);

	return server_socket;

}

t_esi_operacion_sin_puntero * armar_esi_operacion_sin_puntero(t_sentencia * sentencia){
	t_esi_operacion_sin_puntero * op_sin_punt = malloc(sizeof(t_esi_operacion_sin_puntero));
	strncpy(op_sin_punt->clave, sentencia->clave,40);
	op_sin_punt->keyword = sentencia->keyword;
	op_sin_punt->tam_valor = strlen(sentencia->valor)+1;
	op_sin_punt->pid = sentencia->pid;
	return op_sin_punt;
}

t_sentencia * armar_sentencia(t_esi_operacion_sin_puntero * op_sin_punt, char * valor){

	t_sentencia * sentencia_con_punteros = malloc(sizeof(t_sentencia));
	strncpy(sentencia_con_punteros->clave, op_sin_punt->clave,40);
	sentencia_con_punteros->valor = strdup(valor);
	sentencia_con_punteros->keyword = op_sin_punt->keyword;
	sentencia_con_punteros->pid = op_sin_punt->pid;
	return sentencia_con_punteros;

}

void log_operacion_esi(t_sentencia * sentencia, t_log * logger){
	char * keyw;
	if (sentencia->keyword == GET) {keyw = strdup("GET");}
	else if(sentencia->keyword == SET) {keyw = strdup("SET");}
	else {keyw = strdup("STORE");}

	logger_coordinador(loguear,l_esi, "ESI %d = %s %s %s \n",sentencia->pid,keyw,sentencia->clave,sentencia->valor);

	free(keyw);
}
void log_error_operacion_esi(t_sentencia * sentencia, int puedoEnviar){
	char * keyw;
	if (sentencia->keyword == GET) {keyw = strdup("GET");}
	else if(sentencia->keyword == SET) {keyw = strdup("SET");}
	else {keyw = strdup("STORE");}
	char * error;
	if (puedoEnviar == ABORTAR) {error = strdup("ABORTAR");}
	else {error = strdup("CLAVE BLOQUEADA");}

	logger_coordinador(loguear,l_esi, "ESI %d = %s %s %s - ERROR: %s \n",sentencia->pid,keyw,sentencia->clave,sentencia->valor,error);

	free(keyw);
	free(error);
}

void destruir_lista_instancia(t_instancia * instancia){
	free(instancia->nombre);
	free(instancia);
}

void destruir_lista_clave(t_clave * clave){
	free(clave);
}

void liberar_listas(){
	list_destroy_and_destroy_elements(lista_instancias, (void*)destruir_lista_instancia);
	list_destroy_and_destroy_elements(lista_claves, (void*)destruir_lista_clave);

}

void liberar_loggers(){
	log_destroy(logger);
	log_destroy(logger_operaciones);
}

void finalizar_coordinador(){
	liberar_listas();
	liberar_loggers();

	if(rta1 != NULL){
		free(rta1);
	}
}

void captura_sigpipe(int signo)
{
    int i;

    if(signo == SIGINT)
    {
    	logger_coordinador(escribir_loguear, l_warning,"\n Finalizando proceso...\n");
    	GLOBAL_SEGUIR = 0;
    	finalizar_coordinador();
    	exit(EXIT_FAILURE);
    }
    else if(signo == SIGPIPE)
    {
    	logger_coordinador(escribir_loguear, l_error,"\n Se desconectó un proceso al que se quizo enviar.\n");
    }

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
    	logger_coordinador(escribir_loguear, l_error,"\n SIGACTION error \n");
        //fprintf(stderr, "sigaction error\n");
        //exit(1);
    }

    sigaddset(&signal_struct.sa_mask, SIGINT);
    if (sigaction(SIGINT, &signal_struct, NULL) < 0)
    {
    	logger_coordinador(escribir_loguear, l_error,"\n SIGACTION error \n");
    	//fprintf(stderr, "sigaction error\n");
        //exit(1);
    }

}

int existeClave(char *nombre){
	bool mismaClave(t_clave * clave){return string_equals_ignore_case(clave->clave, nombre);}
	return list_any_satisfy(lista_claves, (void *)mismaClave);
}

void eliminarClave(char clave[40]){
	int pos = 0;
	int tam = list_size(lista_claves);
	t_clave * aux = NULL;
	for (int i = 0; tam>i;i++){
		aux = list_get(lista_claves,i);
		if (string_equals_ignore_case(aux->clave, clave)){
			pos = i;
			i = tam;
		}
	}
	list_remove(lista_claves,pos);
}

#include "FuncionesPlanificador.c"
