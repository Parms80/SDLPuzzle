#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
// #include <SDL_ttf.h>
// #include "Render.h"
#include <time.h>

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int NUM_ROWS = 8;
const int NUM_COLUMNS = 8;
const int ROW_HEIGHT = SCREEN_HEIGHT / NUM_ROWS;
const int COLUMN_WIDTH = SCREEN_WIDTH / NUM_COLUMNS;
const int MAX_DROP_SPEED = 10;
const int SWAP_SPEED = 10;
const int NUM_GEM_TYPES = 5;
const int DRAG_DEAD_ZONE = 30;

const int GAMESTATE_INIT = 0;
const int GAMESTATE_CHECKDROP = 1;
const int GAMESTATE_DROP = 2;
const int GAMESTATE_AWAIT_INPUT = 3;
const int GAMESTATE_SWAP_LEFT = 4;
const int GAMESTATE_SWAP_RIGHT = 5;
const int GAMESTATE_SWAP_UP = 6;
const int GAMESTATE_SWAP_DOWN = 7;
const int GAMESTATE_CHECK_MATCH = 8;
const int GAMESTATE_INITIAL_CHECK_MATCH = 9;
const int GAMESTATE_SWAP_HORIZONTAL = 10;
const int GAMESTATE_ADD_GEMS = 11;

const int GEMSTATE_IDLE = 0;
const int GEMSTATE_FALL = 1;
const int GEMSTATE_DEAD = 2;
const int GEMSTATE_BOUNCE = 3;
const int GEMSTATE_MOVE_LEFT = 4;
const int GEMSTATE_MOVE_RIGHT = 5;
const int GEMSTATE_MOVE_UP = 6;
const int GEMSTATE_MOVE_DOWN = 7;
const int GEMSTATE_ENTER = 8;

const int ROW = 0;
const int COLUMN = 1;


struct gem
{
	float x;
	float y;
	float velocity;
	int type;
	int state;
	int row;
	int column;
	int prevState;
	int dropToRow;
};

/*
 * Log an SDL error with some error message to the output stream of our choice
 * @param os The output stream to write the message too
 * @param msg The error message to write, format will be msg error: SDL_GetError()
 */
void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}


/**
* Render the message we want to display to a texture for drawing
* @param message The message we want to display
* @param fontFile The font we want to use to render the text
* @param color The color we want the text to be
* @param fontSize The size we want the font to be
* @param renderer The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
*/
/*
SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr){
		logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}	
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr){
		TTF_CloseFont(font);
		logSDLError(std::cout, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr){
		logSDLError(std::cout, "CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}
*/

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
	int gridCount = 0;

	for (int row = 0; row < NUM_ROWS; row++)
	{
		pGrid[row] = new int[NUM_COLUMNS];

		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			// pGrid[row][column] = rand() % 5;
			// pGrid[row][column] = testGridArray[row][column];
			pGrid[row][column] = gridCount;
			pGems[pGrid[row][column]].type = rand() % NUM_GEM_TYPES;//testGridArray[row][column];
			pGems[pGrid[row][column]].y = 0;//row * rowHeight;
			pGems[pGrid[row][column]].x = 0;
			pGems[pGrid[row][column]].row = row;
			pGems[pGrid[row][column]].column = column;
			pGems[pGrid[row][column]].velocity = 1;
			pGems[pGrid[row][column]].prevState = -1;
			pGems[pGrid[row][column]].dropToRow = row;

			gridCount++;
		}
	}

/*
	std::cout << "initGrid pGems\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[pGrid[row][column]].type << ",";
		}
		std::cout << "]\n";
	}

	std::cout << "initGrid pGrid\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGrid[row][column] << ",";
		}
		std::cout << "]\n";
	}
	*/

	return pGrid;
}

/*
void checkMatchColumn(int** pGrid, gem* pGems, int row, int column)
{
	int gemToCompare = pGems[pGrid[row][column]].type;
	// Now check for at least 2 adjacent matching gems below
	if (row < NUM_ROWS-2 && 
		pGems[pGrid[row+1][column]].type == gemToCompare && 
		pGems[pGrid[row+2][column]].type == gemToCompare)
	{
		// Continue marking matching gems until reached a different gem
		bool matchRow = true;
		int j = 1;
		while (matchRow && row+j < NUM_ROWS)
		{
			if (pGems[pGrid[row+j][column]].type == gemToCompare)
			{
				pGems[pGrid[row+j][column]].type = -1;
				j++;
			}
			else
			{
				matchRow = false;
			}
		}
		pGems[pGrid[row][column]].type = -1;	// Get rid of origin gem
	}
}
*/
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


void addScore(int numGemsInMatch, int* pScore)
{

	*pScore += 100 * numGemsInMatch;

	std::cout << "Score: " << *pScore << "\n";
}


bool checkMatchAllColumns(int** pGrid, gem* pGems, int* pScore)
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

	// std::cout << "checkMatchAllRows\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[tempGridRow[row][column]].type << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

	bool foundMatch = false;
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			int gemToCompare = pGems[tempGridRow[row][column]].type;

			// std::cout << "checkMatchAllRows: gemToCompare = " << gemToCompare << ", row = " << row << ", column = " << column << "\n";

			if (gemToCompare != -1)
			{
				// Now check for at least 2 adjacent matching gems to the right
				if (pGems[tempGridRow[row][column+1]].type == gemToCompare && 
					pGems[tempGridRow[row][column+2]].type == gemToCompare)
				{
					// std::cout << "checkMatchAllRows: found matching row. row = " << row << ", column = " << column << "\n";
					// Continue marking matching gems until reached a different gem
					bool matchRow = true;
					int j = 1;
					while (matchRow && column+j < NUM_COLUMNS)
					{
						// std::cout << "checkMatchAllRows: j = " << j << "\n";

						if (pGems[tempGridRow[row][column+j]].type == gemToCompare)
						{
							// std::cout << "checkMatchAllRows: mark for deletion\n";
							tempGridRow[row][column+j] = -1;
							j++;
						}
						else
						{
							// std::cout << "checkMatchAllRows: matchRow = false\n";
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
	// std::cout << "Before checkMatchAllColumns\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << tempGridRow[row][column] << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

	bool foundMatchColumns = checkMatchAllColumns(pGrid, pGems, pScore);

	// std::cout << "After checkMatchAllColumns\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[pGrid[row][column]].type << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

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
	// std::cout << "After merge\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[pGrid[row][column]].type << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

	// std::cout << "checkMatchAllRows: foundMatch = " << foundMatch << ", foundMatchColumns = " << foundMatchColumns << "\n";
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
	// std::cout << "checkDrop\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[pGrid[row][column]].type << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

	bool shouldDrop = false;

	// Check if there is an empty space in each column
	for (int column = 0; column < NUM_COLUMNS; column++)
	{
		int row = NUM_ROWS-1;
		bool foundEmpty = false;
		while (!foundEmpty && row >= 0)
		{
			if (pGems[pGrid[row][column]].type == -1)// && pGems[pGrid[row][column]].state == GEMSTATE_IDLE)
			{
				// std::cout << "checkDrop: found empty at [" << row << "][" << column << "]\n";
				foundEmpty = true;
				// Found a blank space so mark all gems above to drop
				int nextFreeRow = 0;
				for (int i = row; i >= 0; i--)
				{
					if (pGems[pGrid[i][column]].type != -1)
					{
						shouldDrop = true;
						// std::cout << "checkDrop: mark [" << i << "][" << column << "] to fall\n";
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

	// std::cout << "checkDrop: shouldDrop = " << shouldDrop << "\n";
	return shouldDrop;
}


void dropGem(int** pGrid, gem* pThisGem, gem* pGems)
{
	// std::cout << "dropGem\n";
	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[pGrid[row][column]].dropToRow << ",";
	// 	}
	// 	std::cout << "]\n";
	// }
	// std::cout << "dropGem: pThisGem.state = " << pThisGem->state << "\n";
	pThisGem->y += pThisGem->velocity;		
	pThisGem->velocity++;		

	// std::cout << "dropGem: pThisGem.y = " << pThisGem->y << ", velocity = " << pThisGem->velocity << ", ROW_HEIGHT = " << ROW_HEIGHT <<"\n";

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
			// pGrid[row][column] = -1;
			pGrid[pThisGem->row][pThisGem->column] = temp;
		}
		// pGems[pGrid[pThisGem->row+1][pThisGem->column]].row = pThisGem->row;
		
		if (pThisGem->dropToRow > 0)
		{
			pThisGem->row++;
		}
		// std::cout << "dropGem: pThisGem->row = " << pThisGem->row << ", pThisGem->type = " << pThisGem->type << ", type below = " << pGems[pGrid[ pThisGem->row+1][pThisGem->column]].type
		// 	<< ", state = " << pGems[pGrid[ pThisGem->row][pThisGem->column]].state << ", state below = " << pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state 
		// 	<< ", drop to row = " << pThisGem->dropToRow << "\n";
		// Make gem stop falling if there is a gem below or it has reached bottom of grid
		if (pThisGem->row == NUM_ROWS-1 || 
			// (pGems[pGrid[ pThisGem->row+1][pThisGem->column]].type != -1 && pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state != GEMSTATE_FALL))
			 // (pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state == GEMSTATE_IDLE || pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state == GEMSTATE_BOUNCE)))
			pThisGem->row == pThisGem->dropToRow)
		{
			// std::cout << "dropToRow = " << pThisGem->dropToRow << "\n";
			// 	for (int row = 0; row < NUM_ROWS; row++)
			// 	{
			// 		std::cout << "[";
			// 		for (int column = 0; column < NUM_COLUMNS; column++)
			// 		{
			// 			std::cout << pGems[pGrid[row][column]].type << ",";
			// 		}
			// 		std::cout << "]\n";
			// 	}
			// std::cout <<  "dropGem: stopping gem. row = "<< pThisGem->row << ". column = " << pThisGem->column <<
			// ". This gem type = " << pGems[pGrid[ pThisGem->row][pThisGem->column]].type << 
			// ". pGrid[pThisGem] = " << pGrid[ pThisGem->row][pThisGem->column] << 
			// ". Gem type below = " << pGems[pGrid[ pThisGem->row+1][pThisGem->column]].type << "\n";
			
			pThisGem->state = GEMSTATE_BOUNCE;
			pThisGem->velocity = pThisGem->velocity/2;
		}
	}
		
}


// void swapGem(int** pGrid, gem* pThisGem, gem* pGems)
// {
// 	pGemsArray[pGridArray[row][column]].x += pGemsArray[pGridArray[row][column]].velocity;
// }

int checkSwapGems(int* gemPosArray)
{
	// Check if the second gem is one space from first
	// int horizontalDist = gemPosArray[0][COLUMN] - gemPosArray[1][COLUMN];
	// int verticalDist = gemPosArray[0][ROW] - gemPosArray[1][ROW];
	int horizontalDist = (*(gemPosArray+3)) - (*(gemPosArray+1));
	int verticalDist = (*(gemPosArray+2)) - (*gemPosArray);
	
	// std::cout << "checkSwapGems: horizontalDist = " << horizontalDist << " \n";
	// std::cout << "checkSwapGems: verticalDist = " << verticalDist << " \n";

	if (horizontalDist == 1 && verticalDist == 0)
	{
		// std::cout << "checkSwapGems: adjacent. Swap right \n";
		return GAMESTATE_SWAP_RIGHT;
	}
	else if	(horizontalDist == -1 && verticalDist == 0)
	{
		// std::cout << "checkSwapGems: adjacent. Swap left \n";
		return GAMESTATE_SWAP_LEFT;
	}
	else if (horizontalDist == 0 && verticalDist == 1)
	{
		// std::cout << "checkSwapGems: adjacent. Swap down \n";
		return GAMESTATE_SWAP_DOWN;
	}
	else if (horizontalDist == 0 && verticalDist == -1)
	{
		// std::cout << "checkSwapGems: adjacent. Swap up \n";
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
	// std::cout << "checkMatchAfterSwap\n";


	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	std::cout << "[";
	// 	for (int column = 0; column < NUM_COLUMNS; column++)
	// 	{
	// 		std::cout << pGems[pGrid[row][column]].type << ",";
	// 	}
	// 	std::cout << "]\n";
	// }

	// std::cout << "pThisGem: row = " << pThisGem->row << ", column = " << pThisGem->column << ", type = " << pThisGem->type << "\n";

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
				// std::cout << "checkMatchAfterSwap 2: compareColumn = " << compareColumn << "\n";
				// std::cout << "checkMatchAfterSwap: pGems[pGrid["<<row<<"]["<<compareColumn<<"]].type = " << pGems[pGrid[row][compareColumn]].type << "\n";
				if (pGems[pGrid[row][compareColumn]].type == pThisGem->type)
				{
					// pGems[pGrid[row][compareColumn]].type = -1;
					numHorizontalMatches[numMatchesIndex]++;
					compareColumn += direction;
					// std::cout << "match. direction = " << direction << ", numHorizontalMatches[" << numMatchesIndex << "] = " << numHorizontalMatches[numMatchesIndex] << "\n";
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

		// Delete matching gems
		// if (numHorizontalMatches[numMatchesIndex] >= 2)
		// {
		// 	for (int i = 0; i < numHorizontalMatches[numMatchesIndex]; i++)
		// 	{
		// 		pGems[pGrid[row][pThisGem->column + direction * (i+1)]].type = -1;
		// 	}
		// 	pGems[pGrid[row][pThisGem->column]].type = -1;
		// 	std::cout << "checkMatchAfterSwap: pGems[pGrid["<<row<<"]["<<pThisGem->column<<"]].type = " << pGems[pGrid[row][pThisGem->column]].type << "\n";
		// 	foundMatch = true;
		// }

		// Set to compare gems to the right for second iteration of loop
		direction = 1;
	}

	// Delete matching gems
	if (numHorizontalMatches[0] + numHorizontalMatches[1]  >= 2)
	{
		// std::cout << "Found horizontal match. Origin gem at row " << pThisGem->row << ", column " << pThisGem->column << "\n";
		for (int column = pThisGem->column - numHorizontalMatches[0]; column <= pThisGem->column + numHorizontalMatches[1]; column++)
		{
			// std::cout << "Before: column = " << column << ", pGems[pGrid["<<row<<"]["<<column<<"]].type = " << pGems[pGrid[row][column]].type << "\n";
			// Mark them for deletion
			if (!(row == pThisGem->row && column == pThisGem->column))	// Keep origin gem to check vertical match
			{
				// std::cout << "Delete gem at row " << row << ", column " << column << "\n";
				pGems[pGrid[row][column]].type = -1;
			}
			// std::cout << "After: column = " << column << ", pGems[pGrid["<<row<<"]["<<column<<"]].type = " << pGems[pGrid[row][column]].type << "\n";
		}
		//pGems[pGrid[pThisGem->row][column]].type = -1;
		foundMatch = true;
		addScore(numHorizontalMatches[0] + numHorizontalMatches[1] + 1, pScore);
	}
	// std::cout << "checkMatchAfterSwap: numHorizontalMatches[0] = " << numHorizontalMatches[0] << ", numHorizontalMatches[1] = " << numHorizontalMatches[1] << "\n";

	// Now check vertical matches
	// std::cout << "Check vertical matches\n";
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
				// std::cout << "checkMatchAfterSwap 2: compareColumn = " << compareColumn << "\n";
				// std::cout << "checkMatchAfterSwap: pGems[pGrid["<<compareRow<<"]["<<column<<"]].type = " << pGems[pGrid[compareRow][column]].type << "\n";
				if (pGems[pGrid[compareRow][column]].type == pThisGem->type)
				{
					numVerticalMatches[numMatchesIndex]++;
					compareRow += direction;
					// std::cout << "match. direction = " << direction << ", numVerticalMatches[" << numMatchesIndex << "] = " << numVerticalMatches[numMatchesIndex] << "\n";
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

		// // Delete matching gems
		// if (numVerticalMatches[numMatchesIndex] >= 2)
		// {
		// 	for (int i = 0; i < numVerticalMatches[numMatchesIndex]; i++)
		// 	{
		// 		// Mark them for deletion
		// 		pGems[pGrid[pThisGem->row + direction * (i+1)][column]].type = -1;
		// 	}
		// 	pGems[pGrid[pThisGem->row][column]].type = -1;
		// 	foundMatch = true;
		// }

		// Set to compare gems below for second iteration of loop
		direction = 1;
	}

	// Delete matching gems
	if (numVerticalMatches[0] + numVerticalMatches[1]  >= 2)
	{
		// std::cout << "Found vertical match\n";
		for (int row = pThisGem->row - numVerticalMatches[0]; row <= pThisGem->row + numVerticalMatches[1]; row++)
		{
				// std::cout << "Delete gem at row " << row << ", column " << column << "\n";
			// Mark them for deletion
			pGems[pGrid[row][column]].type = -1;
		}
		//pGems[pGrid[pThisGem->row][column]].type = -1;
		foundMatch = true;
		addScore(numVerticalMatches[0] + numVerticalMatches[1] + 1, pScore);
	}
	// std::cout << "checkMatchAfterSwap: numVerticalMatches[0] = " << numVerticalMatches[0] << ", numVerticalMatches[1] = " << numVerticalMatches[1] << "\n";

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

	// if (TTF_Init() != 0){
	// logSDLError(std::cout, "TTF_Init");
	// return 1;
	// }

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
	SDL_Texture *cursorYellowImg = loadTexture("res/cursor_yellow.png", renderer);
	SDL_Texture *cursorGreenImg = loadTexture("res/cursor_green.png", renderer);
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

	int score;

	int mouseX = 0;
	int mouseY = 0;
	int numGemsSelected = 0;
	int selectedGems[2][2] = {{-1, -1}, {-1, -1}};
	bool gemSelected = false;
	int cursorRow = 0;
	int cursorColumn = 0;
	bool foundMatch = false;
	// gem* thisGem;
	// gem* rightGem;
	int thisRow, thisColumn;
	bool gemsDropping;
	bool paused = false;
	bool cancelSwap = false;
	bool mouseDown = false;
	int dragStartX;
	int dragStartY;
	bool dragStarted = false;
	time_t timer;
	time_t startTime;
	// time_t pauseStartTime;

	while (!quit)
	{
		if (!paused)
		{

			timer = time(0);
			// std::cout << "timer = " << timer << ", timer - startTime = " << (timer - startTime) << "\n";
			if (timer - startTime >= 60)
			{
				std::cout << "Init\n";
				gameState = GAMESTATE_INIT;
			}
			switch (gameState)
			{
				case GAMESTATE_INIT:
					for (int i = 0; i < NUM_ROWS*NUM_COLUMNS; i++)
					{
						// pGemStateArray[i] = GEMSTATE_IDLE;
						pGemsArray[i].state = GEMSTATE_IDLE;
					}
					pGridArray = initGrid(pGemsArray);
					score = 0;
					startTime = time(0);
					// std::cout << "startTime = " << startTime << "\n";
					gameState = GAMESTATE_INITIAL_CHECK_MATCH;
				break;

				case GAMESTATE_INITIAL_CHECK_MATCH:
					SDL_Delay(1000);
					checkMatchAllRows(pGridArray, pGemsArray, &score);
					// gameState = GAMESTATE_AWAIT_INPUT;
					// checkDrop(pGridArray, pGemsArray);
					gameState = GAMESTATE_CHECKDROP;
				break;

				case GAMESTATE_CHECK_MATCH:

					std::cout << "GAMESTATE_CHECK_MATCH\n";

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
						cursorRow = mouseY / ROW_HEIGHT;
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
								cursorRow = mouseY / ROW_HEIGHT;
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
					
					// gameState = GAMESTATE_CHECKDROP;
				break;


				case GAMESTATE_SWAP_LEFT:
				case GAMESTATE_SWAP_RIGHT:

					thisRow = selectedGems[0][ROW];
					if (gameState == GAMESTATE_SWAP_LEFT)
						thisColumn = selectedGems[0][COLUMN];
					else
						thisColumn = selectedGems[1][COLUMN];
					// thisGem = &pGemsArray[pGridArray[thisRow][thisColumn]];
					// rightGem = &pGemsArray[pGridArray[thisRow][selectedGems[1][COLUMN]]];
					
					// std::cout << "velocity = " << (thisGem->velocity) << ", x = " << (thisGem->x)<< "\n";
					// thisGem->x += thisGem->velocity;
					// rightGem->x--;// += rightGem->velocity;
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
						

						//thisGem = &pGemsArray[pGridArray[row][column]];
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

					// std::cout << "GAMESTATE_SWAP_HORIZONTAL\n";
					thisColumn = selectedGems[0][COLUMN];
					if (gameState == GAMESTATE_SWAP_UP)
						thisRow = selectedGems[0][ROW];
					else
						thisRow = selectedGems[1][ROW];
					// thisGem = &pGemsArray[pGridArray[thisRow][thisColumn]];
					// rightGem = &pGemsArray[pGridArray[thisRow][selectedGems[1][COLUMN]]];
					
					// std::cout << "velocity = " << (thisGem->velocity) << ", x = " << (thisGem->x)<< "\n";
					// thisGem->x += thisGem->velocity;
					// rightGem->x--;// += rightGem->velocity;
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
						

						//thisGem = &pGemsArray[pGridArray[row][column]];
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

					std::cout << "GAMESTATE_CHECKDROP\n";
					if (checkDrop(pGridArray, pGemsArray))
					{
						gameState = GAMESTATE_DROP;
					}
					else
					{
						// gameState = GAMESTATE_AWAIT_INPUT;
						gameState = GAMESTATE_ADD_GEMS;
					}

				break;

				case GAMESTATE_DROP:
					// std::cout << "GAMESTATE_DROP\n";
					// dropGems(pGridArray, pGemsArray);

					// Check if any more gems are dropping
					gemsDropping = false;
					for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
					{
						// std::cout << "pGemsArray["<<i<<"].state = " << pGemsArray[i].state << "\n";
						if (pGemsArray[i].state == GEMSTATE_FALL || pGemsArray[i].state == GEMSTATE_ENTER || pGemsArray[i].state == GEMSTATE_BOUNCE)
						{
							gemsDropping = true;
							break;
						}
					}

					if (!gemsDropping)
					{
						std::cout << "GAMESTATE_DROP: !gemsDropping\n";
						gameState = GAMESTATE_CHECK_MATCH;
					}
				break;

				case GAMESTATE_ADD_GEMS:

					std::cout << "GAMESTATE_ADD_GEMS\n";

					// for (int row = 0; row < NUM_ROWS; row++)
					// {
					// 	std::cout << "[";
					// 	for (int column = 0; column < NUM_COLUMNS; column++)
					// 	{
					// 		std::cout << pGemsArray[pGridArray[row][column]].type << ",";
					// 	}
					// 	std::cout << "]\n";
					// }

					int i = 0;
					bool gemsAdded = false;
					// Find empty columns
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						// Find empty spaces in this column starting from top
						int row = 0;
						// std::cout << "row = " << row << ", column = " << column << ", type = " << pGemsArray[pGridArray[row][column]].type << "\n";
						while (pGemsArray[pGridArray[row][column]].type == -1)
						{
							// std::cout << "Found empty at ["<<row<<"]["<<column<<"], id is " << pGridArray[row][column] << "\n";
							// Found empty space. Now make this gem come alive
							// pGemsArray[pGridArray[row][column]].type = 0;
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

					// std::cout << "Drop to\n";
					// for (int row = 0; row < NUM_ROWS; row++)
					// {
					// 	std::cout << "[";
					// 	for (int column = 0; column < NUM_COLUMNS; column++)
					// 	{
					// 		std::cout << pGemsArray[pGridArray[row][column]].dropToRow << ",";
					// 	}
					// 	std::cout << "]\n";
					// }

					// std::cout << "Gems added = " << i << "\n";
					// gameState = GAMESTATE_CHECKDROP;
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

				// case GAMESTATE_DROP_NEW_GEMS:


				// break;
			}

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
							// std::cout << "GEMSTATE_FALL: row "<<row<< ", column "<<column << "\n";
							if ((pGemsArray[pGridArray[row][column]].type != -1 && pGemsArray[pGridArray[row+1][column]].type == -1) ||
								pGemsArray[pGridArray[row][column]].state == GEMSTATE_ENTER)
							{
								// std::cout << "GEMSTATE_FALL: dropping gem at row = " << row << ", column " << column << 
								// ", type " << pGemsArray[pGridArray[row][column]].type << "\n";
								dropGem(pGridArray, &pGemsArray[pGridArray[row][column]], pGemsArray);
							}
						break;

						case GEMSTATE_BOUNCE:

							thisGem->y += thisGem->velocity;
							thisGem->velocity--;		

							// if (thisGem->row == 6 && thisGem->column == 2)
							// {
							// 	std::cout << "GEMSTATE_BOUNCE: y = " << thisGem->y << "\n";
							// }
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

		//Read user input & handle it
		//Read any events that occured, for now we'll just quit if any event occurs
		while (SDL_PollEvent(&e)){
			//If user closes the window
			if (e.type == SDL_QUIT){
				quit = true;
			}
			//If user presses any key
			if (e.type == SDL_KEYDOWN){
				// quit = true;
				paused = !paused;

				// if (paused)
				// {
				// 	pauseStartTime = time(0);
				// }
			}
			//If user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				// std::cout << "x = " << (e.button.x) << "\n";
				mouseX = e.button.x;
				mouseY = e.button.y;
				// gemSelected = true;
				mouseDown = true;
				dragStartX = mouseX;
				dragStartY = mouseY;
				dragStarted = false;
			}
			if (e.type == SDL_MOUSEBUTTONUP){
				// if (!(e.button.x == mouseX && e.button.y == mouseY))
				{
					mouseDown = false;
					// mouseX = e.button.x;
					// mouseY = e.button.y;
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
		// int rowHeight = SCREEN_HEIGHT / NUM_ROWS;
		int gemWidth, gemHeight;
		int x, y;
		SDL_QueryTexture(gems[0], NULL, NULL, &gemWidth, &gemHeight);

		// Draw cursors
		if (numGemsSelected > 0)
		{
			int cursorWidth, cursorHeight;
			SDL_QueryTexture(cursorYellowImg, NULL, NULL, &cursorWidth, &cursorHeight);
			renderTexture(cursorYellowImg, renderer, 
				selectedGems[0][COLUMN] * COLUMN_WIDTH + COLUMN_WIDTH/2 - cursorWidth/2, 
				selectedGems[0][ROW] * ROW_HEIGHT + ROW_HEIGHT/2 - cursorHeight/2);
		
			if (numGemsSelected == 2)
			{
				renderTexture(cursorGreenImg, renderer, 
					selectedGems[1][COLUMN] * COLUMN_WIDTH + COLUMN_WIDTH/2 - cursorWidth/2, 
					selectedGems[1][ROW] * ROW_HEIGHT + ROW_HEIGHT/2 - cursorHeight/2);
			}
		}

		for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
		{
			if (pGemsArray[i].type != -1)
			{
				x = COLUMN_WIDTH * pGemsArray[i].column + COLUMN_WIDTH/2 - gemWidth/2;
				y = ROW_HEIGHT * pGemsArray[i].row + ROW_HEIGHT/2 - gemHeight/2;
				renderTexture(gems[pGemsArray[i].type], renderer, x + pGemsArray[i].x, y + pGemsArray[i].y);
			}
		}

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