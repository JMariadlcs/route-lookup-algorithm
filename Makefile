SRC = tables.c io.c io.h utils.c utils.h main.c main.h tables.h
CFLAGS = -Wall -O3 -lm 

all: my_route_lookup

my_route_lookup: $(SRC)
	gcc $(CFLAGS) $(SRC) -o my_route_lookup -lm

.PHONY: clean

clean:
	rm -f my_route_lookup

#RL Lab 2020 Switching UC3M
