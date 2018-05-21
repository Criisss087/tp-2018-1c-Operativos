Linkear con -lredis_lib

Para usarla en un .c/.h debería incluirse de la siguiente forma: 

#include <redis_lib.h>

Desde eclipse

    Ir a las Properties del proyecto (en el Project Explorer - la columna de la izquierda - la opción aparece dándole click derecho al proyecto), y dentro de la categoría C/C++ Build entrar a Settings, y ahí a Tool Settings.
    Buscar GCC Linker > Libraries > Libraries. Notar que entre paréntesis dice -l, el parámetro de gcc que estamos buscando.
    Darle click en el botón de +, y poner el nombre de la biblioteca sin el -l (en este caso, commons).
    Aceptar y buildear el proyecto.
