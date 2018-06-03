/*
 * Funciones.c
 *
 *  Created on: 11 may. 2018
 *      Author: utnso
 */

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
	tsp->clave[strlen(clavep)-1] = '\0';

	//tsp->clave = strdup(clavep);
	return tsp;
}

