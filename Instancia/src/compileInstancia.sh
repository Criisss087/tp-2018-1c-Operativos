mkdir  valgrind/

rm instancia

gcc Instancia.c -lpthread -lcommons -lparsi -lredis_lib -o instancia
