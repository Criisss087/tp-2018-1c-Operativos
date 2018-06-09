mkdir  Instancia/src/valgrind/

rm instancia

gcc Instancia/src/Instancia.c -lpthread -lcommons -lparsi -lredis_lib -o instancia

valgrind --leak-check=yes --log-file=Instancia/src/valgrind/valgrind.log ./instancia Instancia/src/config.txt
