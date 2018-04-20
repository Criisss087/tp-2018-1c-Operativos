/*
 ============================================================================
 Name        : Coordinador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int main(void) {
	puts(""); /* prints  */
	return EXIT_SUCCESS;
}

/*

 Funciones que a mi parecer se van a utilizar:
 ---------------------------------------------

 inicializar() ----> 1. leerArchivoDeConfiguracion();
                     2. inicializarEstructurasAdministrativas();
 esperarSolicitud(); EN CASO DE QUE LA SOLICITUD QUE LLEGA ES DE PARTE DE UN ESI
                     - si es aceptada lanza un hilo encargado de atender la conexion
                     - si no es aceptada, avisa al planificador
                     EN CASO DE QUE LA SOLICITUD QUE LLEGA ES DE PARTE DE UNA INSTANCIA
                     - le provee la configuracion de tamaños de la cantidad y el tamaño de las entradas
 procesarSolicitud(); esto es el hilo para una solicitud de ESI---ejecuta una instancia segun un algoritmo de distribucion
                      si la instancia no esta disponible, reordena y elige una de las instancias restantes
                      debe retornar un mensaje informando el resultado de la operacion
 registrarEjecucion(); en el log de operaciones

void *elegirYutilizarInstancia(void*); // o gestionarInstancia (algoritmo de distribucion)



LA FUNCION ESPERARSOLICITUD() es la que determina que el coordinador es un servidor, ya que va a utilizar sockets de escucha.


 Para trabajar con hilos:
 ------------------------
 pthread_create(direccion de memoria del thread que se va a crear, NULL, nombre de la funcion que va a usar, direccion de memoria del argumento que va a recibir);
 pthread_join(thread que se creo, donde se guarda el resultado tras ejecutar la funcion);


 Ejemplo de como debe quedar ?) :
 -------------------------------

 int main(){

     inicializar();

     if(esperarSolicitud() es un ESI y la conexion es aceptada){

 	 pthread_t  procesarSolicitud; //nombre del thread
 	 pthread_create(&procesarSolicitud, NULL, elegirYUtilizarInstancia, NULL)//por ahora relleno con NULL;
 	 pthread_join(procesarSolicitud, NULL);

 	 registrarEjecucion();

 	 }else{
 	       --avisar al planificador--
 	       }

 	 if(esperarSolicitud() es una instancia y la conexion es aceptada){
 	 	 proveerRecursosParaLasEntradas();
 	 	 //¿Se hace otro hilo?
 	 }
 }

 */
