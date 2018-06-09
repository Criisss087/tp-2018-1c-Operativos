/*
 * Funciones.c
 *
 *  Created on: 11 may. 2018
 *      Author: utnso
 */

#include <commons/config.h>

//Configuracion
char * IP_COORDINADOR;
char * PUERTO_COORDINADOR;
char * IP_PLANIFICADOR;
char * PUERTO_PLANIFICADOR;

//Nombres para el archivo de configuracion
#define ARCH_CONFIG_PUERTO_COORD "PUERTO COORDINADOR"
#define ARCH_CONFIG_PUERTO_PLANIF "PUERTO PLANIFICADOR"
#define ARCH_CONFIG_IP_COORD "IP COORDINADOR"
#define ARCH_CONFIG_IP_PLANIF "IP PLANIFICADOR"

t_esi_operacion_sin_puntero  *transformarSinPunteroYagregarpID(t_esi_operacion t, int id){
	char * valorp = NULL;
	char * clavep = NULL;
	char clave[40];

	t_esi_operacion_sin_puntero  *tsp = malloc(sizeof(t_esi_operacion_sin_puntero));
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
