#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
//#include "Render.h"

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int NUM_ROWS = 8;
const int NUM_COLUMNS = 8;

int xOffset = 0;
int yOffset = 50;

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}


/*
 * Loads an image into a texture on the rendering device
 * @param file The image file to load
 * @param ren The renderer to load the texture onto
 * @return the loaded texture, or nullptr if something went wrong.
 */
SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren){
	SDL_Texture *texture = IMG_LoadTexture(ren, file.c_str());
	if (texture == nullptr){	
		logSDLError(std::cout, "LoadTexture");
	}
	return texture;
}


/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
 * width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 * @param w The width of the texture to draw
 * @param h The height of the texture to draw
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h){
	//Setup the destination rectangle to be at the position we want
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	dst.w = w;
	dst.h = h;
	SDL_RenderCopy(ren, tex, NULL, &dst);
}

/*
 * Draw an SDL_Texture to an SDL_Renderer at position x, y, preserving
 * the texture's width and height
 * @param tex The source texture we want to draw
 * @param rend The renderer we want to draw too
 * @param x The x coordinate to draw too
 * @param y The y coordinate to draw too
 */
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y){
	int w, h;
	SDL_QueryTexture(tex, NULL, NULL, &w, &h);
	renderTexture(tex, ren, x, y, w, h);
}


int main(int argc, char **argv){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Hello World!", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT,
	SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 2;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		// cleanup(window);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 3;
	}

	//The textures we'll be using
	SDL_Texture *background = loadTexture("res/asteroid_texture.bmp", renderer);
	SDL_Texture *image = loadTexture("res/Blue.png", renderer);
	SDL_Texture *blueGem = loadTexture("res/Blue.png", renderer);
	SDL_Texture *greenGem = loadTexture("res/Green.png", renderer);
	SDL_Texture *purpleGem = loadTexture("res/Purple.png", renderer);
	SDL_Texture *redGem = loadTexture("res/Red.png", renderer);
	SDL_Texture *yellowGem = loadTexture("res/Yellow.png", renderer);
	SDL_Texture *gems[] = {blueGem, greenGem, purpleGem, redGem, yellowGem};
	//Make sure they both loaded ok
	if (background == nullptr || image == nullptr){
		// cleanup(background, image, renderer, window);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 4;
	}

	//Our event structure
	SDL_Event e;
	//For tracking if we want to quit
	bool quit = false;
	//Render* gameRenderer = new Render();
	int gridArray[NUM_ROWS][NUM_COLUMNS] = 
						  {{0,2,0,0,0,0,0,0},
						   {1,0,0,0,0,0,0,0},
						   {0,0,0,3,0,0,0,0},
						   {0,0,4,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0}};


	while (!quit){
		//Read user input & handle it
		//Read any events that occured, for now we'll just quit if any event occurs
		while (SDL_PollEvent(&e)){
			//If user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
			//If user presses any key
			if (e.type == SDL_KEYDOWN){
				quit = true;
			}
			//If user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				quit = true;
			}
		}
		//Render our scene

		//Now lets draw our image
		//First clear the renderer
		SDL_RenderClear(renderer);

		//Get the width and height from the texture so we know how much to move x,y by
		//to tile it correctly
		int bW, bH;
		SDL_QueryTexture(background, NULL, NULL, &bW, &bH);

		//Draw the tiles by calculating their positions
		for (int i = 0; i < bW * bH; ++i){
			int x = i % bW;
			int y = i / bW;
			renderTexture(background, renderer, x * bW, y * bH);
		}

		// Draw the gems
		int columnWidth = SCREEN_WIDTH / (NUM_COLUMNS+1);
		for (int row = 0; row < NUM_ROWS; row++)
		{
			int y = (SCREEN_HEIGHT / NUM_ROWS) * row;

			for (int column = 0; column < NUM_COLUMNS; column++)
			{
				int x = columnWidth * column + columnWidth/2;

				//if (gridArray[row][column] == 0)
				{
					renderTexture(gems[gridArray[row][column]], renderer, x, y);
				}
			}
		}

		xOffset++;
		//Draw our image in the center of the window
		//We need the foreground image's width to properly compute the position
		//of it's top left corner so that the image will be centered
		int iW, iH;
		SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
		int x = SCREEN_WIDTH / 2 - iW / 2;
		int y = SCREEN_HEIGHT / 2 - iH / 2;
		renderTexture(image, renderer, x+xOffset, y);

		//Draw the texture
		//SDL_RenderCopy(ren, tex, NULL, NULL);
		//Update the screen
		SDL_RenderPresent(renderer);

	}
	//Have the program wait for 2000ms so we get a chance to see the screen
	//SDL_Delay(2000);

	//Clean up our objects and quit
	//gameRenderer = NULL;
	SDL_DestroyTexture(background);
	SDL_DestroyTexture(image);
	SDL_DestroyTexture(blueGem);
	SDL_DestroyTexture(greenGem);
	SDL_DestroyTexture(purpleGem);
	SDL_DestroyTexture(redGem);
	SDL_DestroyTexture(yellowGem);

	for (int i = 0; i < sizeof(*gems); i++)
	{
		gems[i] = NULL;
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}