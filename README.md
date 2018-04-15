# tp-2018-1c-La-Orden-del-Socket

Después de primer charla
  Resumen mega general:

Existirán 4 procesos distintos.
Coordinador,
Instancia (1 o más),
Planificador,
ESI (0, 1 o más)

Entendemos que:
Coordinador actuará de servidor para las Instancias, los ESIs y el Planificador.
Planificador actuará de servidor para los ESIs y como cliente del Coordinador. Como es cliente del Coordinador, el Coordinador debe levantarse primero que el Planificador.

ESI:
  Se levantan por consola por el usuario, leen un scritp (archivo) de a una línea por vez. Lo hace después de que el planificador le avise que siga (le avisa que puede continuar, que lea la próxima línea). Lee la línea, parsea la misma (la traduce para entender la sentencia) y le manda la misma al Coordinador. El coordinador una vez que termina le avisa del exito o fracaso de la operación al ESI.
    
Planificador:
  Es el que planifica :B

  Planifica según el algoritmo que se explicite en el archivo de configuración.
  
  Una vez levantado facilita al usuario mediante una consola varias operaciones (pausar, continuar, bloquear ESI, matar ESI, etc..)
  
Coordinador:
  Distribuye las operaciones (que recibe de los ESIs) entre las instancias.
  
Instancia:
  Proceso encargado del almacenamiento de los datos.
  

PRIMER ENCARE - CHECKPOINT 1
-Creacion de todos los procesos
-Desarrollar comunicación simple entre los procesos para propagar un mensaje por cada conexón
-Implementar consola del planificador sin funcionalidades

Armamos como servidor el Coordinador únicamente. El resto de los procesos se conectan a él una vez levantados. Toda la data de IP's, puertos y etc se hardcodea por ahora. Para eso está el header Utilidades, seguramente vuele en un futuro cercano.
Para la comunicación pensamos en simplemente que cada cliente pueda mandar mensaje al servidor (Coordinador) y que este lo devuelva a todos los demás. Para esto nos juntamos el sábado, ver si todos coincidimos o no, e implementarlo.
