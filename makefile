OBJS = main.cpp
CC = g++ -I /usr/include/SDL2 
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall -lSDL2 -lSDL2_image $(DEBUG)

car_race: $(OBJS)
	    $(CC) $(LFLAGS) $(OBJS) $(LFLAGS) -o car_race 


clean:
		rm *.o car_race
