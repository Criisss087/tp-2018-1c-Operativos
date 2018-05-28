RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=%.o)

# Clean and compile .so
all: 
	$(CC) -c -Wall -Werror -fPIC  $(C_SRCS)
	$(CC) -shared -o libredis_lib.so $(OBJS)

# Add debug parameters and compile
debug: CC += -DDEBUG -g
debug: all

# Clean build files
clean:
	$(RM) $(OBJS)
	$(RM) libredis_lib.so

install: all
	cp -u libredis_lib.so /usr/lib
	cp --parents -u $(H_SRCS) /usr/include

uninstall:
	rm -f /usr/lib/libredis_lib.so

.PHONY: all clean install uninstall	