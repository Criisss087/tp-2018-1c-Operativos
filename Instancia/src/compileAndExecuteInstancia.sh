mkdir  valgrind/

rm instancia

gcc Instancia.c -lpthread -lcommons -lparsi -lredis_lib -o instancia

valgrind --leak-check=yes --log-file=valgrind/valgrind.log ./instancia config.txt
