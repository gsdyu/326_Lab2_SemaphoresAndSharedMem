CC=gcc
CFLAGS=-lrt -pthread
DEPS=dungeon_info.h dungeon_settings.h
OBJ = dungeon.o

%.o: %.c $(DEPS)
	%(CC) -c -o $@ $< $(CFLAGS)

all: wizard rogue barbarian game
	

wizard rogue barbarian: $@ $(DEPS)
	$(CC) $@.c $^ -o $@ $(CFLAGS) 

game: $@ $(DEPS) $(OBJ)
	$(CC) $@.c $^ -o $@ $(CFLAGS)

