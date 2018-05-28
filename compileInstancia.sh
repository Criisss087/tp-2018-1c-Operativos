mkdir  Instancia/src/valgrind/

gcc Instancia/src/Instancia.c -lpthread -lcommons -lparsi


valgrind --leak-check=yes --log-file=Instancia/src/valgrind/valgrind.log ./a.out