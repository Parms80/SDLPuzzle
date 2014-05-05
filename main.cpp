#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
// #include "Render.h"

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int NUM_ROWS = 8;
const int NUM_COLUMNS = 8;

const int GAMESTATE_INIT = 0;
const int GAMESTATE_CHECKDROP = 1;
const int GAMESTATE_DROP = 2;
const int GAMESTATE_AWAIT_INPUT = 3;

const int GEMSTATE_IDLE = 0;
const int GEMSTATE_FALL = 1;
const int GEMSTATE_DEAD = 2;

// int xOffset = 0;
// int yOffset = 50;


struct gem
{
	float x;
	float y;
	float velocity;
	int type;
	int state;
	int row;
	int column;
};

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


int** initGrid(gem* pGems)
{
	int columnWidth = SCREEN_WIDTH / NUM_COLUMNS;
	int rowHeight = SCREEN_HEIGHT / NUM_ROWS;
	int testGridArray[NUM_ROWS][NUM_COLUMNS] = 
						  {{1,2,0,0,0,0,0,0},
						   {1,0,0,0,0,0,0,0},
						   {1,0,0,3,3,3,0,0},
						   {0,0,4,0,0,3,0,0},
						   {0,0,0,0,0,3,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,2,2,2}};

	int** pGrid = new int*[NUM_ROWS];

	for (int row = 0; row < NUM_ROWS; row++)
	{
		pGrid[row] = new int[NUM_COLUMNS];

		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			pGrid[row][column] = rand() % 5;
			// pGrid[row][column] = testGridArray[row][column];
			pGems[row*NUM_ROWS + column*NUM_COLUMNS].type = pGrid[row][column];
			pGems[row*NUM_ROWS + column*NUM_COLUMNS].x = row * rowHeight;
			pGems[row*NUM_ROWS + column*NUM_COLUMNS].y = column * columnWidth;
		}
	}

	std::cout << "initGrid\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[row*NUM_ROWS + column*NUM_COLUMNS].type << ",";
		}
		std::cout << "]\n";
	}

	return pGrid;
}


void checkMatchColumn(int** pGrid, int row, int column)
{
	int gemToCompare = pGrid[row][column];
	// Now check for at least 2 adjacent matching gems below
	if (row < NUM_ROWS-2 && 
		pGrid[row+1][column] == gemToCompare && 
		pGrid[row+2][column] == gemToCompare)
	{
		// Continue marking matching gems until reached a different gem
		bool matchRow = true;
		int j = 1;
		while (matchRow && row+j < NUM_ROWS)
		{
			if (pGrid[row+j][column] == gemToCompare)
			{
				pGrid[row+j][column] = -1;
				j++;
			}
			else
			{
				matchRow = false;
			}
		}
		pGrid[row][column] = -1;	// Get rid of origin gem
	}
}
/*
void checkMatch(int** pGrid, int numGemTypes)
{
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			int gemToCompare = pGrid[row][column];
			if (gemToCompare != -1)
			{
				// Check for at least 2 adjacent matching gems to the right
				if (pGrid[row][column+1] == gemToCompare && pGrid[row][column+2] == gemToCompare)
				{
					// Continue marking matching gems until reached a different gem
					bool match = true;
					int i = 1;
					while (match && column+i < NUM_COLUMNS)
					{
						if (pGrid[row][column+i] == gemToCompare)
						{
							// Now check for at least 2 adjacent matching gems below
							// if (row < NUM_ROWS-2 && 
							// 	pGrid[row+1][column+i] == pGrid[row][column+i] && 
							// 	pGrid[row+2][column+i] == pGrid[row][column+i])
							// {
							// 	// Continue marking matching gems until reached a different gem
							// 	bool matchRow = true;
							// 	int j = 1;
							// 	while (matchRow && row+j < NUM_ROWS)
							// 	{
							// 		if (pGrid[row+j][column+i] == pGrid[row][column+i])
							// 		{
							// 			pGrid[row+j][column+i] = -1;
							// 			j++;
							// 		}
							// 		else
							// 		{
							// 			matchRow = false;
							// 		}
							// 	}
							// }
							checkMatchRow(pGrid, row, column+i);

							pGrid[row][column+i] = -1;
							i++;
						}
						else
						{
							match = false;
						}
					}
					pGrid[row][column] = -1;	// Get rid of origin gem
				}
				else
				{
					checkMatchRow(pGrid, row, column);
				}
			}
		}
	}
}
*/

void checkMatchAllColumns(int** pGrid)
{
	// int tempGridRow[NUM_ROWS][NUM_COLUMNS] = 
	// 					  {{0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0},
	// 					   {0,0,0,0,0,0,0,0}};
	// // Copy the grid to a temporary grid to mark column matches
	// std::copy(&pGrid[0][0], &pGrid[0][0]+NUM_ROWS*NUM_COLUMNS, &tempGridRow[0][0]);
	
	for (int row = 0; row < NUM_ROWS-2; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			int gemToCompare = pGrid[row][column];
			// Now check for at least 2 adjacent matching gems below
			if (row < NUM_ROWS-2 && 
				pGrid[row+1][column] == gemToCompare && 
				pGrid[row+2][column] == gemToCompare)
			{
				// Continue marking matching gems until reached a different gem
				bool matchColumn = true;
				int j = 1;
				while (matchColumn && row+j < NUM_ROWS)
				{
					if (pGrid[row+j][column] == gemToCompare)
					{
						pGrid[row+j][column] = -1;
						j++;
					}
					else
					{
						matchColumn = false;
					}
				}
				pGrid[row][column] = -1;	// Get rid of origin gem
			}
		}
	}
}

void checkMatchAllRows(int** pGrid)
{
	int tempGridRow[NUM_ROWS][NUM_COLUMNS] = 
						  {{0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0}};
	// Copy the grid to a temporary grid to mark column matches
	// std::copy(&pGrid[0][0], &pGrid[0][0]+NUM_ROWS*NUM_COLUMNS, &tempGridRow[0][0]);
	
	// std::copy not working properly so copy this way instead
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			tempGridRow[row][column] = pGrid[row][column];
		}
	}

	std::cout << "checkMatchAllRows\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << tempGridRow[row][column] << ",";
		}
		std::cout << "]\n";
	}

	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			int gemToCompare = tempGridRow[row][column];
			// Now check for at least 2 adjacent matching gems to the right
			if (
				tempGridRow[row][column+1] == gemToCompare && 
				tempGridRow[row][column+2] == gemToCompare)
			{
				// Continue marking matching gems until reached a different gem
				bool matchRow = true;
				int j = 1;
				while (matchRow && column+j < NUM_COLUMNS)
				{
					if (tempGridRow[row][column+j] == gemToCompare)
					{
						tempGridRow[row][column+j] = -1;
						j++;
					}
					else
					{
						matchRow = false;
					}
				}
				tempGridRow[row][column] = -1;	// Get rid of origin gem
			}
		}
	}
	std::cout << "tempGridRow\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << tempGridRow[row][column] << ",";
		}
		std::cout << "]\n";
	}

	checkMatchAllColumns(pGrid);

	std::cout << "After checkMatchAllColumns\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGrid[row][column] << ",";
		}
		std::cout << "]\n";
	}

	// Merge result of deleted colums with result of deleted rows
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			if (tempGridRow[row][column] == -1)
			{
				pGrid[row][column] = -1;
			}
		}
	}
	std::cout << "After merge\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGrid[row][column] << ",";
		}
		std::cout << "]\n";
	}
}

void checkMatch(int** pGrid)
{
	//int** tempGrid = new int*[NUM_ROWS];
	int tempGridRow[NUM_ROWS][NUM_COLUMNS] = 
						  {{0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0}};
	// memcpy(tempGridRow, pGrid, sizeof(int)*NUM_ROWS*NUM_COLUMNS);
	// Copy the grid to a temporary grid to mark row matches
	std::copy(&pGrid[0][0], &pGrid[0][0]+NUM_ROWS*NUM_COLUMNS, &tempGridRow[0][0]);

	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << tempGridRow[row][column] << ",";
		}
		std::cout << "]\n";
	}
/*
	for (int row = 0; row < NUM_ROWS; row++)
	{
		delete [] tempGrid[row];
	}
	delete [] tempGrid;
*/
	for (int row = 0; row < NUM_ROWS; row++)
	{
		int column = 0;
		while (column < NUM_COLUMNS)
		//for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			std::cout << "row = " << row << ", column = " << column << "\n";
			int gemToCompare = tempGridRow[row][column];
			int numMatches = 1;

			if (gemToCompare != -1)
			{
				std::cout << "1\n";
				// Continue marking matching gems until reached a different gem
				bool match = true;
				int i = 1;
				while (match && column+i < NUM_COLUMNS)
				{
					std::cout << "1.1: column+i = " << (column+i) << ", tempGridRow["<<row<<"]["<<(column+i)<<"] = "<<tempGridRow[row][column+i]<<
					", gemToCompare = "<<gemToCompare<<"\n";
					if (tempGridRow[row][column+i] == gemToCompare)
					{
						std::cout << "Match\n";
						numMatches++;
						i++;
						std::cout << "numMatches = " << numMatches << "\n";
					}
					else
					{
						std::cout << "No match\n";
						match = false;
					}
				}

				// Check and get rid of vertical matches connected to this matching row
				// std::cout << "1.2: numMatches = " << numMatches << "\n";
				// for (i = 0; i < numMatches; i++)
				// {
					// checkMatchRow(pGrid, row, column+i);
				// }

				std::cout << "2: numMatches = " << numMatches << "\n";
				// Mark all matching gems for removal
				if (numMatches >= 3)
				{
					//int j;
					for (i = 0; i < numMatches; i++)
					{
						tempGridRow[row][column+i] = -1;
					}
					column = column+i;

					std::cout << "3: column = " << column << "\n";
				}
				else
				{
					std::cout << "4: column = " << column << "\n";
					column += numMatches;
				}
			}
			else
			{
				// No gem here so move to next column
				std::cout << "4: column = " << column << "\n";
				column++;
			}
		}
	}

	checkMatchAllColumns(pGrid);
}


void checkDrop(int** pGrid, gem* pGems)
{
	// Check if there's an empty space below gem
	for (int row = 0; row < NUM_ROWS-1; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			if (pGrid[row+1][column] == -1)
			{
				pGems[row*NUM_ROWS + column*NUM_COLUMNS].state = GEMSTATE_FALL;
			}
		}
	}
}

void dropGems(int** pGrid, gem* pGems)
{
	/*
	for (int i = 0; i < NUM_ROWS*NUM_COLUMNS; i++)
	{
		if (pGems[i].state == GEMSTATE_FALL)
		{
			pGems[i].y++;
		}
	}
	*/
	for (int row = NUM_ROWS-2; row >= 0; row--)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			if (pGrid[row+1][column] == -1)
			{
				// Move this gem down to next row
				pGrid[row+1][column] = pGrid[row][column];
				pGrid[row][column] = -1;
			}
		}
	}
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
	int gameState = GAMESTATE_INIT;
	// Render* gameRenderer = new Render();
	/*int gridArray[NUM_ROWS][NUM_COLUMNS] = 
						  {{0,2,0,0,0,0,0,0},
						   {1,0,0,0,0,0,0,0},
						   {0,0,0,3,0,0,0,0},
						   {0,0,4,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0},
						   {0,0,0,0,0,0,0,0}};
	*/
	int** pGridArray;
	// int* pGemStateArray = nullptr;
	gem* pGemsArray = new gem[NUM_ROWS * NUM_COLUMNS];

	while (!quit){

		switch (gameState)
		{
			case GAMESTATE_INIT:
				for (int i = 0; i < NUM_ROWS*NUM_COLUMNS; i++)
				{
					// pGemStateArray[i] = GEMSTATE_IDLE;
					pGemsArray[i].state = GEMSTATE_IDLE;
				}
				pGridArray = initGrid(pGemsArray);
				checkMatchAllRows(pGridArray);
				gameState = GAMESTATE_AWAIT_INPUT;
			break;

			case GAMESTATE_AWAIT_INPUT:
				gameState = GAMESTATE_CHECKDROP;
			break;

			case GAMESTATE_CHECKDROP:
				checkDrop(pGridArray, pGemsArray);
				gameState = GAMESTATE_DROP;
			break;

			case GAMESTATE_DROP:
				dropGems(pGridArray, pGemsArray);
			break;
		}
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
		int columnWidth = SCREEN_WIDTH / NUM_COLUMNS;
		int rowHeight = SCREEN_HEIGHT / NUM_ROWS;
		int gemWidth, gemHeight;
		int x, y;
		SDL_QueryTexture(gems[0], NULL, NULL, &gemWidth, &gemHeight);
		for (int row = 0; row < NUM_ROWS; row++)
		{
			y = rowHeight * row + rowHeight/2 - gemHeight/2;

			for (int column = 0; column < NUM_COLUMNS; column++)
			{
				if (pGridArray[row][column] != -1)
				{
					x = columnWidth * column + columnWidth/2 - gemWidth/2;
					renderTexture(gems[pGridArray[row][column]], renderer, x, y);
				}
			}
		}
		/*
		for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
		{
			if (pGemsArray[i].state != GEMSTATE_DEAD)
			{
				renderTexture(gems[pGemsArray[i].type], renderer, pGemsArray[i].x, pGemsArray[i].y);
			}
		}
		*/

		// xOffset++;
		//Draw our image in the center of the window
		//We need the foreground image's width to properly compute the position
		//of it's top left corner so that the image will be centered
		// int iW, iH;
		// SDL_QueryTexture(image, NULL, NULL, &iW, &iH);
		// int x = SCREEN_WIDTH / 2 - iW / 2;
		// int y = SCREEN_HEIGHT / 2 - iH / 2;
		// renderTexture(image, renderer, x+xOffset, y);

		//Draw the texture
		//SDL_RenderCopy(ren, tex, NULL, NULL);
		//Update the screen
		SDL_RenderPresent(renderer);

	}
	//Have the program wait for 2000ms so we get a chance to see the screen
	//SDL_Delay(2000);

	//Clean up our objects and quit
	// gameRenderer = NULL;
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
		//SDL_DestroyTexture(gems[i]);
	}


	for (int row = 0; row < NUM_ROWS; row++)
	{
		delete [] pGridArray[row];
	}
	delete [] pGridArray;

	delete [] pGemsArray;

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}