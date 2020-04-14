OBJS = main.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG) `sdl2-config --libs --cflags` -ggdb3 -O0 --std=c99 -Wall -lSDL2_image -lm
LFLAGS = -Wall -lSDL2 -lSDL2_image -lSDL2_gfx $(DEBUG)

car_race: $(OBJS)
	    $(CC) $(LFLAGS) $(OBJS) $(LFLAGS) -o car_race 


clean:
		rm *.o car_race
