/* This code is released under the GNU GPL v2 license. */

//SDLpong main file
//compile (on linux) with:
//gcc main.c -o pong -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -I/usr/include/SDL

/* SDLpong written by Brandon Foltz as practice for coding in C.
   This code may not be as nice/clean as it could be, but it's a 
   learning experience, so bear with my n00bishness. It could 
   also (probably) be written in considerably less code, but I 
   wrote it this way in an attempt to keep everything organized 
   and easily understandable. Comments abound. */
   
/*
TODO: Ball serving for players
TODO: Multiple source files?
TODO: Make game scale per resolution
TODO: Graphics settings?
TODO: Bounce sound
TODO: Clean up code!
*/

//standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//file handling and sys stuff
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

//game specific includes
#include "main.h"

//definitions

//engine
//#define WINDOW_WIDTH 640
//#define WINDOW_HEIGHT 480
#define BPP 32
#define WINDOW_TITLE "SpacePong"
#define MAX_FPS 32 //set this to some high number to pseudo-uncap fps (even numbers plz)

#define FALSE 0
#define TRUE !FALSE

//--help text
#define HELP_TEXT " Usage: pong [options] \n" \
				" -w [window width in pixels] default: 640 \n" \
				" -h [window height in pixels] default: 480\n" \
				" -f starts game in fullscreen mode" 

//menu
#define MENU_TEXT "Spacebar to play. Esc to quit. Tab for 2 players."
#define MENU_TEXT2 "Up/Down keys for Player 1, A/Z for Player 2."
#define MENU_TEXT3 "Written by: Brandon Foltz."

//game
//these are all old defaults for 640x480
//#define PADDLE_SPEED 8
//#define PADDLE_LENGTH 64
//#define PADDLE_WIDTH 12
//#define BALL_MAX_SPEED 10 //this should be less than paddle width to avoid glitching
//#define BALL_SIZE 6
#define BALL_DELAY 3 //after score, ball delays movement for this many seconds
#define MAX_SCORE 10 //game ends at this score

//some values to tweak AI difficulty with
//#define AI_REACT_DIST 320 //WINDOW_WIDTH / 2 //bot wont react until ball reaches middle of screen
//#define AI_TOLERANCE 20 //lower num = greater difficulty. more than half PADDLE_LENGTH = bot fail, hard.

//effects values
#define BALL_HIT_SPARKS 12
#define BALL_TRAIL_LIFE 16 
#define BACKGROUND_SPEED 1.5 //experiment with floating values
#define PADDLE_FADETIME 16 //in frames. lower = faster

#ifdef _WIN32
int WinMain(int argc, char *argv[])
{	
	main(argc, argv);
}
#endif

int main(int argc, char *argv[])
{	
	InitArgs(argc, argv);
	
	if(!InitSDL(GameState)) //set up SDL stuffz
	{
		puts("SDL Init failed!");
		return 0; //no need to GameQuit() because nothing is initialized
	}
	InitScales();
	InitMisc();
	SetGameState(STATE_MENU);
	InitMenu();
	UpdateProgressBar(1,100); //finished loading, game starts!
	
	//start the game loop
	Frames.maxframetime = (1000 / MAX_FPS);
	if(GameState.debug)
		printf("Frames.maxframetime: %d\n", Frames.maxframetime);
	Frames.framestart = SDL_GetTicks();
	Frames.update = SDL_GetTicks();
	Frames.framems = 1;
	while(GameState.state != STATE_QUITTING)
	{		
		Frames.framestart = SDL_GetTicks();
		CalcFPS(); //not part of "game" code. consider it engine code.
		
		GameLoop(); //compartmentalize main game code
		
		Frames.frametime += (SDL_GetTicks() - Frames.framestart);
		Frames.framems = (SDL_GetTicks() - Frames.framestart);
		RegulateFPS(); //if this starts bugging out, just stick SDL_Delay(30); in its place
		Frames.framems = 0;
	}	
	if(GameState.debug)
		printf("GameState at end = %d\n",GameState.state);
		
	puts("If the public doesn't like one's work... one has to accept the fact, gracefully."); 
	//lets me know the end of the program is reached	
	return 0;
}

//#####################################
//General Purpose/Engine Functions    
//#####################################

int InitArgs(int argc, char *argv[])
{
	//might want some extra error checking in here
	//initializing default gamestate params
	GameState.resx = 640;
	GameState.resy = 480;
	GameState.fullscreen = 0;
	
	int i = 0;
	for(i=0;i<argc;i++)
	{
		if(0 == strcmp(argv[i], "-w"))
			GameState.resx = atoi(argv[i+1]);
		else if(0 == strcmp(argv[i], "-h"))
			GameState.resy = atoi(argv[i+1]);
		else if(0 == strcmp(argv[i], "-f"))
			GameState.fullscreen = 1;	
		else if(0 == strcmp(argv[i], "--help"))
		{
			puts(HELP_TEXT);
			GameQuit();
		}
	}
	return 1; //unless fail
}

int InitSDL(s_state GameState)
{
	//this func returns 0 on failure, 1 on success

	if (SDL_Init(SDL_INIT_VIDEO) < 0 ) //initialize SDL video
	{
		puts("SDL Video Init failed!");
		return 0; 
	}
	
	if(TTF_Init() < 0) //initialize SDL text
	{
		puts("SDL TTF_Init failed!");
		return 0;
	}	
	
	if(!(font = TTF_OpenFont("arial.ttf", 12))) //load font
	{
		puts("Unable to load font! (arial.ttf)");
		return 0;
	}
	
	//set up SDL window
	if(!(GameState.fullscreen))
	{
		if (!(screen = SDL_SetVideoMode(GameState.resx, GameState.resy, BPP, SDL_HWSURFACE)))
		//SDL_FULLSCREEN)))//SDL_SWSURFACE || SDL_FULLSCREEN)))char imgname[64];
		{
			puts("SDL Init Failed.");
    		SDL_Quit();
    		return 0;
		}
   	 }
	else if(GameState.fullscreen)
	{
		if (!(screen = SDL_SetVideoMode(GameState.resx, GameState.resy, BPP, SDL_FULLSCREEN | SDL_HWSURFACE)))
		{
			puts("SDL Init Failed.");
    		SDL_Quit();
    		return 0;
		}
	}	
    
	SDL_WM_SetCaption(WINDOW_TITLE, WINDOW_TITLE); //set window caption
	
	if(GameState.debug)
		puts("SDL Init Success!");
		
	return 1; //success!
}	

void InitMisc(void)
{
	//render "loading..."
	//SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
	DrawText("Loading...", (GameState.resx / 2) - 22, (GameState.resy / 2) - 32);
	UpdateProgressBar(0,1);

	GameState.debug = 0; //debug mode is only useful when run from terminal
	GameState.state = 1; //initialize it to avoid quitting on start
	SetGameState(STATE_INITIALIZING); //for consistency's sake
	
	GameState.paused = 0;
	GameState.winner = -1; //-1 means no games have been played, so no winner
	
	srand((unsigned)time(NULL)); //seed the randomizer ONLY ONCE!
	
	LoadBackgrounds();
	UpdateProgressBar(0,1);
	
	//init sprite states!
	int i;
	for(i=0;i<MAX_SPRITES;i++)
		Sprite[i].enabled = 0;
	
	Ball.r = 255; Ball.g = 255; Ball.b = 255; Ball.a = 255;
	
	for(i=0;i<2;i++)
	{
		Paddle[i].r = 255; 
		Paddle[i].g = 255; 
		Paddle[i].b = 255; 
		Paddle[i].a = 255;
		Paddle[i].defr = 255; 
		Paddle[i].defg = 255; 
		Paddle[i].defb = 255; 
		Paddle[i].defa = 255;
		Paddle[i].ticks = 0;
		Paddle[i].score = 0;
	}
	
	//init default AI values
	AI.reactdist = GameState.resx / 2;
	AI.tolerance = 20;
	
	UpdateProgressBar(0,1);
}

void InitScales(void)
{
	Paddle[0].width = GameState.resx / 50;
	Paddle[0].height = GameState.resy / 7;
	Paddle[0].speed = GameState.resy / 60;
	Paddle[1].width = GameState.resx / 50;
	Paddle[1].height = GameState.resy / 7;
	Paddle[1].speed = GameState.resy / 60;
	
	Ball.maxspeed = GameState.resx / 64;
	Ball.size = GameState.resx / 80;
}

void DrawText(char text[128], int x, int y) 
{
	//could use a better text drawing function... 
	SDL_Color foregroundColor = { 255, 255, 255 }; 
   	SDL_Color backgroundColor = { 0, 0, 0 };
   	SDL_Rect textLocation = { x, y, 0, 0 };
	//textSurface = TTF_RenderText_Shaded(font, text, foregroundColor, backgroundColor);
    textSurface = TTF_RenderText_Solid(font, text, foregroundColor);
    SDL_BlitSurface(textSurface, NULL, screen, &textLocation);
    SDL_FreeSurface(textSurface); //have to free after render (otherwise memory leak)
}

void ClearScreen(int r, int g, int b)
{
	//fill the screen with whatever rgb color
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, r, g, b));
}

SDL_Surface *LoadImage(char filename[128]) 
{
	//Temporary storage for the image that's loaded
	SDL_Surface *loadedImage = NULL;

	//The optimized image that will be used
	SDL_Surface* optimizedImage = NULL;
    
   	loadedImage = IMG_Load(filename);//SDL_LoadBMP(filename);
	if(loadedImage != NULL)
	{
    	//Create an optimized image
    	optimizedImage = SDL_DisplayFormat(loadedImage);
    
    	//Free the old image
    	SDL_FreeSurface(loadedImage);
	}
	//Return the optimized image
	return optimizedImage;
}

void SpawnSprite(s_sprite TmpSprite)
{
	int i;
	for(i=0;i<MAX_SPRITES;i++)
	{
		if(!(Sprite[i].enabled))
		{
			Sprite[i].enabled = 1;
			Sprite[i].type = TmpSprite.type;
			Sprite[i].x = TmpSprite.x;
			Sprite[i].y = TmpSprite.y;
			Sprite[i].xspeed = TmpSprite.xspeed;
			Sprite[i].yspeed = TmpSprite.yspeed;
			Sprite[i].w = TmpSprite.w;
			Sprite[i].h = TmpSprite.h;
			Sprite[i].tick = 0;
			Sprite[i].lifespan = TmpSprite.lifespan;
			Sprite[i].r = TmpSprite.r;
			Sprite[i].g = TmpSprite.g;
			Sprite[i].b = TmpSprite.b;
			Sprite[i].a = TmpSprite.a;
			Sprite[i].alphafade = TmpSprite.alphafade;
			Sprite[i].faderate = TmpSprite.faderate;
			break;
		}
	}		
}
	
void ManageSprites(void)
{
	int i;
	for(i=0;i<MAX_SPRITES;i++)
	{
		if(Sprite[i].enabled)
		{
			if(Sprite[i].tick < Sprite[i].lifespan)	
			{
				Sprite[i].tick += 1;
				Sprite[i].x += Sprite[i].xspeed;
				Sprite[i].y += Sprite[i].yspeed;
				if(Sprite[i].alphafade)
					Sprite[i].a -= Sprite[i].faderate;
					
				//here is sprite physics
				Sprite[i].x += Sprite[i].xspeed;
				Sprite[i].y += Sprite[i].yspeed;		
			}
			else if(Sprite[i].tick >= Sprite[i].lifespan)
			{
				Sprite[i].enabled = 0;
				Sprite[i].tick = 0;
			}
		}	
	}			
}

void RenderSprites(void)
//Much of this is untested/unused. Just written for completeness.
{
	int i;
	for(i=0;i<MAX_SPRITES;i++)
	{
		if(Sprite[i].enabled)
		{
			if(Sprite[i].type == SPRITE_IMAGE)
			{
				SDL_Rect offset,srcoffset;
				offset.x = Sprite[i].x;
				offset.y = Sprite[i].y;
				offset.w = Sprite[i].w;
				offset.h = Sprite[i].h;
				
				srcoffset.x = Sprite[i].x;
				srcoffset.y = Sprite[i].y;
				srcoffset.w = Sprite[i].w;
				srcoffset.h = Sprite[i].h;
				//FIXME: This isn't correct
				//SDL_BlitSurface(background, &srcoffset, screen, &offset );
			}
			else if(Sprite[i].type == SPRITE_PIXEL)
			{
				pixelRGBA(screen, Sprite[i].x, Sprite[i].y, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, \
					Sprite[i].a);
			}
			else if(Sprite[i].type == SPRITE_SPARK)
			{
				lineRGBA(screen, Sprite[i].x, Sprite[i].y, \
					Sprite[i].x + Sprite[i].xspeed, \
					Sprite[i].y + Sprite[i].yspeed, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, \
					Sprite[i].a);
			}
			else if(Sprite[i].type == SPRITE_RECT)
			{
				rectangleRGBA(screen, Sprite[i].x, Sprite[i].y, \
					Sprite[i].x + Sprite[i].w, \
					Sprite[i].y + Sprite[i].h, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, \
					Sprite[i].a);
			}
			else if(Sprite[i].type == SPRITE_FILLEDRECT)
			{
				boxRGBA(screen, Sprite[i].x, Sprite[i].y, \
					Sprite[i].x + Sprite[i].w, \
					Sprite[i].y + Sprite[i].h, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, \
					Sprite[i].a);
			}
			else if(Sprite[i].type == SPRITE_CIRCLE)
			{
				circleRGBA(screen, Sprite[i].x, Sprite[i].y, Sprite[i].rad, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, Sprite[i].a);
			}
			else if(Sprite[i].type == SPRITE_FILLEDCIRCLE)
			{
				filledCircleRGBA(screen, Sprite[i].x, Sprite[i].y, Sprite[i].rad, \
					Sprite[i].r, Sprite[i].g, Sprite[i].b, Sprite[i].a);
			}
		}	
	}
}

void SetGameState(enum GAME_STATES state)
{
	if(GameState.state == STATE_QUITTING)
	{
		puts("Game Quitting!");
		GameQuit();
	}
	
	GameState.state = state;
	
	if(GameState.debug)
		printf("GameState set to %d\n",GameState.state);
		
	//check again to see if it was set to STATE_QUITTING
	//and go ahead and quit from here. No point in waiting
	//STATE_QUITTING checks in the game loop are basically just for safety
	
	if(GameState.state == STATE_QUITTING)
	{
		puts("Game Quitting!");
		GameQuit();
	}
}	

void RegulateFPS(void)
{	
	int timetodelay;
	timetodelay = Frames.maxframetime - Frames.framems;

	if(timetodelay > 0)
	{
		if(timetodelay < Frames.maxframetime);
			SDL_Delay(timetodelay);
	}
}

void CalcFPS(void)
{
	if ((SDL_GetTicks() - (Frames.update)) > 1000)
	{
		Frames.fps = (Frames.frames) / (float)((SDL_GetTicks()-(Frames.update))) * 1000;
		Frames.update = SDL_GetTicks();
		
		if(GameState.debug)
			printf("FPS: %4.3f, Avg. Frametime: %3.3fms\n",Frames.fps,(Frames.frametime / Frames.frames));
			
		Frames.frames=0;
		Frames.frametime=0;
	}
	Frames.frames++;
}

int RandInt(int min, int max)
{
	int TmpRand,Neg; 
	if(min < 0)
	{
		Neg = rand() % 10 + 1;
		if(Neg > 5)
			return (rand() % max) + min;
		else if(Neg <= 5)
			return -((rand() % max) + min);
	}
	else
		return (rand() % max) + min;
}

float RandFloat(float min, float max)
{
	int Neg; 
	if(min < 0)
	{
		Neg = rand() % 10 + 1;
		if(Neg > 5)
			return ( ((float)rand()/RAND_MAX) * (max-min)) + min; //space parentheses like this for readability?
		else if(Neg <= 5)
			return -( ( ((float)rand()/RAND_MAX) * (max-min)) + min);
	}
	else
		return ( ((float)rand()/RAND_MAX) * (max-min)) + min;
}

int DetectJPGs(char indir[256])
{
	//TODO: pass an array byref to take the filenames
	//TODO: and run this at start, loading images to pointer array
	//or maybe not...
	DIR *dhandle;
	struct dirent *drecord;
	struct stat sbuf;
	int numfiles = 0;
	
	dhandle = opendir(indir);
	if(dhandle == NULL)
		printf("(DetectJPGs) Failed to open directory!\n");
	
	puts("Detecting .jpg files...");
	while((drecord = readdir(dhandle)) != NULL)
	{
		stat(drecord->d_name,&sbuf);
		
		if(!(strcmp(drecord->d_name+(strlen(drecord->d_name) - 4),".jpg")))
		{
			numfiles++;
			printf("%s\n",drecord->d_name);
		}	
	}
	closedir(dhandle);
	printf("Number of JPG's found: %d\n", numfiles);
	return numfiles;
}

int UpdateProgressBar(int mode, int param)
{
	if (mode == 0)
		progressbar += param;
	else if(mode == 1)
		progressbar = param;
		
	//bar container
	rectangleRGBA(screen, (GameState.resx / 2) - 52, (GameState.resy / 2) - 12, \
				(GameState.resx / 2) + 52, (GameState.resy / 2) + 11, \
				255,255,255,255);	
	//bar				
	SDL_Rect bar;
	bar.x = (GameState.resx / 2) - 50;
	bar.y = (GameState.resy / 2) - 10;
	bar.h = 20;
	bar.w = progressbar;
	SDL_FillRect(screen, &bar, SDL_MapRGB(screen->format, 0, 255, 0));
	//rectangleRGBA(screen, (GameState.resx / 2) - 50, (GameState.resy / 2) - 10, \
				((GameState.resx / 2) - 50) + progressbar, (GameState.resy / 2) + 10, \
				0,255,0,255);			
					
	SDL_Flip(screen);
}

void GameQuit(void)
{
	TTF_Quit();
	SDL_Quit();
	if(GameState.debug)
		puts("Hasta la vista, baby!");
	exit(EXIT_SUCCESS);
}

//#####################################
//Menu Functions   
//#####################################

void InitMenu(void)
{
	Ball.x = RandInt(1,GameState.resx - Ball.size);
	Ball.y = RandInt(1,GameState.resy - Ball.size);
	Ball.xspeed = RandFloat(1,Ball.maxspeed);
	Ball.yspeed = RandFloat(1,Ball.maxspeed);
	Paddle[0].score = 0; Paddle[1].score = 0; //better fix...	
}

void MenuLogic(void)
{
	//Paddle[0].score = 0; Paddle[1].score = 0; //hackish fix...
	MenuPhysics();
	ManageBackground();
	ManageSprites();
	MenuRender();
	RenderSprites();
	SDL_Flip(screen);
}

void MenuKeyCheck(void)
{
	while(SDL_PollEvent(&SDLevent)) 
    {      
      	switch (SDLevent.type) 
      	{
          	case SDL_QUIT:
          		SetGameState(STATE_QUITTING); 
          		break; 
          	case SDL_KEYDOWN:
          		//if(GameState.debug)
          			//printf("Key pressed: %d\n",SDLevent.key.keysym.sym);                  			
        		
          		if(SDLevent.key.keysym.sym == SDLK_ESCAPE)
          		{
          			if(GameState.debug)
          				puts("Escape key pressed, quitting!");
          				
          			SetGameState(STATE_QUITTING);
          		}     
          		//spacebar to play!
          		else if(SDLevent.key.keysym.sym == SDLK_SPACE)
          		{
          			//setup and begin playing
          			if(GameState.debug)
          				puts("Starting game...");
          				
          			GameState.twoplayer = 0;
          			SetGameState(STATE_INITIALIZING);
          			InitGame();
          		}
          		else if(SDLevent.key.keysym.sym == SDLK_TAB)
          		{
          			//setup and begin playing
          			if(GameState.debug)
          				puts("Starting game...");
          				
          			GameState.twoplayer = 1;
          			SetGameState(STATE_INITIALIZING);
          			InitGame();
          		}
          		else if(SDLevent.key.keysym.sym == SDLK_d)
          		{
          			if(!GameState.debug)
          			{
          				GameState.debug = 1;
          				puts("Debug mode on.");
          			}
          			else
          			{
          				GameState.debug = 0;
          				puts("Debug mode off.");
          			}
          		}
          		else if(SDLevent.key.keysym.sym == SDLK_f)
          		{
          			if(!GameState.fullscreen)
          			{
          				GameState.fullscreen = 1;
          				InitSDL(GameState);
          			}
          			else
          			{
          				GameState.fullscreen = 0;
          				InitSDL(GameState);
          			}
          		}
          		else if(SDLevent.key.keysym.sym == SDLK_b)
          		{
          			InitBackground();
          		}
               	break;
       	}
	}	
}

void MenuPhysics(void)
{
	//move ball around
	Ball.x += Ball.xspeed;
	Ball.y += Ball.yspeed;
	BallTrail(BALL_TRAIL_LIFE); //ball trail
	
	//check if it hits the sides, and make it bounce off
	//nothing fancy, just using center of block as coords
	if((Ball.x + Ball.size / 2) <= 0 || (Ball.x + Ball.size / 2) >= GameState.resx)
	{
		Ball.xspeed = -(Ball.xspeed);
		BallSpark(BALL_HIT_SPARKS);
	}			
	if((Ball.y + Ball.size / 2) <= 0 || (Ball.y + Ball.size / 2) >= GameState.resy)
	{
		Ball.yspeed = -(Ball.yspeed);
		BallSpark(BALL_HIT_SPARKS);
	}
}

void MenuRender(void)
{	
	//draw bkg before anything else
	SDL_Rect offset,srcoffset;
	offset.x = 0;
	offset.y = 0;
	offset.w = GameState.resx;
	offset.h = GameState.resy;
	
	srcoffset.x = Background.x;
	srcoffset.y = Background.y;
	srcoffset.w = GameState.resx;
	srcoffset.h = GameState.resy;
	SDL_BlitSurface(Background.image, &srcoffset, screen, &offset );
	//char imgname[64];
	//draw the ball. It be bouncin' (and it's actually a square)
	SDL_Rect balldims;
	balldims.x = Ball.x;
	balldims.y = Ball.y;
	balldims.w = Ball.size;
	balldims.h = Ball.size;
	SDL_FillRect(screen, &balldims, SDL_MapRGB(screen->format, 255, 255, 255));

	//print winning text on menu once game is over
	
	char EndRoundText[128];
	if (GameState.winner >= 0)
	{
		if (GameState.twoplayer)
		{
			if (GameState.winner = 0)
			{
				sprintf(EndRoundText, "Player 2 Wins!");
				DrawText(EndRoundText, ((GameState.resx / 2) - (strlen(EndRoundText) * 2)),(GameState.resy / 2) - 24);
			}
			else if (GameState.winner = 1)
			{
				sprintf(EndRoundText, "Player 1 Wins!");
				DrawText(EndRoundText, ((GameState.resx / 2) - (strlen(EndRoundText) * 2)),(GameState.resy / 2) - 24);
			}
		}
		else 
		{
			if (GameState.winner = 0)
			{
				sprintf(EndRoundText, "You Win!");
				DrawText(EndRoundText, ((GameState.resx / 2) - (strlen(EndRoundText) * 2)),(GameState.resy / 2) - 24);
			}
			else if (GameState.winner = 1)
			{
				sprintf(EndRoundText, "Bot Wins!");
				DrawText(EndRoundText, ((GameState.resx / 2) - (strlen(EndRoundText) * 2)),(GameState.resy / 2) - 24);
			}
		}
	}

	//drawing the menu in the (more or less) middle of the screen	
	DrawText(MENU_TEXT, ((GameState.resx / 2) - (strlen(MENU_TEXT) * 2)),(GameState.resy / 2));
	DrawText(MENU_TEXT2, ((GameState.resx / 2) - (strlen(MENU_TEXT2) * 2)),(GameState.resy / 2) + 14);
	DrawText(MENU_TEXT3, ((GameState.resx / 2) - (strlen(MENU_TEXT3) * 2)),(GameState.resy / 2) + 28);
}

//#####################################
//Game Functions   
//#####################################

void GameLoop(void)
{
	//can only do one key check per frame due to the way SDL queue's events
	//keypresses and other events
	
	/*keychecking (specifically state-changing key checks) 
	go ahead of other stuff to avoid a bug */
	/*switch(GameState.state)
	{
		case STATE_MENU:
			MenuKeyCheck();
			break;
		case STATE_PLAYING:
			GameKeyCheck(); 
			break;
		default:
			break;
	}*/
	//herp why don't we just do it this way... as seen below
	//don't see why they can't go into their respective logic functions...
		
	switch(GameState.state)
	{
		/*Each state has it's own rendering/physics 
		function. This cuts down on complexity (as compared to 
		one big function with lots of condition checking). */		
		
		case STATE_QUITTING:
			GameQuit();
			break;
		case STATE_MENU:
			MenuKeyCheck();
			MenuLogic();
			break;
		case STATE_PLAYING:
			GameKeyCheck(); 
			GameLogic();
			break;
		default:
			puts("Houston, we have a problem.");
			break;
	}	
}

void GameLogic(void)
{
	if(!(GameState.paused))
	{
		if(!(GameState.twoplayer))
			AIThink();
		
		GamePhysics();
		GoalCheck();
		ManageBackground();
		ManageSprites(); 
		PaddleEffects();
	}
	GameRender();
	RenderSprites();
	SDL_Flip(screen);
}

void GameKeyCheck(void)
{
	//check keystates for game stuffs before keydown events that will 
	//break the loop
	
	SDL_KeyStates = SDL_GetKeyState( NULL );
	
	if(SDL_KeyStates[SDLK_UP])
		Paddle[1].yspeed = -(Paddle[1].speed);
	else if(SDL_KeyStates[SDLK_DOWN])
		Paddle[1].yspeed = Paddle[1].speed;
	
	if(GameState.twoplayer)
	{	
		if(SDL_KeyStates[SDLK_a])
			Paddle[0].yspeed = -(Paddle[0].speed);
		else if(SDL_KeyStates[SDLK_z])
			Paddle[0].yspeed = Paddle[0].speed;
	}

	while(SDL_PollEvent(&SDLevent))
    {      
	  	switch (SDLevent.type) 
	  	{
		  	case SDL_QUIT:
		  		SetGameState(STATE_QUITTING); //could do it directly
		  		//but this avoids errors
		  		break; 
		  	case SDL_KEYDOWN:
		  		if(GameState.debug)
		  			printf("Key pressed: %d\n",SDLevent.key.keysym.sym);                  						
		  		if(SDLevent.key.keysym.sym == SDLK_ESCAPE)
		  		{
		  			if(GameState.debug)
		  				puts("Escape key pressed, quitting!");
		  				
		  			SetGameState(STATE_QUITTING);
		  		}     
		  		else if(SDLevent.key.keysym.sym == SDLK_SPACE)
		  		{
		  			//something?
		  		}
		  		else if(SDLevent.key.keysym.sym == SDLK_d)
		  		{
		  			if(!GameState.debug)
		  			{
		  				GameState.debug = 1;
		  				puts("Debug mode on.");
		  			}
		  			else
		  			{
		  				GameState.debug = 0;
		  				puts("Debug mode off.");
		  			}
		  		}
		  		else if(SDLevent.key.keysym.sym == SDLK_p)
		  		{
		  			if(!GameState.paused)
		  			{
		  				GameState.paused = 1;
		  			}
		  			else
		  			{
		  				GameState.paused = 0;
		  			}
		  		}
		  		else if(SDLevent.key.keysym.sym == SDLK_f)
		  		{
		  			if(!GameState.fullscreen)
		  			{
		  				GameState.fullscreen = 1;
		  				InitSDL(GameState);
		  			}
		  			else
		  			{
		  				GameState.fullscreen = 0;
		  				InitSDL(GameState);
		  			}
		  		}
		  		else if(SDLevent.key.keysym.sym == SDLK_m)
		  		{
		  			SetGameState(STATE_MENU);
		  			InitMenu();
		  			GameState.paused = 0;
		  		}
		  		else if(SDLevent.key.keysym.sym == SDLK_b)
		  		{
		  			InitBackground();
		  		}
		       		break;
	       	case SDL_KEYUP:
	       		if(SDLevent.key.keysym.sym == SDLK_UP)
	       			Paddle[1].yspeed = 0;
	       		else if(SDLevent.key.keysym.sym == SDLK_DOWN)
	       			Paddle[1].yspeed = 0;
	       			
	       		if(GameState.twoplayer)
	       		{
	       			if(SDLevent.key.keysym.sym == SDLK_a)
	       				Paddle[0].yspeed = 0;
	       			else if(SDLevent.key.keysym.sym == SDLK_z)
   						Paddle[0].yspeed = 0;
   				}
		}
   	}
}	


void GamePhysics(void)
{
	int over,under,i; //vars used in this func

	//move paddles
	for(i=0;i<2;i++)
	{
		if(Paddle[i].y >= 0 && (Paddle[i].y + Paddle[i].height) <= GameState.resy)
			Paddle[i].y += Paddle[i].yspeed;
	}
		
	//bot sets it's own velocity in AIThink() function, but follows same rules as player
	
	//if it's out of bounds, move it back 
	for(i=0;i<2;i++)
	{
		if(Paddle[i].y < 0)
		{
			under = 0 - Paddle[i].y;
			Paddle[i].y += under;
		}
		else if((Paddle[i].y + Paddle[i].height) > GameState.resy)
		{
			over = (Paddle[i].y + Paddle[i].height) - GameState.resy;
			Paddle[i].y -= over;
		}
	}
	
	//move ball around
	if(Ball.movedelay <= 0)
	{
		Ball.x += Ball.xspeed;
		Ball.y += Ball.yspeed;
		BallTrail(BALL_TRAIL_LIFE); //ball trail (no point in trailing when it's not moving!)
	}
	else if(Ball.movedelay > 0)
	{
		Ball.movedelay -= 1;
		//if(GameState.debug)
		//	printf("Delay: %d\n", Ball.movedelay);
	}
	
	//for now I'm using simple center-of-ball-meets-paddle collision detection (easy)
	//ill use (more accurate) rectangle based collision detection later
	for(i=0;i<2;i++)
	{
		if( (Ball.y + Ball.size / 2) > Paddle[i].y && (Ball.y + Ball.size / 2) < (Paddle[i].y + Paddle[i].height) )
		{
			if( (Ball.x + Ball.size / 2) > Paddle[i].x && (Ball.x + Ball.size / 2) < (Paddle[i].x + Paddle[i].width) )
			{
				Ball.xspeed = -(Ball.xspeed); //bounce off paddle
				Ball.x += (Ball.xspeed / 2); //fix "ball stuck in paddle" bug
				
				//set g,b paddle values down for fade effect
				Paddle[i].g = 0; Paddle[i].b = 0; Paddle[i].ticks = PADDLE_FADETIME;
				
				//spawn some sparks
				BallSpark(BALL_HIT_SPARKS);
				
				Ball.yspeed += (Paddle[i].yspeed / 2); //so you can change ball trajectory with paddle
				if(Ball.yspeed > Ball.maxspeed)
					Ball.yspeed = Ball.maxspeed;
				else if(Ball.yspeed < -(Ball.maxspeed))
					Ball.yspeed = -(Ball.maxspeed);
				
				//if paddle is stationary on impact, ball.xspeed gets increased
				if(Paddle[i].yspeed == 0)
				{
					if(Ball.xspeed < 0)
					{
						Ball.xspeed--;
						if(Ball.xspeed < -(Ball.maxspeed))
							Ball.xspeed = -Ball.maxspeed;
					}	
					else if(Ball.xspeed > 0)
					{
						Ball.xspeed++;
						if(Ball.xspeed > Ball.maxspeed)
							Ball.xspeed = Ball.maxspeed;
					}
				}
			}
		}
	}
			
	//bounce off top/bottom walls
	if((Ball.y + Ball.size / 2) <= 0 || (Ball.y + Ball.size / 2) >= GameState.resy)
	{
		Ball.yspeed = -(Ball.yspeed);
		BallSpark(BALL_HIT_SPARKS);
	}
}

void GameRender(void)
{
	//draw bkg before anything else
	SDL_Rect offset,srcoffset;
	offset.x = 0;
	offset.y = 0;
	offset.w = GameState.resx;
	offset.h = GameState.resy;
	
	srcoffset.x = Background.x;
	srcoffset.y = Background.y;
	srcoffset.w = GameState.resx;
	srcoffset.h = GameState.resy;
	SDL_BlitSurface(Background.image, &srcoffset, screen, &offset );

	//draw paddles
	SDL_Rect paddledims[2];
	int i;
	for(i=0;i<2;i++)
	{
		paddledims[i].w = Paddle[i].width;
		paddledims[i].h = Paddle[i].height;
		paddledims[i].x = Paddle[i].x;
		paddledims[i].y = Paddle[i].y;
		SDL_FillRect(screen, &paddledims[i], SDL_MapRGB(screen->format, Paddle[i].r, Paddle[i].g, Paddle[i].b));
		
		//BoxRGBA is slow, use it minimally.
		
		//paddle outline
		rectangleRGBA(screen, Paddle[i].x, Paddle[i].y, Paddle[i].x + Paddle[i].width, Paddle[i].y + Paddle[i].height, 10,10,10,255);
	}
	
	//draw scores in upper window
	char scores[32];
	if(!(GameState.twoplayer))
		sprintf(scores,"%d - %d",Paddle[0].score,Paddle[1].score);
	else
		sprintf(scores,"%d - %d",Paddle[0].score,Paddle[1].score);
		
	DrawText(scores,(GameState.resx / 2) - (strlen(scores) * 2),10); //middle of screen-ish
	
	//drawing a filled rect to fix the box color problem
	//maybe this and paddles can be drawn more efficiently?
	SDL_Rect balldims;
	balldims.x = Ball.x;
	balldims.y = Ball.y;
	balldims.w = Ball.size;
	balldims.h = Ball.size;
	SDL_FillRect(screen, &balldims, SDL_MapRGB(screen->format, 255, 255, 255));  //(screen->format, Ball.r, Ball.g, Ball.b));
	
	if(Ball.movedelay > 0)
	{
		char delaytext[64];
		sprintf(delaytext,"%2.2f",( (float)Ball.movedelay / (float)MAX_FPS ) );
		DrawText(delaytext,(GameState.resx / 2) - (strlen(delaytext) * 2),(GameState.resy / 2) + 12); //middle of screen-ish
	}
	
	if(GameState.debug) 
	{
		char debugtext[64];
		sprintf(debugtext, "Xs: %2.2f, Ys: %2.2f", Ball.xspeed, Ball.yspeed);
		DrawText(debugtext,Ball.x + Ball.size,Ball.y + Ball.size); 
	}
		
	if(GameState.paused)
		DrawText("Paused",(GameState.resx / 2) - (strlen("Paused") * 2),(GameState.resy / 2) + 32); //middle of screen-ish
}

void AIThink(void)
{	
	//this is a fairly simple AI. Not much is needed for pong.
	if((Paddle[0].x - Ball.x) <= AI.reactdist)
	{
		if((Ball.y + (Ball.size / 2)) > (Paddle[0].y + ((Paddle[0].height / 2) + AI.tolerance)))
			Paddle[0].yspeed = Paddle[0].speed;
		else if((Ball.y + (Ball.size / 2)) < (Paddle[0].y + ((Paddle[0].height / 2) - AI.tolerance)))
			Paddle[0].yspeed = -(Paddle[0].speed);
		else
			Paddle[0].yspeed = 0;
	}
	else
		Paddle[0].yspeed = 0;		
}

void InitGame()
{
	//init paddles
	Paddle[0].x = 16;
	Paddle[0].y = (GameState.resy / 2) - (Paddle[0].height / 2);
	
	Paddle[1].x = GameState.resx - (16 + Paddle[1].width); 
	Paddle[1].y = (GameState.resy / 2) - (Paddle[1].height / 2);
	
	GameState.winner = -1;
	
	ResetBall();
	
	SetGameState(STATE_PLAYING);
}

void ResetBall(void)
{
	Ball.x = (GameState.resx / 2) - (Ball.size / 2);
	Ball.y = (GameState.resy / 2) - (Ball.size / 2);
	Ball.movedelay = BALL_DELAY * MAX_FPS;
	
	
	//generate ball velocities
	int rneg;
	rneg = RandInt(1,10);
	if(rneg > 5)
		Ball.xspeed = RandFloat((Ball.maxspeed / 2),Ball.maxspeed - (Ball.maxspeed / 4));
	else if(rneg <= 5)
		Ball.xspeed = -(RandFloat((Ball.maxspeed / 2),Ball.maxspeed - (Ball.maxspeed / 4)));
		
	rneg = RandInt(1,10);
	if(rneg > 5)
		Ball.yspeed = RandFloat((Ball.maxspeed / 4),Ball.maxspeed / 3);
	else if(rneg <= 5)
		Ball.yspeed = -(RandFloat((Ball.maxspeed / 4),Ball.maxspeed / 3));
	
	if(GameState.debug)
		printf("Generated random ball speeds X=%2.2f, Y=%2.2f\n",Ball.xspeed,Ball.yspeed);
}

void GoalCheck(void)
{
	//check if ball meets back wall, score a point
	if((Ball.x + Ball.size / 2) < 0)
	{	
		Paddle[1].score++; //bot scores a pointer
		if(!(EndGameCheck()))
		{
			Firework(GameState.resx / 2, GameState.resy / 2, \
					RandInt(1, 255), RandInt(1, 255), RandInt(1, 255), \
					255, 200);
			InitGame(); //reset ball and such
		}
		else
		{
			SetGameState(STATE_MENU); //new state needed for end game screen
			InitMenu();
			puts("End game"); //end game screen
		}
	}
	else if((Ball.x + Ball.size / 2) > GameState.resx)
	{
		Paddle[0].score++; //player scores a pointer
		if(!(EndGameCheck()))
		{
			Firework(GameState.resx / 2, GameState.resy / 2, \
					RandInt(1, 255), RandInt(1, 255), RandInt(1, 255), \
					255, 200);
			InitGame(); //reset ball and such
		}
		else
		{
			SetGameState(STATE_MENU); //new state needed for end game screen
			InitMenu();
			puts("End game"); //end game screen
		}
	}
}

int EndGameCheck(void)
{
	int i = 0;
	int endgame = 0;
	for(i=0;i<2;i++)
	{
		if(Paddle[i].score >= MAX_SCORE)
		{
			GameState.winner = i;
			return 1;
			break;
		}
		else if(Paddle[i].score < MAX_SCORE)
			endgame = 0;
	}
	return endgame;
}


//#####################################
//Game Effect (FX) Functions
//#####################################

void LoadBackgrounds(void)
{
	Background.numimg = DetectJPGs("./img");
	
	int i;
	for(i=1;i<=Background.numimg;i++)
	{
		char imgname[64];
		sprintf(imgname,"./img/%d.jpg", i);
		if(!(Background.Backgrounds[i] = LoadImage(imgname)))
		{
			puts("Loading background image failed!");
   			printf("imgname=%s\n",imgname);
   			GameQuit();
		}
		UpdateProgressBar(0, 75 / Background.numimg);
	}
	InitBackground();
}

void InitBackground(void)
{		
	int imgnum;
	imgnum = RandInt(1, Background.numimg);
	//SDL_FreeSurface(Background.image); 
	//dont need to free because it deletes surface in Backgrounds[]
	printf("Switching background to: %d\n", imgnum);
	Background.image = Background.Backgrounds[imgnum];
    	
	Background.xspeed = RandFloat(-BACKGROUND_SPEED,BACKGROUND_SPEED);
	Background.yspeed = RandFloat(-BACKGROUND_SPEED,BACKGROUND_SPEED);
	
	//this of course assumes background dimensions are even numbers
	Background.x = (Background.image->w / 2);
	Background.y = (Background.image->h / 2);
}

void BallSpark(int numsparks)
{
	//spawn some sparks
	int k;
	float xmod,ymod;
	s_sprite Tmp;
	for(k=0;k<numsparks;k++)
	{
		//FIXME: when yspeed is 0, sparks spawn in a straight line. Result of bug fix below				
		xmod = RandFloat(1,abs(Ball.xspeed)); //random() % (Ball.xspeed) + 1;
		
		//bug fix pt.1 (avoid divide by 0)
		if(Ball.yspeed != 0)
			ymod = RandFloat(1,abs(Ball.yspeed)); //random() % (Ball.yspeed) + 1;
		else
			ymod = 0;
					
		if(Ball.xspeed > 0)
			Tmp.xspeed = xmod;
		else if(Ball.xspeed < 0)
			Tmp.xspeed = -xmod;
						
		if(Ball.yspeed > 0)
			Tmp.yspeed = ymod;
		else if(Ball.yspeed < 0)
			Tmp.yspeed = -ymod;
		else
			Tmp.yspeed = 0; //set to 0 on this rare occasion (bug fix pt.2)
					
		Tmp.type = SPRITE_SPARK;
		Tmp.x = Ball.x;
		Tmp.y = Ball.y;
		Tmp.r = 255; Tmp.g = 255; Tmp.b = 0;
		Tmp.a = 255;
		
		//Tmp.lifespan = (random() % 12) + 9;
		Tmp.lifespan = RandInt(8,16);
		Tmp.alphafade = 1;
		Tmp.faderate = (255 / Tmp.lifespan) - 1;
		SpawnSprite(Tmp);
	}
}

void BallTrail(int lifespan)
{
	s_sprite Tmp;
	Tmp.type = SPRITE_FILLEDRECT;
	Tmp.x = Ball.x;
	Tmp.y = Ball.y;
	Tmp.xspeed = 0;
	Tmp.yspeed = 0;
	Tmp.w = Ball.size;
	Tmp.h = Ball.size;
	Tmp.r = 255; Tmp.g = 255; Tmp.b = 255; Tmp.a = 255;
	Tmp.lifespan = lifespan;
	Tmp.alphafade = 1;
	Tmp.faderate = (int)(255 / lifespan);
	SpawnSprite(Tmp);
}

void ManageBackground(void)
{
	Background.x += Background.xspeed;
	Background.y += Background.yspeed;
	
	//bounce off walls
	if(Background.x <= 0 || (Background.x + GameState.resx) >= Background.image->w)
	{
		Background.xspeed = -(Background.xspeed);
	}
	
	if(Background.y <= 0 || Background.y + GameState.resy >= Background.image->h)
	{
		Background.yspeed = -(Background.yspeed);
	}
}

void PaddleEffects(void)
{
	int i;
	for(i=0;i<2;i++)
	{
		if(Paddle[i].ticks > 0)
		{
			Paddle[i].r += ((Paddle[i].defr - Paddle[i].r) / Paddle[i].ticks);
			Paddle[i].g += ((Paddle[i].defb - Paddle[i].g) / Paddle[i].ticks);
			Paddle[i].b += ((Paddle[i].defb - Paddle[i].b) / Paddle[i].ticks);
			Paddle[i].ticks--;
		}
	}
}

void Firework(float x, float y, int r, int g, int b, int a, int size)
{
	int i;
	for(i=1;i<=size;i++)
	{
		s_sprite Tmp;
		Tmp.type = SPRITE_SPARK;
		Tmp.x = x;
		Tmp.y = y;
		Tmp.r = r; Tmp.g = g; Tmp.b = b; Tmp.a = a;
		Tmp.xspeed = RandFloat(-3, 3);
		Tmp.yspeed = RandFloat(-3, 3);
	
		Tmp.lifespan = RandInt(10,42);
		Tmp.alphafade = 1;
		Tmp.faderate = (255 / Tmp.lifespan) - 1;
		SpawnSprite(Tmp);
	}
}
