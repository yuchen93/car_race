/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and, strings
#include </usr/include/SDL2/SDL.h>
#include </usr/include/SDL2/SDL_image.h>
#include <stdio.h>
#include <string>
#include <math.h>   
#include <vector>   


using namespace std;

#define PI 3.14159265
//Screen dimension constants
const int SCREEN_WIDTH = 240;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Creates blank texture
		bool createBlank( int width, int height, SDL_TextureAccess = SDL_TEXTUREACCESS_STREAMING );

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Set self as render target
		void setAsRenderTarget();

		//Gets image dimensions
		int getWidth();
		int getHeight();

		//Pixel manipulators
		bool lockTexture();
		bool unlockTexture();
		void* getPixels();
		void copyPixels( void* pixels );
		int getPitch();
		Uint32 getPixel32( unsigned int x, unsigned int y );

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;
		void* mPixels;
		int mPitch;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The application time based timer
class LTimer
{
    public:
		//Initializes variables
		LTimer();

		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();

		//Gets the timer's time
		Uint32 getTicks();

		//Checks the status of the timer
		bool isStarted();
		bool isPaused();

    private:
		//The clock time when the timer started
		Uint32 mStartTicks;

		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;

		//The timer status
		bool mPaused;
		bool mStarted;
};

//The dot that will move around on the screen
class Dot
{
    public:
		//The dimensions of the dot
		static const int DOT_WIDTH = 30;
		static const int DOT_HEIGHT = 60;

		//Maximum axis velocity of the dot
		static const int DOT_VEL = 50;

		//Initializes the variables
		Dot();

		//Takes key presses and adjusts the dot's velocity
		void handleEvent( SDL_Event& e );

		//Moves the dot
		void move( float timeStep );

		//Shows the dot on the screen
		void render();

		float mPosX, mPosY;
		float mVelX, mVelY;
                float angle;
};


class Marker
{
    public:
		//The dimensions of the marker
		static const int MARKER_WIDTH = 4;
		static const int MARKER_HEIGHT = 40;
		//Maximum axis velocity of the marker
		static const int MARKER_VEL = 50;
		//Initializes the variables
		Marker(float x_pos, float y_pos);
		//Takes key presses and adjusts the marker's velocity
		void handleEvent( SDL_Event& e );
		//Moves the marker
		void move( float timeStep );
		//Shows the marker on the screen
		void render();

    private:
		float mPosX, mPosY;
		float mVelX, mVelY;
                float angle;
};

class Agent
{
    public:
		//The dimensions of the agent
		static const int MARKER_WIDTH = 30;
		static const int MARKER_HEIGHT = 60;
		static const int MARKER_VEL = 50;
		//Initializes the variables
		Agent(float x_pos, float y_pos);
		void handleEvent( SDL_Event& e );
		void move( float timeStep );
		void render();

    
		float mPosX, mPosY;
		float mVelX, mVelY;
                float angle;
};

//Starts up SDL and creates window
bool init();
//Loads media
bool loadMedia();
//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gDotTexture;
LTexture gAgentTexture;


LTexture::LTexture(): mTexture(NULL), mWidth(0), mHeight(0)
{
	//Initialize
	mPixels = NULL;
	mPitch = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Convert surface to display format
		SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat( loadedSurface, SDL_PIXELFORMAT_RGBA8888, NULL );
		if( formattedSurface == NULL )
		{
			printf( "Unable to convert loaded surface to display format! %s\n", SDL_GetError() );
		}
		else
		{
			//Create blank streamable texture
			newTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, formattedSurface->w, formattedSurface->h );
			if( newTexture == NULL )
			{
				printf( "Unable to create blank texture! SDL Error: %s\n", SDL_GetError() );
			}
			else
			{
				//Enable blending on texture
				SDL_SetTextureBlendMode( newTexture, SDL_BLENDMODE_BLEND );

				//Lock texture for manipulation
				SDL_LockTexture( newTexture, &formattedSurface->clip_rect, &mPixels, &mPitch );

				//Copy loaded/formatted surface pixels
				memcpy( mPixels, formattedSurface->pixels, formattedSurface->pitch * formattedSurface->h );

				//Get image dimensions
				mWidth = formattedSurface->w;
				mHeight = formattedSurface->h;

				//Get pixel data in editable format
				Uint32* pixels = (Uint32*)mPixels;
				int pixelCount = ( mPitch / 4 ) * mHeight;

				//Map colors				
				Uint32 colorKey = SDL_MapRGB( formattedSurface->format, 0, 0xFF, 0xFF );
				Uint32 transparent = SDL_MapRGBA( formattedSurface->format, 0x00, 0xFF, 0xFF, 0x00 );

				//Color key pixels
				for( int i = 0; i < pixelCount; ++i )
				{
					if( pixels[ i ] == colorKey )
					{
						pixels[ i ] = transparent;
					}
				}

				//Unlock texture to update
				SDL_UnlockTexture( newTexture );
				mPixels = NULL;
			}

			//Get rid of old formatted surface
			SDL_FreeSurface( formattedSurface );
		}	
		
		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif
		
bool LTexture::createBlank( int width, int height, SDL_TextureAccess access )
{
	//Create uninitialized texture
	mTexture = SDL_CreateTexture( gRenderer, SDL_PIXELFORMAT_RGBA8888, access, width, height );
	if( mTexture == NULL )
	{
		printf( "Unable to create blank texture! SDL Error: %s\n", SDL_GetError() );
	}
	else
	{
		mWidth = width;
		mHeight = height;
	}

	return mTexture != NULL;
}

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
		mPixels = NULL;
		mPitch = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

void LTexture::setAsRenderTarget()
{
	//Make self render target
	SDL_SetRenderTarget( gRenderer, mTexture );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

bool LTexture::lockTexture()
{
	bool success = true;

	//Texture is already locked
	if( mPixels != NULL )
	{
		printf( "Texture is already locked!\n" );
		success = false;
	}
	//Lock texture
	else
	{
		if( SDL_LockTexture( mTexture, NULL, &mPixels, &mPitch ) != 0 )
		{
			printf( "Unable to lock texture! %s\n", SDL_GetError() );
			success = false;
		}
	}

	return success;
}

bool LTexture::unlockTexture()
{
	bool success = true;

	//Texture is not locked
	if( mPixels == NULL )
	{
		printf( "Texture is not locked!\n" );
		success = false;
	}
	//Unlock texture
	else
	{
		SDL_UnlockTexture( mTexture );
		mPixels = NULL;
		mPitch = 0;
	}

	return success;
}

void* LTexture::getPixels()
{
	return mPixels;
}

void LTexture::copyPixels( void* pixels )
{
	//Texture is locked
	if( mPixels != NULL )
	{
		//Copy to locked pixels
		memcpy( mPixels, pixels, mPitch * mHeight );
	}
}

int LTexture::getPitch()
{
	return mPitch;
}

Uint32 LTexture::getPixel32( unsigned int x, unsigned int y )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32*)mPixels;

    //Get the pixel requested
    return pixels[ ( y * ( mPitch / 4 ) ) + x ];
}


LTimer::LTimer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks()
{
	//The actual timer time
	Uint32 time = 0;

    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
	//Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
	//Timer is running and paused
    return mPaused && mStarted;
}


Dot::Dot()
{
    //Initialize the position
    mPosX = SCREEN_WIDTH/2 - DOT_WIDTH/2;
    mPosY = 2*SCREEN_HEIGHT/3 - DOT_HEIGHT/2;
    angle = 0;

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
}

void Dot::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY -= DOT_VEL; break;
            case SDLK_DOWN: mVelY += DOT_VEL; break;
            case SDLK_LEFT: mVelX -= DOT_VEL; break;
            case SDLK_RIGHT: mVelX += DOT_VEL; break;
            //case SDLK_LEFT: mVelX -= DOT_VEL; break;
            //case SDLK_RIGHT: mVelX += DOT_VEL; break;
        }
    }else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_LEFT: mVelX += DOT_VEL; break;
            case SDLK_RIGHT: mVelX -= DOT_VEL; break;
             case SDLK_UP: mVelY += DOT_VEL; break;
            case SDLK_DOWN: mVelY -= DOT_VEL; break;
        }
    }
    if (mVelY < -500) mVelY = -500;
    else if (mVelY > -50) mVelY = -50;
}

void Dot::move( float timeStep )
{
   
    angle += mVelX * timeStep;
    if (mVelX == 0 && angle > 0) angle -= timeStep*20;
    else if ( mVelX == 0 && angle < 0) angle += timeStep*20;
    if (angle < -55) angle = -55;
    else if (angle > 55) angle = 55;

     //Move the dot left or right
      mPosX += mVelY * timeStep * sin(-angle*PI/180);
    //If the dot went too far to the left or right
	if( mPosX < 0 )
	{
		mPosX = 0;
	}
	else if( mPosX > SCREEN_WIDTH - DOT_WIDTH )
	{
		mPosX = SCREEN_WIDTH - DOT_WIDTH;
	}
	
    //Move the dot up or down
    //  mPosY += mVelY * timeStep * cos(-angle*PI/180);

    //If the dot went too far up or down
	if( mPosY < 0 )
	{
		mPosY = 0;
	}
	else if( mPosY > SCREEN_HEIGHT - DOT_HEIGHT )
	{
		mPosY = SCREEN_HEIGHT - DOT_HEIGHT;
	}
}

void Dot::render()
{
    //Show the dot
	gDotTexture.render( (int)mPosX, (int)mPosY, NULL, (int)angle );
}



// MARKER
Marker::Marker(float pos_x, float pos_y)
{
    //Initialize the position
    mPosX = pos_x - MARKER_WIDTH/2;
    mPosY = pos_y - MARKER_HEIGHT/2;
    angle = 0;
    //Initialize the velocity
    mVelX = 0;
    mVelY = 40;
}

void Marker::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP:    mVelY += MARKER_VEL; break;
            case SDLK_DOWN:  mVelY -= MARKER_VEL; break;
            case SDLK_LEFT:  mVelX -= MARKER_VEL; break;
            case SDLK_RIGHT: mVelX += MARKER_VEL; break;
        }
        if (mVelY < 50) mVelY = 50;
        else if (mVelY > 500) mVelY = 500;
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY -= MARKER_VEL; break;
            case SDLK_DOWN: mVelY += MARKER_VEL; break;
            case SDLK_LEFT: mVelX += MARKER_VEL; break;
            case SDLK_RIGHT: mVelX -= MARKER_VEL; break;
        }
    }
}

void Marker::move( float timeStep )
{
    angle += mVelX * timeStep;
    if (mVelX == 0 && angle > 0) angle -= timeStep*20;
    else if ( mVelX == 0 && angle < 0) angle += timeStep*20;
    if (angle < -55) angle = -55;
    else if (angle > 55) angle = 55;
    //Move the marker up or down
    mPosY += mVelY * timeStep * cos(-angle*PI/180);

    //If the dot went too far up or down
	if( mPosY < - MARKER_HEIGHT )
	{
		mPosY = SCREEN_HEIGHT + MARKER_HEIGHT;
	}
	else if( mPosY > SCREEN_HEIGHT + MARKER_HEIGHT )
	{
		mPosY = - MARKER_HEIGHT;
	}
}

void Marker::render()
{
     //Render markers
	SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x22, 0xCC );
	SDL_Rect fillRect = {(int)mPosX - MARKER_WIDTH/2, (int)mPosY - MARKER_HEIGHT/2, MARKER_WIDTH, MARKER_HEIGHT };        
	SDL_RenderFillRect( gRenderer, &fillRect );

}



// Agent
Agent::Agent(float pos_x, float pos_y)
{
    //Initialize the position
    mPosX = pos_x - MARKER_WIDTH/2;
    mPosY = pos_y - MARKER_HEIGHT/2;
    angle = 0;
    //Initialize the velocity
    mVelX = 0;
    mVelY = 20;
}

void Agent::handleEvent( SDL_Event& e )
{
    //If a key was pressed
    if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY += MARKER_VEL; break;
            case SDLK_DOWN: mVelY -= MARKER_VEL; break;
           // case SDLK_LEFT: mVelX -= MARKER_VEL; break;
           //  case SDLK_RIGHT: mVelX += MARKER_VEL; break;
        }

        if (mVelY < -50) mVelY = -50;
        else if (mVelY > 200) mVelY = 200; 
    } 
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_UP: mVelY -= MARKER_VEL; break;
            case SDLK_DOWN: mVelY += MARKER_VEL; break;
        }
    }
  
}

void Agent::move( float timeStep )
{
   
    //Move the marker up or down
    mPosY += mVelY * timeStep ;

    //If the dot went too far up or down
	if( mPosY < - MARKER_HEIGHT - 20 )
	{
		mPosY = SCREEN_HEIGHT + MARKER_HEIGHT + rand()%20;
                mPosX =  3 * SCREEN_WIDTH / 10 + (rand()%3) * 2 * SCREEN_WIDTH / 10 - MARKER_WIDTH/2;
	}
	else if( mPosY > SCREEN_HEIGHT + MARKER_HEIGHT + 20)
	{
		mPosY = -  MARKER_HEIGHT - rand() % 20;
                mPosX = 3 * SCREEN_WIDTH / 10 + (rand()%3) * 2 * SCREEN_WIDTH / 10 - MARKER_WIDTH/2;
	}
}

void Agent::render()
{
     //Render markers
	gAgentTexture.render( (int)mPosX, (int)mPosY, NULL, (int)angle);

}


bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		/*if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}*/

		//Create window
		gWindow = SDL_CreateWindow( "Car Sim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	
	//Load dot texture
	if( !gDotTexture.loadFromFile("images/tinycar.png" ) ) 
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}

        //Load dot texture
	if( !gAgentTexture.loadFromFile("images/tinyredcar.png" ) ) 
	{
		printf( "Failed to load dot texture!\n" );
		success = false;
	}
        

	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
        gAgentTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The dot that will be moving around on the screen
			Dot dot;
            const int num_markers = 14;
            const int num_agents = 2;
            
            Marker markers [num_markers] = {Marker(2 * SCREEN_WIDTH / 5,  0), 
                                            Marker(2 * SCREEN_WIDTH / 5,  2*SCREEN_HEIGHT/12),
	                                        Marker(2 * SCREEN_WIDTH / 5,  4*SCREEN_HEIGHT/12),
                                            Marker(2 * SCREEN_WIDTH / 5,  6*SCREEN_HEIGHT/12),
                                            Marker(2 * SCREEN_WIDTH / 5,  8*SCREEN_HEIGHT/12),
                                            Marker(2 * SCREEN_WIDTH / 5,  10*SCREEN_HEIGHT/12),
                                            Marker(2 * SCREEN_WIDTH / 5,  SCREEN_HEIGHT),
                                            Marker(3 * SCREEN_WIDTH / 5,  0), 
                                            Marker(3 * SCREEN_WIDTH / 5,  2*SCREEN_HEIGHT/12),
		                                    Marker(3 * SCREEN_WIDTH / 5,  4*SCREEN_HEIGHT/12),
                                            Marker(3 * SCREEN_WIDTH / 5,  6*SCREEN_HEIGHT/12),
                                            Marker(3 * SCREEN_WIDTH / 5,  8*SCREEN_HEIGHT/12),
                                            Marker(3 * SCREEN_WIDTH / 5,  10*SCREEN_HEIGHT/12),
                                            Marker(3 * SCREEN_WIDTH / 5,  SCREEN_HEIGHT)};

            Agent agents [num_agents] = {Agent( 3 * SCREEN_WIDTH / 10 , -30), Agent( 7 * SCREEN_WIDTH / 10 , -60)};
			//Keeps track of time between steps
			LTimer stepTimer;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}

					//Handle input for the dot
					dot.handleEvent( e );
                                        for(int i=0;i<num_markers;i++) markers[i].handleEvent( e );
                                        for(int i=0;i<num_agents;i++) agents[i].handleEvent( e );
                                        
                                        
				}

				//Calculate time step
				float timeStep = stepTimer.getTicks() / 1000.f;

				//Move for time step
				dot.move( timeStep );
                                for(int i=0;i<num_markers;i++) markers[i].move( timeStep );
				for(int i=0;i<num_agents;i++) agents[i].move( timeStep );

				//Restart step timer
				stepTimer.start();
                                

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

                                //Render grass lanes
                                SDL_SetRenderDrawColor( gRenderer, 0x22, 0x99, 0x22, 0xFF );
				SDL_Rect fillRect = { 0, 0, SCREEN_WIDTH / 5, SCREEN_HEIGHT };        
				SDL_RenderFillRect( gRenderer, &fillRect );
                                SDL_Rect fillRect2 = { 4*SCREEN_WIDTH / 5, 0, SCREEN_WIDTH / 5, SCREEN_HEIGHT };       
				SDL_RenderFillRect( gRenderer, &fillRect2 );

                                //Render road lanes
                                SDL_SetRenderDrawColor( gRenderer, 0x55, 0x55, 0x55, 0xBB );
				SDL_Rect fillRect3 = { SCREEN_WIDTH / 5, 0, 3*SCREEN_WIDTH / 5, SCREEN_HEIGHT };        
				SDL_RenderFillRect( gRenderer, &fillRect3 );
                               
                                //Render markers
                                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0x22, 0xCC );
				SDL_Rect fillRect4 = { SCREEN_WIDTH / 5 - 4, 0, 8, SCREEN_HEIGHT };        
				SDL_RenderFillRect( gRenderer, &fillRect4 );
                                SDL_Rect fillRect5 = { 4*SCREEN_WIDTH / 5 - 4, 0, 8, SCREEN_HEIGHT };        
				SDL_RenderFillRect( gRenderer, &fillRect5 );
                                for(int i=0;i<num_markers;i++) markers[i].render();
                                // HACK FOR collision check
                                
				for(int i=0;i<num_agents;i++){
                                    if (abs(agents[i].mPosX - dot.mPosX) < agents[i].MARKER_WIDTH - 5 && abs(agents[i].mPosY - dot.mPosY) < agents[i].MARKER_HEIGHT - 5 ) {
					agents[i].mPosY = -  agents[i].MARKER_HEIGHT - rand() % 20;
					agents[i].mPosX = 3 * SCREEN_WIDTH / 10 + (rand()%3) * 2 * SCREEN_WIDTH / 10 - agents[i].MARKER_WIDTH/2;
 				}
					}
				//while( agents[0].mPosX == agents[1].mPosX && abs(agents[0].mPosY - agents[1].mPosY) < agents[0].MARKER_HEIGHT + 5 ) agents[0].mPosX = 3 * SCREEN_WIDTH / 10 + (rand()%3) * 2 * SCREEN_WIDTH / 10 - agents[0].MARKER_WIDTH/2;
				//Render car
				for(int i=0;i<num_agents;i++) agents[i].render();
                                dot.render();

				//Update screen
				SDL_RenderPresent( gRenderer );
                                //SDL_Surface *screen = SDL_GetWindowSurface(gWindow);
				//IMG_SavePNG( screen, "./test_out.png");
                                // SDL_SaveBMP(screen, "./test_out.png");

                                
   // Create an empty RGB surface that will be used to create the screenshot bmp file 
   SDL_Surface* pScreenShot = SDL_GetWindowSurface(gWindow);; //SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_WIDTH, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000); 

   if(pScreenShot) 
   { 
      // Read the pixels from the current render target and save them onto the surface 
      SDL_RenderReadPixels(gRenderer, NULL, SDL_GetWindowPixelFormat(gWindow), pScreenShot->pixels, pScreenShot->pitch); 

      // Create the bmp screenshot file 
      SDL_SaveBMP(pScreenShot, "Screenshot_new.bmp"); 

      // Destroy the screenshot surface 
      SDL_FreeSurface(pScreenShot); 
   } 
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
