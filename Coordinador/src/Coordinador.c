/*
 ============================================================================
 Name        : Coordinador.c
 Author      : La Orden Del Socket
 Version     :
 Copyright   : Si nos copias nos desaprueban el coloquio
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


#define PUERTO "5050"
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024

int main()
{
    struct addrinfo hints;
    struct addrinfo *serverInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
    hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
    hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

    getaddrinfo(NULL, PUERTO, &hints, &serverInfo);

    int listenningSocket;
    listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    freeaddrinfo(serverInfo);

    listen(listenningSocket, BACKLOG);

    struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
    socklen_t addrlen = sizeof(addr);

    int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

    char package[PACKAGESIZE];
    int status = 1;		// Estructura que manjea el status de los recieve.

    printf("Cliente conectado. Esperando mensajes:\n");

    while (status != 0){
    	status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
    	if (status != 0) printf("%s", package);
   	}

    close(socketCliente);
    close(listenningSocket);

    return 0;


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
