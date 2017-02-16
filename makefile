OBJS = main.o
CC = g++ -I /usr/include/SDL 
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

car_race: $(OBJS)
	    $(CC) $(LFLAGS) $(OBJS) -o car_race -lSDL -lSDL_image

main.o: 
		$(CC) $(CFLAGS) main.cpp -lSDL -lSDL_image

clean:
		rm *.o car_race
