/*
 * liberar_clave_atomica.c
 *
 *  Created on: 15 jul. 2018
 *      Author: utnso
 */

t_list * obtenerClavesDeMayorEspacioUtilizado(void);

void liberar_clave_atom(){
	aplicarAlgoritmoDeReemplazoYBorrar();
}

_Bool entradaAtomicaExistenteEnIndice(int nroEntrada) {
	_Bool entradaOcupada(t_indice_entrada * entrada) {
		return (entrada->numeroEntrada == nroEntrada && entrada->esAtomica);
	}
	return (list_any_satisfy(l_indice_entradas, (void *) entradaOcupada));
}

aplicarAlgoritmoDeReemplazoYBorrar() {
	printf("Aplicando algoritmo de reemplazo: %s\n", ALGORITMO_DE_REEMPLAZO);

	t_indice_entrada * indiceEntrada;

	if (strcmp(ALGORITMO_DE_REEMPLAZO, "CIRC") == 0) {
		numeroEntrada = 0;
		while(!entradaAtomicaExistenteEnIndice(numeroEntrada )){numeroEntrada++;};
		printf("El puntero fue posicionado en la entrada: %d\n", numeroEntrada);
			// TODO en lugar de puntero a char debe ser un char[40]
			char * clave = obtenerClaveExistenteEnEntrada(numeroEntrada ,
					l_indice_entradas);
			printf("Eliminando todas las entradas asociadas a la clave: %s...\n",
					clave);
			eliminarEntradasAsociadasAClave(clave);
	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "LRU") == 0) {
		// TODO: Buscar entrada con menor nro de operacion y reemplazarla

		t_list * listaAux = list_duplicate(l_indice_entradas);

		ordenarAscPorCodDeOperacion(listaAux);

		t_indice_entrada * entradaMenosUsada = list_get(listaAux, 0);

		// TODO en lugar de puntero a char debe ser un char[40]
		char * claveAReemplazar = obtenerClaveExistenteEnEntrada(
				entradaMenosUsada->numeroEntrada, listaAux);

		printf("Clave a ser reemplazada: %s\n", claveAReemplazar);

		// TODO: Eliminar los elementos de la lista destruida
		eliminarEntradasAsociadasAClave(claveAReemplazar);
		list_destroy(listaAux);


	} else if (strcmp(ALGORITMO_DE_REEMPLAZO, "BSU") == 0) {

		t_list * claves = obtenerClavesDeMayorEspacioUtilizado();
		char * claveAReemplazar = list_get(claves, 0);
		eliminarEntradasAsociadasAClave(claveAReemplazar);
	}
}
