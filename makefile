CC = gcc
CFLAGS= -Wall -c
LFLAGS = -Wall

main: Render.so
	rm -f *.o
Render.so: src/Render.c src/Render.h
	$(CC) $(CFLAGS) -fpic src/Render.c  
	@mkdir -p lib
	$(CC) -shared -o lib/libRender.so Render.o
	@mkdir -p include
	@cp src/Render.h include
clean: 
	rm -rf lib
	rm -rf include
	rm -f *.o
