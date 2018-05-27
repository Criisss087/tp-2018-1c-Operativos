/*
 * Funciones.c
 *
 *  Created on: 11 may. 2018
 *      Author: utnso
 */

t_esi_operacion_sin_puntero  *transformarSinPunteroYagregarpID(t_esi_operacion t, int id){
	t_esi_operacion_sin_puntero  *tsp;
	int keyword = t.keyword;
	char * valorp = NULL;
	char * clavep;
	char clave[40];
	int tam_valor;
	int pid;

	/*
	get 0
	set 1
	store 2
	*/

	switch(keyword){
	case 0:
		clavep = t.argumentos.GET.clave;
		break;
	case 1:
		clavep = t.argumentos.SET.clave;
		valorp = t.argumentos.SET.valor;
		break;
	case 2:
		clavep = t.argumentos.STORE.clave;
		break;
	default: break;
	}

	tsp->keyword = keyword;

	strncpy(tsp->clave, clavep, sizeof clave - 1);
	tsp->clave[strlen(clavep)-1] = '\0';

	tsp->pid = id;
	tsp->tam_valor = sizeof(t.argumentos.SET.valor);

	return tsp;
}

