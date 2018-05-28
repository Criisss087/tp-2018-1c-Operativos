# Shared Library para el TP!

### Instalación

Para usar la libreria, primero hay que instalarla:

Desde el repositorio del TP

* `git pull`					-> 	Obtener la ultima version

* `cd redis_lib`			-> 	Pararse en el repo de la biblioteca

* `sudo make install` 	-> 	Instala la biblioteca en el sistema



### Para usar 

En un .c/.h debería incluirse de la siguiente forma: 

`#include <redis_lib.h>`

Al compilar con gcc, linkear con -lredis_lib

### Desde eclipse

Ir a las Properties del proyecto (en el Project Explorer - la columna de la izquierda - la opción aparece dándole click derecho al proyecto), y dentro de la categoría C/C++ Build entrar a Settings, y ahí a Tool Settings.
Buscar GCC Linker > Libraries > Libraries. Notar que entre paréntesis dice -l, el parámetro de gcc que estamos buscando.
Darle click en el botón de +, y poner el nombre de la biblioteca sin el -l (en este caso, redis_lib).
Aceptar y buildear el proyecto.

### Para editarla:

Hay que importar el proyecto como cualquier otro, y se pueden editar los 2 src, para crear nuevas funciones, las declaraciones van en el .h, y las implementaciones en el .c
Para recompilarla, se puede hacer make all tanto desde /src como del directorio original

### Importante:

Una vez terminados los cambios, hay que desinstalarla y volver a instalar la nueva version, ya que nuestros proyectos leen el .h desde la ruta de includes del usuario (La misma donde estan los .h genericos tipo stdlib.h)

`sudo make uninstall`   -> 	desinstala la biblioteca

`sudo make install`	  	 -> 	vuelve a instalar la biblioteca

Recomiendo que cada vez que alguno haga un cambio avise al grupo para reinstalar.