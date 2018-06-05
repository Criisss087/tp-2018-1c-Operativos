mkdir  Instancia/src/valgrind/

rm a.out

gcc Instancia/src/Instancia.c -lpthread -lcommons -lparsi -lredis_lib

valgrind --leak-check=yes --log-file=Instancia/src/valgrind/valgrind.log ./a.out