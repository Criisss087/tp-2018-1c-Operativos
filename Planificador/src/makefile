RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=%.o)


all: 
	$(CC) -o "Planificador"  Planificador.c   -lcommons -lpthread -lreadline -lredis_lib





.PHONY: all clean install uninstall	