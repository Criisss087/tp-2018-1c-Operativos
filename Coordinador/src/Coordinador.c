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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#define MIPUERTO 5050    // Puerto al que conectarán los usuarios
#define BACKLOG 3     // Cuántas conexiones pendientes se mantienen en cola

int main(void)
{
    int sockfd, new_fd;  // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
    struct sockaddr_in my_addr;    // información sobre mi dirección
    struct sockaddr_in their_addr; // información sobre la dirección del cliente
    int sin_size;
    int yes=1;
//agrego este texto para hacer pruebas con github
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        /*if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }*/ //para liberar un puerto que estuvo usado?)

        my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
        my_addr.sin_port = htons(MIPUERTO);     // short, Ordenación de bytes de la red
        my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
        memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

        if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1) {
            perror("bind");
            exit(1);
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }

        while(1) {  // main accept() loop
            sin_size = sizeof(struct sockaddr_in);
            if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
                perror("accept");
                continue;
            }
            printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
            if (!fork()) { // Este es el proceso hijo
                close(sockfd); // El hijo no necesita este descriptor
                if (send(new_fd, "Hello, world!\n", 14, 0) == -1)
                    perror("send");
                close(new_fd);
                exit(0);
            }
            close(new_fd);  // El proceso padre no lo necesita
        }

        return 0;
    }
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
