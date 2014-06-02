#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "Render.h"
#include <time.h>
#include <sstream>
#include <string>
#include "text.h"
#include "util.h"
#include "constants.h"
#include "gem.h"



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


int** initGrid(gem* pGems)
{
	srand(time(NULL));

	int** pGrid = new int*[NUM_ROWS];
	int gridCount = 0;

	for (int row = 0; row < NUM_ROWS; row++)
	{
		pGrid[row] = new int[NUM_COLUMNS];

		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			pGrid[row][column] = gridCount;
			pGems[pGrid[row][column]].type = rand() % NUM_GEM_TYPES;
			pGems[pGrid[row][column]].y = 0;
			pGems[pGrid[row][column]].x = 0;
			pGems[pGrid[row][column]].row = row;
			pGems[pGrid[row][column]].column = column;
			pGems[pGrid[row][column]].velocity = 1;
			pGems[pGrid[row][column]].prevState = -1;
			pGems[pGrid[row][column]].dropToRow = row;

			gridCount++;
		}
	}


	return pGrid;
}


void addScore(int numGemsInMatch, int* pScore)
{
	*pScore += 100 * numGemsInMatch;
}


bool checkMatchAllColumns(int** pGrid, gem* pGems, int* pScore)
{
	bool foundMatch = false;

	for (int row = 0; row < NUM_ROWS-2; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			int gemToCompare = pGems[pGrid[row][column]].type;
			if (gemToCompare != -1)
			{
				// Now check for at least 2 adjacent matching gems below
				if (row < NUM_ROWS-2 && 
					pGems[pGrid[row+1][column]].type == gemToCompare && 
					pGems[pGrid[row+2][column]].type == gemToCompare)
				{
					// Continue marking matching gems until reached a different gem
					bool matchColumn = true;
					int j = 1;
					while (matchColumn && row+j < NUM_ROWS)
					{
						if (pGems[pGrid[row+j][column]].type == gemToCompare)
						{
							pGems[pGrid[row+j][column]].type = -1;
							j++;
						}
						else
						{
							matchColumn = false;
						}
					}
					pGems[pGrid[row][column]].type = -1;	// Get rid of origin gem
					foundMatch = true;
					addScore (j, pScore);
				}
			}
		}
	}

	return foundMatch;
}

bool checkMatchAllRows(int** pGrid, gem* pGems, int* pScore)
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


	bool foundMatch = false;
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			int gemToCompare = pGems[tempGridRow[row][column]].type;

			if (gemToCompare != -1)
			{
				// Now check for at least 2 adjacent matching gems to the right
				if (pGems[tempGridRow[row][column+1]].type == gemToCompare && 
					pGems[tempGridRow[row][column+2]].type == gemToCompare)
				{
					// Continue marking matching gems until reached a different gem
					bool matchRow = true;
					int j = 1;
					while (matchRow && column+j < NUM_COLUMNS)
					{
						if (pGems[tempGridRow[row][column+j]].type == gemToCompare)
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
					foundMatch = true;
					addScore (j, pScore);
				}
			}
		}
	}

	bool foundMatchColumns = checkMatchAllColumns(pGrid, pGems, pScore);

	// Merge result of deleted colums with result of deleted rows
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			if (tempGridRow[row][column] == -1)
			{
				pGems[pGrid[row][column]].type = -1;
			}
		}
	}

	if (foundMatch || foundMatchColumns)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool checkDrop(int** pGrid, gem* pGems)
{

	bool shouldDrop = false;

	// Check if there is an empty space in each column
	for (int column = 0; column < NUM_COLUMNS; column++)
	{
		int row = NUM_ROWS-1;
		bool foundEmpty = false;
		while (!foundEmpty && row >= 0)
		{
			if (pGems[pGrid[row][column]].type == -1)
			{
				foundEmpty = true;
				// Found a blank space so mark all gems above to drop
				int nextFreeRow = 0;
				for (int i = row; i >= 0; i--)
				{
					if (pGems[pGrid[i][column]].type != -1)
					{
						shouldDrop = true;
						pGems[pGrid[i][column]].state = GEMSTATE_FALL;
						pGems[pGrid[i][column]].prevState = GEMSTATE_FALL;
						pGems[pGrid[i][column]].dropToRow = row - nextFreeRow;
						nextFreeRow++;
					}
				}
			}
			row--;
		}
	}

	return shouldDrop;
}


void dropGem(int** pGrid, gem* pThisGem, gem* pGems)
{
	pThisGem->y += pThisGem->velocity;		
	pThisGem->velocity++;		

	if (pThisGem->velocity > MAX_DROP_SPEED)
	{
		pThisGem->velocity = MAX_DROP_SPEED;
	}

	if (pThisGem->y >= ROW_HEIGHT && pThisGem->row < NUM_ROWS-1)
	{
		pThisGem->y = 0;

		// Move this gem down to next row
		if (pThisGem->state != GEMSTATE_ENTER)
		{
			int temp = pGrid[pThisGem->row+1][pThisGem->column];
			pGrid[pThisGem->row+1][pThisGem->column] = pGrid[pThisGem->row][pThisGem->column];
			pGrid[pThisGem->row][pThisGem->column] = temp;
		}
		
		if (pThisGem->dropToRow > 0)
		{
			pThisGem->row++;
		}

		// Make gem stop falling if there is a gem below or it has reached bottom of grid
		if (pThisGem->row == NUM_ROWS-1 || pThisGem->row == pThisGem->dropToRow)
		{			
			pThisGem->state = GEMSTATE_BOUNCE;
			pThisGem->velocity = pThisGem->velocity/2;
		}
	}
		
}


int checkSwapGems(int* gemPosArray)
{
	// Check if the second gem is one space from first
	// int horizontalDist = gemPosArray[0][COLUMN] - gemPosArray[1][COLUMN];
	// int verticalDist = gemPosArray[0][ROW] - gemPosArray[1][ROW];
	int horizontalDist = (*(gemPosArray+3)) - (*(gemPosArray+1));
	int verticalDist = (*(gemPosArray+2)) - (*gemPosArray);
	
	if (horizontalDist == 1 && verticalDist == 0)
	{
		return GAMESTATE_SWAP_RIGHT;
	}
	else if	(horizontalDist == -1 && verticalDist == 0)
	{
		return GAMESTATE_SWAP_LEFT;
	}
	else if (horizontalDist == 0 && verticalDist == 1)
	{
		return GAMESTATE_SWAP_DOWN;
	}
	else if (horizontalDist == 0 && verticalDist == -1)
	{
		return GAMESTATE_SWAP_UP;
	}
	else
	{
		return -1;
	}

}


int setSwapGems(int state, gem* pGemsArray, int** pGridArray, int row1, int column1, int row2, int column2)
{
	switch (state)
	{
		case GAMESTATE_SWAP_LEFT:
			// Move right gem left
			pGemsArray[pGridArray[row1][column1]].velocity = -SWAP_SPEED;
			pGemsArray[pGridArray[row1][column1]].state = GEMSTATE_MOVE_LEFT;

			// Move left gem right
			pGemsArray[pGridArray[row2][column2]].velocity = SWAP_SPEED;
			pGemsArray[pGridArray[row2][column2]].state = GEMSTATE_MOVE_RIGHT;
			return GAMESTATE_SWAP_LEFT;
		break;

		case GAMESTATE_SWAP_RIGHT:
			// Move left gem right
			pGemsArray[pGridArray[row1][column1]].velocity = SWAP_SPEED;
			pGemsArray[pGridArray[row1][column1]].state = GEMSTATE_MOVE_RIGHT;

			// Move right gem left
			pGemsArray[pGridArray[row2][column2]].velocity = -SWAP_SPEED;
			pGemsArray[pGridArray[row2][column2]].state = GEMSTATE_MOVE_LEFT;
			return GAMESTATE_SWAP_RIGHT;

		break;

		case GAMESTATE_SWAP_UP:
			// Move bottom gem up
			pGemsArray[pGridArray[row1][column1]].velocity = -SWAP_SPEED;
			pGemsArray[pGridArray[row1][column1]].state = GEMSTATE_MOVE_UP;

			// Move right gem left
			pGemsArray[pGridArray[row2][column2]].velocity = SWAP_SPEED;
			pGemsArray[pGridArray[row2][column2]].state = GEMSTATE_MOVE_DOWN;
			return GAMESTATE_SWAP_UP;
		break;

		case GAMESTATE_SWAP_DOWN:
			// Move top gem down
			pGemsArray[pGridArray[row1][column1]].velocity = SWAP_SPEED;
			pGemsArray[pGridArray[row1][column1]].state = GEMSTATE_MOVE_DOWN;

			// Move bottom gem up
			pGemsArray[pGridArray[row2][column2]].velocity = -SWAP_SPEED;
			pGemsArray[pGridArray[row2][column2]].state = GEMSTATE_MOVE_UP;
			return GAMESTATE_SWAP_DOWN;
		break;

		default:
			return -1;
		break;
	}
}


bool checkMatchAfterSwap(int** pGrid, gem* pGems, gem* pThisGem, int* pScore)
{
	bool foundMatch = false;

	// First check horizontal matches
	int direction = -1;
	int numHorizontalMatches[2] = {0,0}; // {Matches left, matches right}
	int row = pThisGem->row;
	for (int numMatchesIndex = 0; numMatchesIndex < 2; numMatchesIndex++)
	{
		// On first iteration check for at least 2 matching gems to the left of the gem
		bool match = true;
		int compareColumn = pThisGem->column + direction;
		while (match)
		{
			if (compareColumn >= 0 && compareColumn < NUM_COLUMNS)
			{
				if (pGems[pGrid[row][compareColumn]].type == pThisGem->type)
				{
					numHorizontalMatches[numMatchesIndex]++;
					compareColumn += direction;
				}
				else
				{
					match = false;
				}
			}
			else
			{
				match = false;
			}
		}

		// Set to compare gems to the right for second iteration of loop
		direction = 1;
	}

	// Delete matching gems
	if (numHorizontalMatches[0] + numHorizontalMatches[1]  >= 2)
	{
		for (int column = pThisGem->column - numHorizontalMatches[0]; column <= pThisGem->column + numHorizontalMatches[1]; column++)
		{
			// Mark them for deletion
			if (!(row == pThisGem->row && column == pThisGem->column))	// Keep origin gem to check vertical match
			{
				pGems[pGrid[row][column]].type = -1;
			}
		}
		foundMatch = true;
		addScore(numHorizontalMatches[0] + numHorizontalMatches[1] + 1, pScore);
	}

	// Now check vertical matches
	direction = -1;
	int numVerticalMatches[2] = {0,0};	// {Matches above, matches below}
	int column = pThisGem->column;
	for (int numMatchesIndex = 0; numMatchesIndex < 2; numMatchesIndex++)
	{
		// On first iteration check for at least 2 matching gems above
		bool match = true;
		int compareRow = pThisGem->row + direction;
		while (match)
		{
			if (compareRow >= 0 && compareRow < NUM_ROWS)
			{
				if (pGems[pGrid[compareRow][column]].type == pThisGem->type)
				{
					numVerticalMatches[numMatchesIndex]++;
					compareRow += direction;
				}
				else
				{
					match = false;
				}
			}
			else
			{
				match = false;
			}
		}

		// Set to compare gems below for second iteration of loop
		direction = 1;
	}

	// Delete matching gems
	if (numVerticalMatches[0] + numVerticalMatches[1]  >= 2)
	{
		for (int row = pThisGem->row - numVerticalMatches[0]; row <= pThisGem->row + numVerticalMatches[1]; row++)
		{
			// Mark them for deletion
			pGems[pGrid[row][column]].type = -1;
		}
		
		foundMatch = true;
		addScore(numVerticalMatches[0] + numVerticalMatches[1] + 1, pScore);
	}
	
	if (foundMatch)
	{
		// Delete origin gem if found a match
		pThisGem->type = -1;
	}

	return foundMatch;
}



int main(int argc, char **argv){
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		logSDLError(std::cout, "SDL_Init");
		return 1;
	}

	if (TTF_Init() != 0){
	logSDLError(std::cout, "TTF_Init");
	return 1;
	}

	SDL_Window *window = SDL_CreateWindow("Hello World!", 100, 100, SCREEN_WIDTH, GRID_HEIGHT + HUD_HEIGHT,
	SDL_WINDOW_SHOWN);
	if (window == nullptr){
		logSDLError(std::cout, "CreateWindow");
		SDL_Quit();
		return 2;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr){
		logSDLError(std::cout, "CreateRenderer");
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 3;
	}

	//The textures we'll be using
	SDL_Texture *background = loadTexture("res/game_bg.jpg", renderer);
	SDL_Texture *image = loadTexture("res/Blue.png", renderer);
	SDL_Texture *blueGem = loadTexture("res/Blue.png", renderer);
	SDL_Texture *greenGem = loadTexture("res/Green.png", renderer);
	SDL_Texture *purpleGem = loadTexture("res/Purple.png", renderer);
	SDL_Texture *redGem = loadTexture("res/Red.png", renderer);
	SDL_Texture *yellowGem = loadTexture("res/Yellow.png", renderer);
	SDL_Texture *gems[] = {blueGem, greenGem, purpleGem, redGem, yellowGem};
	SDL_Texture *cursorYellowImg = loadTexture("res/cursor_yellow.png", renderer);
	SDL_Texture *cursorGreenImg = loadTexture("res/cursor_green.png", renderer);
	//Make sure they both loaded ok
	if (background == nullptr || image == nullptr){
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 4;
	}

	//We'll render the string "TTF fonts are cool!" in white
	//Color is in RGB format
	SDL_Color color = { 255, 255, 255 };
	SDL_Texture *textImage;
	TTF_Font* font = loadFont("res/sample.ttf", 32);

	//Our event structure
	SDL_Event e;
	//For tracking if we want to quit
	bool quit = false;
	int gameState = GAMESTATE_TITLE_SCREEN;
	int** pGridArray;
	gem* pGemsArray = new gem[NUM_ROWS * NUM_COLUMNS];

	int score;

	int mouseX = 0;
	int mouseY = 0;
	int numGemsSelected = 0;
	int selectedGems[2][2] = {{-1, -1}, {-1, -1}};
	bool gemSelected = false;
	int cursorRow = 0;
	int cursorColumn = 0;
	bool foundMatch = false;
	int thisRow, thisColumn;
	bool gemsDropping;
	bool cancelSwap = false;
	bool mouseDown = false;
	int dragStartX;
	int dragStartY;
	bool dragStarted = false;
	time_t timer;
	time_t startTime;
	std::ostringstream os;

	while (!quit)
	{
		switch (gameState)
		{
			case GAMESTATE_TITLE_SCREEN:
			break;

			case GAMESTATE_INIT:
				for (int i = 0; i < NUM_ROWS*NUM_COLUMNS; i++)
				{
					pGemsArray[i].state = GEMSTATE_IDLE;
				}
				pGridArray = initGrid(pGemsArray);
				score = 0;
				startTime = time(0);
				gameState = GAMESTATE_INITIAL_CHECK_MATCH;
			break;

			case GAMESTATE_INITIAL_CHECK_MATCH:
				SDL_Delay(1000);
				checkMatchAllRows(pGridArray, pGemsArray, &score);
				gameState = GAMESTATE_CHECKDROP;
			break;

			case GAMESTATE_CHECK_MATCH:

				foundMatch = false;
				for (int row = NUM_ROWS-1; row >= 0; row--)
				{
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						if (pGemsArray[pGridArray[row][column]].prevState == GEMSTATE_FALL)
						{
							foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[row][column]], &score);
						}
					}
				}

				gameState = GAMESTATE_CHECKDROP;

				for (int row = NUM_ROWS-1; row >= 0; row--)
				{
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						pGemsArray[pGridArray[row][column]].prevState = -1;
					}
				}

			break;

			case GAMESTATE_AWAIT_INPUT:

				if (gemSelected)
				{
					if (numGemsSelected < 2)
					{
						numGemsSelected++;
					}
					else
					{
						numGemsSelected = 0;
					}
					cursorRow = (mouseY-HUD_HEIGHT) / ROW_HEIGHT;
					cursorColumn = mouseX / COLUMN_WIDTH;

					// Store the location of the selected gem
					selectedGems[numGemsSelected-1][ROW] = cursorRow;
					selectedGems[numGemsSelected-1][COLUMN] = cursorColumn; 

					gemSelected = false;

					// Check if ready to swap
					if (numGemsSelected == 2)
					{
						int state = checkSwapGems((int*)selectedGems);
						state = setSwapGems(state, pGemsArray, pGridArray, selectedGems[0][ROW], selectedGems[0][COLUMN], selectedGems[1][ROW], selectedGems[1][COLUMN]);
						if (state == -1)
						{

							// Remove all cursors
							numGemsSelected = 0;

							// Set new cursor to this position
							cursorRow = (mouseY-HUD_HEIGHT) / ROW_HEIGHT;
							cursorColumn = mouseX / COLUMN_WIDTH;

							// Store the location of the selected gem
							selectedGems[numGemsSelected-1][ROW] = cursorRow;
							selectedGems[numGemsSelected-1][COLUMN] = cursorColumn; 
							gemSelected = true;
						}
						else
						{
							gameState = state;
						}
					}
				}
				
			break;


			case GAMESTATE_SWAP_LEFT:
			case GAMESTATE_SWAP_RIGHT:

				thisRow = selectedGems[0][ROW];
				if (gameState == GAMESTATE_SWAP_LEFT)
					thisColumn = selectedGems[0][COLUMN];
				else
					thisColumn = selectedGems[1][COLUMN];

				pGemsArray[pGridArray[thisRow][thisColumn]].x += pGemsArray[pGridArray[thisRow][thisColumn]].velocity;
				pGemsArray[pGridArray[thisRow][thisColumn-1]].x += pGemsArray[pGridArray[thisRow][thisColumn-1]].velocity;
				
				if (pGemsArray[pGridArray[thisRow][thisColumn]].x <= -COLUMN_WIDTH)
				{
					numGemsSelected = 0;

					// Swap with other gem
					int temp = pGridArray[thisRow][thisColumn];
					pGridArray[thisRow][thisColumn] = pGridArray[thisRow][thisColumn-1];
					pGridArray[thisRow][thisColumn-1] = temp;

					pGemsArray[pGridArray[thisRow][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].x = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].column++;
					pGemsArray[pGridArray[thisRow][thisColumn]].state = GEMSTATE_IDLE;
					
					pGemsArray[pGridArray[thisRow][thisColumn-1]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn-1]].x = 0;
					pGemsArray[pGridArray[thisRow][thisColumn-1]].column--;
					pGemsArray[pGridArray[thisRow][thisColumn-1]].state = GEMSTATE_IDLE;

					bool foundMatch = false;
					bool foundMatch2 = false;
					if (!cancelSwap)
					{
						foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn]], &score);
						foundMatch2 = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn-1]], &score);
					}

					if (foundMatch || foundMatch2)
					{
						gameState = GAMESTATE_CHECKDROP;
					}
					else
					{
						if (cancelSwap)
						{
							// Gems have gone back to their original positions after cancelling the swap
							cancelSwap = false;
							gameState = GAMESTATE_AWAIT_INPUT;
						}
						else
						{
							// Swap the gems back to their original positions
							gameState = setSwapGems(gameState, pGemsArray, pGridArray, selectedGems[0][ROW], selectedGems[0][COLUMN], selectedGems[1][ROW], selectedGems[1][COLUMN]);
							cancelSwap = true;
						}
	
					}
				}					
			break;

			case GAMESTATE_SWAP_UP:
			case GAMESTATE_SWAP_DOWN:

				thisColumn = selectedGems[0][COLUMN];
				if (gameState == GAMESTATE_SWAP_UP)
					thisRow = selectedGems[0][ROW];
				else
					thisRow = selectedGems[1][ROW];

				pGemsArray[pGridArray[thisRow][thisColumn]].y += pGemsArray[pGridArray[thisRow][thisColumn]].velocity;
				pGemsArray[pGridArray[thisRow-1][thisColumn]].y += pGemsArray[pGridArray[thisRow-1][thisColumn]].velocity;
				
				if (pGemsArray[pGridArray[thisRow][thisColumn]].y <= -ROW_HEIGHT)
				{
					numGemsSelected = 0;
					
					// Swap with other gem
					int temp = pGridArray[thisRow][thisColumn];
					pGridArray[thisRow][thisColumn] = pGridArray[thisRow-1][thisColumn];
					pGridArray[thisRow-1][thisColumn] = temp;

					pGemsArray[pGridArray[thisRow][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].y = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].row++;
					pGemsArray[pGridArray[thisRow][thisColumn]].state = GEMSTATE_IDLE;

					pGemsArray[pGridArray[thisRow-1][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow-1][thisColumn]].y = 0;
					pGemsArray[pGridArray[thisRow-1][thisColumn]].row--;
					pGemsArray[pGridArray[thisRow-1][thisColumn]].state = GEMSTATE_IDLE;

					bool foundMatch = false;
					bool foundMatch2 = false;
					if (!cancelSwap)
					{
						foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn]], &score);
						foundMatch2 = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow-1][thisColumn]], &score);
					}

					if (foundMatch || foundMatch2)
					{
						gameState = GAMESTATE_CHECKDROP;
					}
					else
					{					
						if (cancelSwap)
						{
							// Gems have gone back to their original positions after cancelling the swap
							cancelSwap = false;
							gameState = GAMESTATE_AWAIT_INPUT;
						}
						else
						{
							// Swap the gems back to their original positions
							gameState = setSwapGems(gameState, pGemsArray, pGridArray, selectedGems[0][ROW], selectedGems[0][COLUMN], selectedGems[1][ROW], selectedGems[1][COLUMN]);
							cancelSwap = true;
						}
					}
				}					
			break;

			case GAMESTATE_CHECKDROP:

				if (checkDrop(pGridArray, pGemsArray))
				{
					gameState = GAMESTATE_DROP;
				}
				else
				{
					gameState = GAMESTATE_ADD_GEMS;
				}

			break;

			case GAMESTATE_DROP:

				// Check if any more gems are dropping
				gemsDropping = false;
				for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
				{
					if (pGemsArray[i].state == GEMSTATE_FALL || pGemsArray[i].state == GEMSTATE_ENTER || pGemsArray[i].state == GEMSTATE_BOUNCE)
					{
						gemsDropping = true;
						break;
					}
				}

				if (!gemsDropping)
				{
					gameState = GAMESTATE_CHECK_MATCH;
				}
			break;


			case GAMESTATE_GAME_OVER:

				if (timer - startTime >= GAME_OVER_TIMEOUT)
				{
					gameState = GAMESTATE_TITLE_SCREEN;
					startTime = time(0);
				}
			break;

			case GAMESTATE_ADD_GEMS:

				srand(time(NULL));
				bool gemsAdded = false;
				// Find empty columns
				for (int column = 0; column < NUM_COLUMNS; column++)
				{
					// Find empty spaces in this column starting from top
					int row = 0;
					
					while (pGemsArray[pGridArray[row][column]].type == -1)
					{
						pGemsArray[pGridArray[row][column]].type = rand() % NUM_GEM_TYPES;
						pGemsArray[pGridArray[row][column]].state = GEMSTATE_ENTER;
						pGemsArray[pGridArray[row][column]].row = 0;
						pGemsArray[pGridArray[row][column]].column = column;
						pGemsArray[pGridArray[row][column]].y = -(ROW_HEIGHT+10)*(NUM_ROWS - row);
						pGemsArray[pGridArray[row][column]].velocity = 1;
						pGemsArray[pGridArray[row][column]].dropToRow = row;
						pGemsArray[pGridArray[row][column]].prevState = GEMSTATE_FALL;
						row++;
						gemsAdded = true;
					}
				}

				numGemsSelected = 0;
				if (gemsAdded)
				{
					gameState = GAMESTATE_DROP;
				}
				else
				{
					gameState = GAMESTATE_AWAIT_INPUT;
				}
			break;

		}

		if (gameState != GAMESTATE_TITLE_SCREEN)
		{

			for (int row = NUM_ROWS-1; row >= 0; row--)
			{
				for (int column = 0; column < NUM_COLUMNS; column++)
				{
					gem* thisGem = &pGemsArray[pGridArray[row][column]];

					switch (pGemsArray[pGridArray[row][column]].state)
					{
						case GEMSTATE_IDLE:
						break;

						case GEMSTATE_FALL:
						case GEMSTATE_ENTER:

							if ((pGemsArray[pGridArray[row][column]].type != -1 && pGemsArray[pGridArray[row+1][column]].type == -1) ||
								pGemsArray[pGridArray[row][column]].state == GEMSTATE_ENTER)
							{
								dropGem(pGridArray, &pGemsArray[pGridArray[row][column]], pGemsArray);
							}
						break;

						case GEMSTATE_BOUNCE:

							thisGem->y += thisGem->velocity;
							thisGem->velocity--;		

							if (thisGem->y <= 0)
							{
								thisGem->y = 0;
								thisGem->velocity = 0;
								thisGem->state = GEMSTATE_IDLE;
							}
						break;
					}	
				}
			}
		}
	

		timer = time(0);

		if (gameState != GAMESTATE_TITLE_SCREEN && gameState != GAMESTATE_GAME_OVER && timer - startTime >= TIMEOUT)
		{
			gameState = GAMESTATE_GAME_OVER;
			startTime = time(0);
		}

		//Read user input & handle it
		//Read any events that occured, for now we'll just quit if any event occurs
		while (SDL_PollEvent(&e)){
			//If user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
			//If user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				if (gameState == GAMESTATE_TITLE_SCREEN)
				{
					gameState = GAMESTATE_INIT;
				}
				else
				{
					mouseX = e.button.x;
					mouseY = e.button.y;
					mouseDown = true;
					dragStartX = mouseX;
					dragStartY = mouseY;
					dragStarted = false;
				}
			}
			if (e.type == SDL_MOUSEBUTTONUP){
				if (gameState != GAMESTATE_TITLE_SCREEN)
				{
					mouseDown = false;
					gemSelected = true;
				}
			}
			if (e.type == SDL_MOUSEMOTION && mouseDown)
			{
				if (e.motion.xrel < 0)		// Dragged left
				{
					if (mouseX - e.motion.x > DRAG_DEAD_ZONE && !dragStarted)
					{
						// Start a new selection
						numGemsSelected = 0;
						dragStarted = true;
						gemSelected = true;
					}				
					 else if (mouseX - e.motion.x > DRAG_DEAD_ZONE && dragStarted)
					{
						// Select next gem
						mouseX -= COLUMN_WIDTH;
						mouseDown = false;
						gemSelected = true;
					}
				}
				else if (e.motion.xrel > 0)		// Dragged right
				{
					if (e.motion.x - mouseX > DRAG_DEAD_ZONE && !dragStarted)
					{
						numGemsSelected = 0;
						dragStarted = true;
						gemSelected = true;
					}				
					 else if (e.motion.x - mouseX > DRAG_DEAD_ZONE && dragStarted)
					{
						mouseX += COLUMN_WIDTH;
						mouseDown = false;
						gemSelected = true;
					}
				}
				else if (e.motion.yrel < 0)		// Dragged up
				{
					if (mouseY - e.motion.y > DRAG_DEAD_ZONE && !dragStarted)
					{
						numGemsSelected = 0;
						dragStarted = true;
						gemSelected = true;
					}				
					 else if (mouseY - e.motion.y > DRAG_DEAD_ZONE && dragStarted)
					{
						mouseY -= ROW_HEIGHT;
						mouseDown = false;
						gemSelected = true;
					}
				}
				else if (e.motion.yrel > 0)		// Dragged down
				{
					if (e.motion.y - mouseY > DRAG_DEAD_ZONE && !dragStarted)
					{
						numGemsSelected = 0;
						dragStarted = true;
						gemSelected = true;
					}				
					 else if (e.motion.y - mouseY > DRAG_DEAD_ZONE && dragStarted)
					{
						mouseY += ROW_HEIGHT;
						mouseDown = false;
						gemSelected = true;
					}
				}
			}
		}

		//Render scene

		//First clear the renderer
		SDL_RenderClear(renderer);

		//Get the width and height from the texture so we know how much to move x,y by
		//to tile it correctly
		int bW, bH;
		SDL_QueryTexture(background, NULL, NULL, &bW, &bH);

		int x;
		int y;
		//Draw the background
		renderTexture(background, renderer, 0, 0);

		if (gameState != GAMESTATE_TITLE_SCREEN && gameState != GAMESTATE_INIT)
		{
			// Draw the gems
			// int rowHeight = GRID_HEIGHT / NUM_ROWS;
			int gemWidth, gemHeight;
			SDL_QueryTexture(gems[0], NULL, NULL, &gemWidth, &gemHeight);

			// Draw cursors
			if (numGemsSelected > 0)
			{
				int cursorWidth, cursorHeight;
				SDL_QueryTexture(cursorYellowImg, NULL, NULL, &cursorWidth, &cursorHeight);
				renderTexture(cursorYellowImg, renderer, 
					selectedGems[0][COLUMN] * COLUMN_WIDTH + COLUMN_WIDTH/2 - cursorWidth/2, 
					selectedGems[0][ROW] * ROW_HEIGHT + ROW_HEIGHT/2 - cursorHeight/2 + HUD_HEIGHT);
			
				if (numGemsSelected == 2)
				{
					renderTexture(cursorGreenImg, renderer, 
						selectedGems[1][COLUMN] * COLUMN_WIDTH + COLUMN_WIDTH/2 - cursorWidth/2, 
						selectedGems[1][ROW] * ROW_HEIGHT + ROW_HEIGHT/2 - cursorHeight/2 + HUD_HEIGHT);
				}
			}

			for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
			{
				if (pGemsArray[i].type != -1)
				{
					x = COLUMN_WIDTH * pGemsArray[i].column + COLUMN_WIDTH/2 - gemWidth/2;
					y = ROW_HEIGHT * pGemsArray[i].row + ROW_HEIGHT/2 - gemHeight/2 + HUD_HEIGHT;
					renderTexture(gems[pGemsArray[i].type], renderer, x + pGemsArray[i].x, y + pGemsArray[i].y);
				}
			}
		}

		
		// Draw text
		int iW, iH;
		switch (gameState)
		{
			case GAMESTATE_TITLE_SCREEN:
		
			os.str("");
			os.clear();
			os << "Click mouse to start";

			textImage = renderText(os.str(), font, color, renderer);
			if (textImage == nullptr){
				return 1;
			}

			SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
			SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
			x = SCREEN_WIDTH/2 - iW/2;
			y = GRID_HEIGHT/2;
			renderTexture(textImage, renderer, x, y);

			break;

			case GAMESTATE_GAME_OVER:

				os.str("");
				os.clear();
				os << "GAME OVER. Your score is " << score;

				textImage = renderText(os.str(), font, color, renderer);
				if (textImage == nullptr){
					return 1;
				}

				SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
				SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
				x = SCREEN_WIDTH/2 - iW/2;
				y = GRID_HEIGHT/2;
				renderTexture(textImage, renderer, x, y);

			break;

			default:

				os.str("");
				os.clear();
				os << "Score " << score;

				textImage = renderText(os.str(), font, color, renderer);
				if (textImage == nullptr){
					return 1;
				}
				x = 10;
				y = 10;
				renderTexture(textImage, renderer, x, y);
				// drawText("Score ", score, renderer, textImage, font, color, 10, 10);


				os.str("");
				os.clear();
				os << "Time " << (TIMEOUT - (timer - startTime));
				textImage = renderText(os.str(), font, color, renderer);
				if (textImage == nullptr){
					return 1;
				}
				//Get the texture w/h so we can position it correctly on the screen
				SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
				SDL_QueryTexture(textImage, NULL, NULL, &iW, &iH);
				x = SCREEN_WIDTH -10 - iW;
				renderTexture(textImage, renderer, x, y);

				// drawText seems to always draw to the left of the screen so commented out
				// drawText("Time ", timer - startTime, renderer, textImage, font, color, 100, 10);

			break;
		}

		//Update the screen
		SDL_RenderPresent(renderer);
	}

	//Clean up our objects and quit
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