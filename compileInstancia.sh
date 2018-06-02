mkdir  Instancia/src/valgrind/

gcc Instancia/src/Instancia.c -lpthread -lcommons -lparsi -lredis_lib


valgrind --leak-check=yes --log-file=Instancia/src/valgrind/valgrind.log ./a.out