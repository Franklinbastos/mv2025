CC = gcc
CFLAGS = -Wall -g

all: montador ligador mv

montador: src/montador.c include/linker_defs.h
	$(CC) $(CFLAGS) -o bin/montador src/montador.c

ligador: src/ligador.c include/linker_defs.h
	$(CC) $(CFLAGS) -o bin/ligador src/ligador.c

mv: src/mv.c
	$(CC) $(CFLAGS) -o bin/mv src/mv.c

clean:
	rm -f bin/montador bin/ligador bin/mv *.o


