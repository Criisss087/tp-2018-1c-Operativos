/*
 ============================================================================
 Name        : ESI.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <parsi/parser.h>

int main(int argc, char **argv){
	FILE * archivo_a_leer_por_el_ESI;
	char * linea_a_parsear =NULL;
	size_t direccion_de_la_linea_a_parsear = 0;
	ssize_t read = 1;
	archivo_a_leer_por_el_ESI = fopen(argv[1], "r");

	//while(!feof(archivo_a_leer_por_el_ESI)){
	//while (read != -1){
	//for(int i = 0;i<3;i++){
		while ((read = getline(&linea_a_parsear, &direccion_de_la_linea_a_parsear, archivo_a_leer_por_el_ESI)) != -1){

			printf("hasta acá llega\n");
			t_esi_operacion parsed = parse(linea_a_parsear);
			printf("hasta acá llega la primera vez\n");

			if(parsed.valido){
				switch(parsed.keyword){
				case GET:
					printf("parsed: %d, %s\n", parsed.keyword,parsed.argumentos.GET.clave);
					break;
				case SET:
					printf("parsed: %d, %s,%s\n", parsed.keyword,parsed.argumentos.SET.clave, parsed.argumentos.SET.valor);
					break;
				case STORE:
					printf("parsed: %d, %s\n", parsed.keyword,parsed.argumentos.STORE.clave);
					break;
				default:
					printf("error?...\n");
					break;
				}

				//destruir_operacion(parsed);


			}

		}//else printf("no leyo\n");
	//}
		if(linea_a_parsear){
					 free(linea_a_parsear);
				 }
		fclose(archivo_a_leer_por_el_ESI);
		return 0;
}


