/*This source code copyrighted by Lazy Foo' Productions (2004-2013)
and may not be redistributed without written permission.*/

//The headers
#include "SDL.h"
#include "SDL_image.h"
#include <string>

//Screen attributes
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

//The frame rate
const int FRAMES_PER_SECOND = 20;

//The attributes of the car
const int SQUARE_WIDTH = 30;
const int SQUARE_HEIGHT = 50;

//The surfaces
SDL_Surface *car = NULL;
SDL_Surface *bad_car = NULL;
SDL_Surface *screen = NULL;

//The event structure
SDL_Event event;

//The wall
SDL_Rect left_wall;
SDL_Rect right_wall;

bool pressed_up = false;
bool pressed_down = false;
bool pressed_left = false;
bool pressed_right = false;

//The car
class Square
{
    private:
    //The collision box of the car
    SDL_Rect box;
    SDL_Rect bad_car_box;

    //The velocity of the car
    int xVel, yVel;

    public:
    //Initializes the variables
    Square();

    //Takes key presses and adjusts the car's velocity
    void handle_input();

    //Moves the car
    void move();

    //Shows the car on the screen
    void show();
};

//The timer
class Timer
{
    private:
    //The clock time when the timer started
    int startTicks;

    //The ticks stored when the timer was paused
    int pausedTicks;

    //The timer status
    bool paused;
    bool started;

    public:
    //Initializes variables
    Timer();

    //The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //Gets the timer's time
    int get_ticks();

    //Checks the status of the timer
    bool is_started();
    bool is_paused();
};

SDL_Surface *load_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = x;
    offset.y = y;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool check_collision( SDL_Rect A, SDL_Rect B )
{
    //The sides of the rectangles
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //Calculate the sides of rect A
    leftA = A.x;
    rightA = A.x + A.w;
    topA = A.y;
    bottomA = A.y + A.h;

    //Calculate the sides of rect B
    leftB = B.x;
    rightB = B.x + B.w;
    topB = B.y;
    bottomB = B.y + B.h;

    //If any of the sides from A are outside of B
    if( bottomA <= topB )
    {
        return false;
    }

    if( topA >= bottomB )
    {
        return false;
    }

    if( rightA <= leftB )
    {
        return false;
    }

    if( leftA >= rightB )
    {
        return false;
    }

    //If none of the sides from A are outside B
    return true;
}

bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }

    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );

    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }

    //Set the window caption
    SDL_WM_SetCaption( "Move the Square", NULL );

    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the car image
    car = load_image( "tinycar.png" );
    bad_car = load_image("tinyredcar.png");

    //If there was a problem in loading the car
    if( car == NULL || bad_car == NULL)
    {
        return false;
    }

    //If everything loaded fine
    return true;
}

void clean_up()
{
    //Free the surface
    SDL_FreeSurface( car );
    SDL_FreeSurface( bad_car );

    //Quit SDL
    SDL_Quit();
}

Square::Square()
{
    //Initialize the offsets
    box.x = 3 * SCREEN_WIDTH / 12 - SQUARE_WIDTH/2;
    box.y = 3 * SCREEN_HEIGHT / 4;

    bad_car_box.x = SCREEN_WIDTH/12 - SQUARE_WIDTH/2;
    bad_car_box.y = 200;
    
    //Set the car's dimentions
    box.w = SQUARE_WIDTH;
    box.h = SQUARE_HEIGHT;
    bad_car_box.w = SQUARE_WIDTH;
    bad_car_box.h = SQUARE_HEIGHT;
    
    //Initialize the velocity
    xVel = 0;
    yVel = -SQUARE_HEIGHT / 3;
}

void Square::handle_input()
{
    //If a key was pressed
    if( event.type == SDL_KEYDOWN )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_UP:
                        yVel -= SQUARE_HEIGHT / 2; 
                        pressed_up = true; 
                        break;
            case SDLK_DOWN: 
                        yVel += SQUARE_HEIGHT / 2; 
                        pressed_down = true;
                        break;
            case SDLK_LEFT: 
                        if(!pressed_left) xVel =  -SCREEN_WIDTH / 12;
                        else xVel =  0;
                        pressed_left = true;
                        break;
            case SDLK_RIGHT: 
                        if(!pressed_right) xVel = SCREEN_WIDTH / 12; 
                        else xVel =  0;
                        pressed_right = true;
                        break;
        }
        
        
    }
    //If a key was released
    else if( event.type == SDL_KEYUP )
    {
        //Adjust the velocity
        switch( event.key.keysym.sym )
        {
            case SDLK_UP: yVel += SQUARE_HEIGHT / 2; break;
            case SDLK_DOWN: yVel -= SQUARE_HEIGHT / 2; break;
            case SDLK_LEFT: xVel =  0; pressed_left = false; break;
            case SDLK_RIGHT: xVel = 0; pressed_right = false; break;
        }
    }
}

void Square::move()
{
    //Move the car left or right
   
    box.x += xVel;
    if( box.x > SCREEN_WIDTH / 2) box.x = SCREEN_WIDTH/12 + SCREEN_WIDTH/3 - SQUARE_WIDTH/2;
     if( box.x < 0) box.x = SCREEN_WIDTH/12 - SQUARE_WIDTH/2;

    //If the car went too far to the left or right or has collided with the wall
    if( ( box.x < 0 ) || ( box.x + SQUARE_WIDTH > SCREEN_WIDTH ) || ( check_collision( box, left_wall ) )  || ( check_collision( box, right_wall ) ) )
    {
        //Move back
        box.x -= xVel;
    }

    //Move the car up or down
    //box.y += yVel;
    bad_car_box.y -=yVel;

    //If the car went too far up or down or has collided with the wall
    if( ( box.y < 0 ) || ( box.y + SQUARE_HEIGHT > SCREEN_HEIGHT ) || ( check_collision( box, left_wall ) )  || ( check_collision( box, right_wall ) ) )
    {
        //Move back
        //box.y -= yVel;
        bad_car_box.y += yVel;
    }
    
    if(bad_car_box.y <0) bad_car_box.y = SCREEN_HEIGHT;
    if(bad_car_box.y > SCREEN_HEIGHT + SQUARE_HEIGHT || check_collision( box, bad_car_box )){
             bad_car_box.y = 0;
             bad_car_box.x = rand() % 3 * (SCREEN_WIDTH/6) + SCREEN_WIDTH/12 - SQUARE_WIDTH/2;
     }
     
   /*if( check_collision( box, bad_car_box ) ){
   //respawn car
    box.x = 3 * SCREEN_WIDTH / 12 - SQUARE_WIDTH/2;
    box.y = 3 * SCREEN_HEIGHT / 4;
   
   }*/
}

void Square::show()
{
    //Show the car
    apply_surface( box.x, box.y, car, screen );
    apply_surface( bad_car_box.x, bad_car_box.y, bad_car, screen );
}

Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}

int main( int argc, char* args[] )
{
    //Quit flag
    bool quit = false;

    //The car
    Square mySquare;

    
    
    //The frame rate regulator
    Timer fps;

    //Initialize
    if( init() == false )
    {
        return 1;
    }

    //Load the files
    if( load_files() == false )
    {
        return 1;
    }

    //Set the wall
    right_wall.x = SCREEN_WIDTH/2;
    right_wall.y = 0;
    right_wall.w = SCREEN_HEIGHT/40;
    right_wall.h = SCREEN_HEIGHT;
    
    left_wall.x = 0;
    left_wall.y = 0;
    left_wall.w = SCREEN_HEIGHT/40;
    left_wall.h = SCREEN_HEIGHT;
    
    SDL_Rect canvas;
    canvas.x = 0;
    canvas.y = 0;
    canvas.w = SCREEN_WIDTH/2;
    canvas.h = SCREEN_HEIGHT;
    
    SDL_Rect line1, line2;
    line1.x = SCREEN_WIDTH/6;
    line1.y = 0;
    line1.w = 2;
    line1.h = SCREEN_HEIGHT;
    line2.x = SCREEN_WIDTH/3;
    line2.y = 0;
    line2.w = 2;
    line2.h = SCREEN_HEIGHT;

    //While the user hasn't quit
    while( quit == false )
    {
        //Start the frame timer
        fps.start();

        //While there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            //Handle events for the car
            mySquare.handle_input();

            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                //Quit the program
                quit = true;
            }
        }

        //Move the car
        mySquare.move();

        //Fill the screen white
        SDL_FillRect( screen, &screen->clip_rect, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
        SDL_FillRect( screen, &canvas, SDL_MapRGB( screen->format, 0x00, 0x00, 0x00 ) );
        
        SDL_FillRect( screen, &line1, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
        SDL_FillRect( screen, &line2, SDL_MapRGB( screen->format, 0xFF, 0xFF, 0xFF ) );
        
        
        //Show the wall
        SDL_FillRect( screen, &left_wall, SDL_MapRGB( screen->format, 0x77, 0x77, 0x77 ) );
        SDL_FillRect( screen, &right_wall, SDL_MapRGB( screen->format, 0x77, 0x77, 0x77 ) );

        //Show the car on the screen
        mySquare.show();

        //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }

        //Cap the frame rate
        if( fps.get_ticks() < 1000 / FRAMES_PER_SECOND )
        {
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fps.get_ticks() );
        }
    }

    //Clean up
    clean_up();

    return 0;
}
