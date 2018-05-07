t_esi_operacion_sin_puntero transformarSinPuntero(t_esi_operacion t){
	t_esi_operacion_sin_puntero tsp;
	int keyword = t.keyword;
	char * valorp = NULL;
	char * clavep;
	char valor[40];
	char clave[40];

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

	tsp.keyword = keyword;

	strncpy(tsp.clave, clavep, sizeof clave - 1);
	tsp.clave[strlen(clavep)-1] = '\0';

	if (keyword == 1 ) {
		strncpy(tsp.valor, valorp, sizeof valor - 1);
		tsp.valor[strlen(valorp)-1] = '\0';
	};

	return tsp;
}

