/*
 ============================================================================
 Name        : Modificar_config.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char **argv)
{

	t_config * config = NULL;
	char property[255], value[255];

	config = config_create(argv[1]);

	FILE* file = fopen(argv[1], "r");
	struct stat stat_file;
	stat(argv[1], &stat_file);

	char* buffer = calloc(1, stat_file.st_size + 1);
	fread(buffer, stat_file.st_size, 1, file);
	printf("Estado actual del archivo: \n\n");
	printf("%s",buffer);
	fclose(file);
	free(buffer);

	printf("\n");

	printf("\nIngrese la clave a modificar: ");
	scanf("%s",property);
	printf("\n\n");

	if(config_has_property(config,property))
	{
		printf("Ingrese el nuevo valor: ");
		scanf("%s",value);

		printf("\n\n");
		printf("Valor antiguo: %s \n",config_get_string_value(config,property));

		config_set_value(config,property,value);
		printf("Valor nuevo: %s \n",config_get_string_value(config,property));

		config_save(config);

	}
	else{
		printf("\nNo existe la propiedad %s en el archivo de configuración %s",property,argv[1]);
	}

	config_destroy(config);
	printf("\nGracias, vuelva prontos\n\n");
	return EXIT_SUCCESS;
}
