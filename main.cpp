#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
// #include "Render.h"

const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 480;
const int NUM_ROWS = 8;
const int NUM_COLUMNS = 8;
const int ROW_HEIGHT = SCREEN_HEIGHT / NUM_ROWS;
const int COLUMN_WIDTH = SCREEN_WIDTH / NUM_COLUMNS;
const int MAX_DROP_SPEED = 10;
const int SWAP_SPEED = 10;

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

const int GEMSTATE_IDLE = 0;
const int GEMSTATE_FALL = 1;
const int GEMSTATE_DEAD = 2;
const int GEMSTATE_BOUNCE = 3;
const int GEMSTATE_MOVE_LEFT = 4;
const int GEMSTATE_MOVE_RIGHT = 5;
const int GEMSTATE_MOVE_UP = 6;
const int GEMSTATE_MOVE_DOWN = 7;

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
			pGems[pGrid[row][column]].type = rand() % 5;//testGridArray[row][column];
			pGems[pGrid[row][column]].y = 0;//row * rowHeight;
			pGems[pGrid[row][column]].x = 0;
			pGems[pGrid[row][column]].row = row;
			pGems[pGrid[row][column]].column = column;
			pGems[pGrid[row][column]].velocity = 1;
			pGems[pGrid[row][column]].prevState = -1;

			gridCount++;
		}
	}

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

	return pGrid;
}


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

bool checkMatchAllColumns(int** pGrid, gem* pGems)
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
				}
			}
		}
	}

	return foundMatch;
}

bool checkMatchAllRows(int** pGrid, gem* pGems)
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
			std::cout << pGems[tempGridRow[row][column]].type << ",";
		}
		std::cout << "]\n";
	}

	bool foundMatch = false;
	for (int row = 0; row < NUM_ROWS; row++)
	{
		for (int column = 0; column < NUM_COLUMNS-2; column++)
		{
			int gemToCompare = pGems[tempGridRow[row][column]].type;

			std::cout << "checkMatchAllRows: gemToCompare = " << gemToCompare << ", row = " << row << ", column = " << column << "\n";

			if (gemToCompare != -1)
			{
				// Now check for at least 2 adjacent matching gems to the right
				if (pGems[tempGridRow[row][column+1]].type == gemToCompare && 
					pGems[tempGridRow[row][column+2]].type == gemToCompare)
				{
					std::cout << "checkMatchAllRows: found matching row. row = " << row << ", column = " << column << "\n";
					// Continue marking matching gems until reached a different gem
					bool matchRow = true;
					int j = 1;
					while (matchRow && column+j < NUM_COLUMNS)
					{
						std::cout << "checkMatchAllRows: j = " << j << "\n";

						if (pGems[tempGridRow[row][column+j]].type == gemToCompare)
						{
							std::cout << "checkMatchAllRows: mark for deletion\n";
							tempGridRow[row][column+j] = -1;
							j++;
						}
						else
						{
							std::cout << "checkMatchAllRows: matchRow = false\n";
							matchRow = false;
						}
					}
					tempGridRow[row][column] = -1;	// Get rid of origin gem
					foundMatch = true;
				}
			}
		}
	}
	std::cout << "Before checkMatchAllColumns\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << tempGridRow[row][column] << ",";
		}
		std::cout << "]\n";
	}

	bool foundMatchColumns = checkMatchAllColumns(pGrid, pGems);

	std::cout << "After checkMatchAllColumns\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[pGrid[row][column]].type << ",";
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
				pGems[pGrid[row][column]].type = -1;
			}
		}
	}
	std::cout << "After merge\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[pGrid[row][column]].type << ",";
		}
		std::cout << "]\n";
	}

	std::cout << "checkMatchAllRows: foundMatch = " << foundMatch << ", foundMatchColumns = " << foundMatchColumns << "\n";
	if (foundMatch || foundMatchColumns)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*
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

	// for (int row = 0; row < NUM_ROWS; row++)
	// {
	// 	delete [] tempGrid[row];
	// }
	// delete [] tempGrid;

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
*/

bool checkDrop(int** pGrid, gem* pGems)
{
	std::cout << "checkDrop\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[pGrid[row][column]].type << ",";
		}
		std::cout << "]\n";
	}

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
				std::cout << "checkDrop: found empty at [" << row << "][" << column << "]\n";
				foundEmpty = true;
				// Found a blank space so mark all gems above to drop
				for (int i = row; i >= 0; i--)
				{
					if (pGems[pGrid[i][column]].type != -1)
					{
						shouldDrop = true;
						std::cout << "checkDrop: mark [" << i << "][" << column << "] to fall\n";
						pGems[pGrid[i][column]].state = GEMSTATE_FALL;
						pGems[pGrid[i][column]].prevState = GEMSTATE_FALL;
					}
				}
			}
			row--;
		}
	}

	std::cout << "checkDrop: shouldDrop = " << shouldDrop << "\n";
	return shouldDrop;
}


void dropGem(int** pGrid, gem* pThisGem, gem* pGems)
{
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
		int temp = pGrid[pThisGem->row+1][pThisGem->column];
		pGrid[pThisGem->row+1][pThisGem->column] = pGrid[pThisGem->row][pThisGem->column];
		// pGrid[row][column] = -1;
		pGrid[pThisGem->row][pThisGem->column] = temp;

		// pGems[pGrid[pThisGem->row+1][pThisGem->column]].row = pThisGem->row;
		pThisGem->row++;

		// Make gem stop falling if there is a gem below or it has reached bottom of grid
		if (pThisGem->row == NUM_ROWS-1 || 
			(pGems[pGrid[ pThisGem->row+1][pThisGem->column]].type != -1 && pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state != GEMSTATE_FALL))
			 // (pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state == GEMSTATE_IDLE || pGems[pGrid[ pThisGem->row+1][pThisGem->column]].state == GEMSTATE_BOUNCE)))
		{
			
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
	
	std::cout << "checkSwapGems: horizontalDist = " << horizontalDist << " \n";
	std::cout << "checkSwapGems: verticalDist = " << verticalDist << " \n";

	if (horizontalDist == 1 && verticalDist == 0)
	{
		std::cout << "checkSwapGems: adjacent. Swap right \n";
		return GAMESTATE_SWAP_RIGHT;
	}
	else if	(horizontalDist == -1 && verticalDist == 0)
	{
		std::cout << "checkSwapGems: adjacent. Swap left \n";
		return GAMESTATE_SWAP_LEFT;
	}
	else if (horizontalDist == 0 && verticalDist == 1)
	{
		std::cout << "checkSwapGems: adjacent. Swap down \n";
		return GAMESTATE_SWAP_DOWN;
	}
	else if (horizontalDist == 0 && verticalDist == -1)
	{
		std::cout << "checkSwapGems: adjacent. Swap up \n";
		return GAMESTATE_SWAP_UP;
	}
	else
	{
		return -1;
	}

}

bool checkMatchAfterSwap(int** pGrid, gem* pGems, gem* pThisGem)
{
	std::cout << "checkMatchAfterSwap\n";


	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGems[pGrid[row][column]].type << ",";
		}
		std::cout << "]\n";
	}

	std::cout << "pThisGem: row = " << pThisGem->row << ", column = " << pThisGem->column << ", type = " << pThisGem->type << "\n";

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
			if (compareColumn > 0 && compareColumn < NUM_COLUMNS)
			{
				// std::cout << "checkMatchAfterSwap 2: compareColumn = " << compareColumn << "\n";
				std::cout << "checkMatchAfterSwap: pGems[pGrid["<<row<<"]["<<compareColumn<<"]].type = " << pGems[pGrid[row][compareColumn]].type << "\n";
				if (pGems[pGrid[row][compareColumn]].type == pThisGem->type)
				{
					// pGems[pGrid[row][compareColumn]].type = -1;
					numHorizontalMatches[numMatchesIndex]++;
					compareColumn += direction;
					std::cout << "match. direction = " << direction << ", numHorizontalMatches[" << numMatchesIndex << "] = " << numHorizontalMatches[numMatchesIndex] << "\n";
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
		for (int column = pThisGem->column - numHorizontalMatches[0]; column <= pThisGem->column + numHorizontalMatches[1]; column++)
		{
			// Mark them for deletion
			pGems[pGrid[row][column]].type = -1;
		}
		//pGems[pGrid[pThisGem->row][column]].type = -1;
		foundMatch = true;
	}
	// std::cout << "checkMatchAfterSwap: numHorizontalMatches[0] = " << numHorizontalMatches[0] << ", numHorizontalMatches[1] = " << numHorizontalMatches[1] << "\n";

	// Now check vertical matches
	std::cout << "Check vertical matches\n";
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
			if (compareRow > 0 && compareRow < NUM_ROWS)
			{
				// std::cout << "checkMatchAfterSwap 2: compareColumn = " << compareColumn << "\n";
				std::cout << "checkMatchAfterSwap: pGems[pGrid["<<compareRow<<"]["<<column<<"]].type = " << pGems[pGrid[compareRow][column]].type << "\n";
				if (pGems[pGrid[compareRow][column]].type == pThisGem->type)
				{
					numVerticalMatches[numMatchesIndex]++;
					compareRow += direction;
					std::cout << "match. direction = " << direction << ", numVerticalMatches[" << numMatchesIndex << "] = " << numVerticalMatches[numMatchesIndex] << "\n";
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
		for (int row = pThisGem->row - numVerticalMatches[0]; row <= pThisGem->row + numVerticalMatches[1]; row++)
		{
			// Mark them for deletion
			pGems[pGrid[row][column]].type = -1;
		}
		//pGems[pGrid[pThisGem->row][column]].type = -1;
		foundMatch = true;
	}
	// std::cout << "checkMatchAfterSwap: numVerticalMatches[0] = " << numVerticalMatches[0] << ", numVerticalMatches[1] = " << numVerticalMatches[1] << "\n";

	return foundMatch;
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

	while (!quit)
	{
		switch (gameState)
		{
			case GAMESTATE_INIT:
				for (int i = 0; i < NUM_ROWS*NUM_COLUMNS; i++)
				{
					// pGemStateArray[i] = GEMSTATE_IDLE;
					pGemsArray[i].state = GEMSTATE_IDLE;
				}
				pGridArray = initGrid(pGemsArray);
				gameState = GAMESTATE_INITIAL_CHECK_MATCH;
			break;

			case GAMESTATE_INITIAL_CHECK_MATCH:
				SDL_Delay(1000);
				checkMatchAllRows(pGridArray, pGemsArray);
				gameState = GAMESTATE_AWAIT_INPUT;
				checkDrop(pGridArray, pGemsArray);
			break;

			case GAMESTATE_CHECK_MATCH:

				std::cout << "GAMESTATE_CHECK_MATCH 0\n";

				foundMatch = false;
				for (int row = NUM_ROWS-1; row >= 0; row--)
				{
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						if (pGemsArray[pGridArray[row][column]].prevState == GEMSTATE_FALL)
						{
							foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[row][column]]);
						}
					}
				}

				// if (foundMatch)
				// {
				// 	std::cout << "foundMatch. Going to GAMESTATE_DROP\n";
				// 	gameState = GAMESTATE_CHECKDROP;
				// }
				// else
				// {
				// 	std::cout << "Not foundMatch. Going to GAMESTATE_AWAIT_INPUT\n";
				// 	gameState = GAMESTATE_AWAIT_INPUT;
				// }
				gameState = GAMESTATE_CHECKDROP;

				for (int row = NUM_ROWS-1; row >= 0; row--)
				{
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						pGemsArray[pGridArray[row][column]].prevState = -1;
					}
				}
				// if (checkMatchAllRows(pGridArray, pGemsArray))
				// {
				// 	// Found match(es)
				// 	std::cout << "GAMESTATE_CHECK_MATCH 1\n";
					
				// 	gameState = GAMESTATE_CHECKDROP;
				// }
				// else
				// {
				// 	std::cout << "GAMESTATE_CHECK_MATCH 2\n";
				// 	gameState = GAMESTATE_AWAIT_INPUT;
				// }

				std::cout << "main\n";
				for (int row = 0; row < NUM_ROWS; row++)
				{
					std::cout << "[";
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						std::cout << pGemsArray[pGridArray[row][column]].type << ",";
					}
					std::cout << "]\n";
				}

				std::cout << "main\n";
				for (int row = 0; row < NUM_ROWS; row++)
				{
					std::cout << "[";
					for (int column = 0; column < NUM_COLUMNS; column++)
					{
						std::cout << pGemsArray[pGridArray[row][column]].type << ",";
					}
					std::cout << "]\n";
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
						int row;
						int column;

						switch (checkSwapGems((int*)selectedGems))
						{
							case GAMESTATE_SWAP_LEFT:

								// Move right gem left
								row = selectedGems[0][ROW];
								column = selectedGems[0][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = -SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_LEFT;

								// Move left gem right
								row = selectedGems[1][ROW];
								column = selectedGems[1][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_RIGHT;
								// gameState = GAMESTATE_SWAP_HORIZONTAL;
								gameState = GAMESTATE_SWAP_LEFT;
							break;

							case GAMESTATE_SWAP_RIGHT:
								// Move left gem right
								row = selectedGems[0][ROW];
								column = selectedGems[0][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_RIGHT;

								// Move right gem left
								row = selectedGems[1][ROW];
								column = selectedGems[1][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = -SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_LEFT;
								gameState = GAMESTATE_SWAP_RIGHT;

							break;

							case GAMESTATE_SWAP_UP:
								// Move bottom gem up
								row = selectedGems[0][ROW];
								column = selectedGems[0][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = -SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_UP;

								// Move right gem left
								row = selectedGems[1][ROW];
								column = selectedGems[1][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_DOWN;
								gameState = GAMESTATE_SWAP_UP;
							break;

							case GAMESTATE_SWAP_DOWN:
								// Move top gem down
								row = selectedGems[0][ROW];
								column = selectedGems[0][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_DOWN;

								// Move bottom gem up
								row = selectedGems[1][ROW];
								column = selectedGems[1][COLUMN];
								pGemsArray[pGridArray[row][column]].velocity = -SWAP_SPEED;
								pGemsArray[pGridArray[row][column]].state = GEMSTATE_MOVE_UP;
								gameState = GAMESTATE_SWAP_DOWN;
							break;

							default:

								// Remove all cursors
								numGemsSelected = 0;

								// Set new cursor to this position
								cursorRow = mouseY / ROW_HEIGHT;
								cursorColumn = mouseX / COLUMN_WIDTH;

								// Store the location of the selected gem
								selectedGems[numGemsSelected-1][ROW] = cursorRow;
								selectedGems[numGemsSelected-1][COLUMN] = cursorColumn; 
								gemSelected = true;
							break;
						}
					}
				}
				
				// gameState = GAMESTATE_CHECKDROP;
			break;


			case GAMESTATE_SWAP_LEFT:
			case GAMESTATE_SWAP_RIGHT:

				// std::cout << "GAMESTATE_SWAP_HORIZONTAL\n";
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
					std::cout << "GEMSTATE_MOVE_LEFT\n";
					// Swap with other gem
					int temp = pGridArray[thisRow][thisColumn];
					pGridArray[thisRow][thisColumn] = pGridArray[thisRow][thisColumn-1];
					pGridArray[thisRow][thisColumn-1] = temp;

					pGemsArray[pGridArray[thisRow][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].x = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].column++;
					

					//thisGem = &pGemsArray[pGridArray[row][column]];
					pGemsArray[pGridArray[thisRow][thisColumn-1]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn-1]].x = 0;
					pGemsArray[pGridArray[thisRow][thisColumn-1]].column--;

					bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn]]);
					bool foundMatch2 = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn-1]]);

					if (foundMatch || foundMatch2)
					{
						gameState = GAMESTATE_CHECKDROP;
					}
					else
					{
						gameState = GAMESTATE_AWAIT_INPUT;
					}
					// checkMatchAllRows(pGridArray, pGemsArray);
					// gameState = GAMESTATE_AWAIT_INPUT;
					// checkDrop(pGridArray, pGemsArray);


					// for (int row = 0; row < NUM_ROWS; row++)
					// {
					// 	std::cout << "[";
					// 	for (int column = 0; column < NUM_COLUMNS; column++)
					// 	{
					// 		std::cout << pGemsArray[pGridArray[row][column]].type << ",";
					// 	}
					// 	std::cout << "]\n";
					// }
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
					std::cout << "GAMESTATE_MOVE_UP\n";
					// Swap with other gem
					int temp = pGridArray[thisRow][thisColumn];
					pGridArray[thisRow][thisColumn] = pGridArray[thisRow-1][thisColumn];
					pGridArray[thisRow-1][thisColumn] = temp;

					pGemsArray[pGridArray[thisRow][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].y = 0;
					pGemsArray[pGridArray[thisRow][thisColumn]].row++;
					

					//thisGem = &pGemsArray[pGridArray[row][column]];
					pGemsArray[pGridArray[thisRow-1][thisColumn]].velocity = 0;
					pGemsArray[pGridArray[thisRow-1][thisColumn]].y = 0;
					pGemsArray[pGridArray[thisRow-1][thisColumn]].row--;

					bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow][thisColumn]]);
					bool foundMatch2 = checkMatchAfterSwap(pGridArray, pGemsArray, &pGemsArray[pGridArray[thisRow-1][thisColumn]]);

					if (foundMatch || foundMatch2)
					{
						gameState = GAMESTATE_CHECKDROP;
					}
					else
					{
						gameState = GAMESTATE_AWAIT_INPUT;
					}
					// checkMatchAllRows(pGridArray, pGemsArray);
					// gameState = GAMESTATE_AWAIT_INPUT;
					// checkDrop(pGridArray, pGemsArray);


					// for (int row = 0; row < NUM_ROWS; row++)
					// {
					// 	std::cout << "[";
					// 	for (int column = 0; column < NUM_COLUMNS; column++)
					// 	{
					// 		std::cout << pGemsArray[pGridArray[row][column]].type << ",";
					// 	}
					// 	std::cout << "]\n";
					// }
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
					gameState = GAMESTATE_AWAIT_INPUT;
				}
				// gameState = GAMESTATE_AWAIT_INPUT;
			break;

			case GAMESTATE_DROP:
				std::cout << "GAMESTATE_DROP\n";
				// dropGems(pGridArray, pGemsArray);

				// Check if any more gems are dropping
				bool gemsDropping = false;
				for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
				{
					if (pGemsArray[i].state == GEMSTATE_FALL || pGemsArray[i].state == GEMSTATE_BOUNCE)
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
						
						if (pGemsArray[pGridArray[row][column]].type != -1)
						{
							dropGem(pGridArray, &pGemsArray[pGridArray[row][column]], pGemsArray);
						}
					break;

					case GEMSTATE_BOUNCE:

						thisGem->y += thisGem->velocity;
						thisGem->velocity--;		

						if (thisGem->row == 6 && thisGem->column == 2)
						{
							std::cout << "GEMSTATE_BOUNCE: y = " << thisGem->y << "\n";
						}
						if (thisGem->y <= 0)
						{
							thisGem->y = 0;
							thisGem->velocity = 0;
							thisGem->state = GEMSTATE_IDLE;
						}
					break;
/*
					case GEMSTATE_MOVE_LEFT:

						thisGem->x += thisGem->velocity;
						// std::cout << "In GEMSTATE_MOVE_LEFT: x = " << pGemsArray[pGridArray[row][column]].x << ", velocity = " << thisGem->velocity << " \n";
						// swapGem(pGridArray, pGemsArray[pGridArray[row][column]], pGemsArray);
						// std::cout << "GEMSTATE_MOVE_LEFT: -COLUMN_WIDTH = " << (-COLUMN_WIDTH) << "\n";
						if (thisGem->x <= -COLUMN_WIDTH)
						{
							std::cout << "GEMSTATE_MOVE_LEFT\n";
					std::cout << "column before = " << thisGem->column << "\n";
							// Swap with other gem
							int temp = pGridArray[row][column];
							pGridArray[row][column] = pGridArray[row][column-1];
							pGridArray[row][column-1] = temp;

							thisGem->velocity = 0;
							thisGem->x = 0;
							thisGem->column--;
							
					std::cout << "column after = " << thisGem->column << "\n";

							// pGemsArray[pGridArray[row][column+1]].velocity = 0;
							// pGemsArray[pGridArray[row][column+1]].x = 0;
							// pGemsArray[pGridArray[row][column+1]].column++;
							//thisGem = &pGemsArray[pGridArray[row][column]];
							bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, thisGem);
							thisGem->state = GEMSTATE_IDLE;

							if (foundMatch)
							{
								gameState = GAMESTATE_CHECKDROP;
							}
							// checkMatchAllRows(pGridArray, pGemsArray);
							// gameState = GAMESTATE_AWAIT_INPUT;
							// checkDrop(pGridArray, pGemsArray);
						}						
					break;

					case GEMSTATE_MOVE_RIGHT:

						thisGem->x += thisGem->velocity;
						
						if (thisGem->x >= COLUMN_WIDTH)
						{
							std::cout << "GEMSTATE_MOVE_RIGHT\n";
							thisGem->velocity = 0;
							thisGem->x = 0;
							thisGem->column++;
							
							//thisGem = &pGemsArray[pGridArray[row][column]];

							bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, thisGem);							
							thisGem->state = GEMSTATE_IDLE;

							if (foundMatch)
							{
								gameState = GAMESTATE_CHECKDROP;
							}
						}						
					break;

					case GEMSTATE_MOVE_UP:

						thisGem->y += thisGem->velocity;
						
						if (thisGem->y <= -ROW_HEIGHT)
						{
							std::cout << "GEMSTATE_MOVE_UP\n";
							// Swap with other gem
							int temp = pGridArray[row][column];
							pGridArray[row][column] = pGridArray[row-1][column];
							pGridArray[row-1][column] = temp;

							thisGem->velocity = 0;
							thisGem->y = 0;
							thisGem->row--;

							bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, thisGem);	
							thisGem->state = GEMSTATE_IDLE;
							// checkMatchAllRows(pGridArray, pGemsArray);
							// gameState = GAMESTATE_AWAIT_INPUT;
							// checkDrop(pGridArray, pGemsArray);

							if (foundMatch)
							{
								gameState = GAMESTATE_CHECKDROP;
							}

						}						
					break;

					case GEMSTATE_MOVE_DOWN:

						thisGem->y += thisGem->velocity;

						if (thisGem->y >= ROW_HEIGHT)
						{
							std::cout << "GEMSTATE_MOVE_DOWN\n";
							thisGem->velocity = 0;
							thisGem->y = 0;
							thisGem->row++;

							bool foundMatch = checkMatchAfterSwap(pGridArray, pGemsArray, thisGem);	
							thisGem->state = GEMSTATE_IDLE;

							if (foundMatch)
							{
								gameState = GAMESTATE_CHECKDROP;
							}
						}
					break;
					*/
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
				quit = true;
			}
			//If user clicks the mouse
			if (e.type == SDL_MOUSEBUTTONDOWN){
				// std::cout << "x = " << (e.button.x) << "\n";
				mouseX = e.button.x;
				mouseY = e.button.y;
				gemSelected = true;
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
/*
	std::cout << "Before rendering\n";
	for (int row = 0; row < NUM_ROWS; row++)
	{
		std::cout << "[";
		for (int column = 0; column < NUM_COLUMNS; column++)
		{
			std::cout << pGemsArray[pGridArray[row][column]].type << ",";
		}
		std::cout << "]\n";
	}
*/
		// Draw the gems
		// int rowHeight = SCREEN_HEIGHT / NUM_ROWS;
		int gemWidth, gemHeight;
		int x, y;
		SDL_QueryTexture(gems[0], NULL, NULL, &gemWidth, &gemHeight);
		/*
		for (int row = 0; row < NUM_ROWS; row++)
		{
			y = ROW_HEIGHT * row + ROW_HEIGHT/2 - gemHeight/2;

			for (int column = 0; column < NUM_COLUMNS; column++)
			{
				if (pGemsArray[pGridArray[row][column]].type != -1)
				{
					x = columnWidth * column + columnWidth/2 - gemWidth/2;
					// y = pGemsArray[pGridArray[row][column]].y;
					renderTexture(gems[pGemsArray[pGridArray[row][column]].type], renderer, x, y + pGemsArray[pGridArray[row][column]].y);
				}
			}
		}
		for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++)
		{
			if (pGemsArray[i].state != GEMSTATE_DEAD)
			{
				renderTexture(gems[pGemsArray[i].type], renderer, pGemsArray[i].x, pGemsArray[i].y);
			}
		}
		*/

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