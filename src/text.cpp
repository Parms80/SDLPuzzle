#include "text.h"
#include <SDL.h>
#include "util.h"
#include "Render.h"
#include <sstream>
/**
* Render the message we want to display to a texture for drawing
* @param message The message we want to display
* @param fontFile The font we want to use to render the text
* @param color The color we want the text to be
* @param fontSize The size we want the font to be
* @param renderer The renderer to load the texture in
* @return An SDL_Texture containing the rendered message, or nullptr if something went wrong
*/

SDL_Texture* renderText(const std::string &message, TTF_Font *font,
	SDL_Color color, SDL_Renderer *renderer)
{
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
	return texture;
}

int drawText(char* msg, int num, SDL_Renderer *renderer, SDL_Texture *textImage, TTF_Font *font, SDL_Color color, int x, int y)
{
	std::ostringstream os;

	os.str("");
	os.clear();
	os << msg;
	if (num != NULL)
	{
		os << num;
	}
	textImage = renderText(os.str(), font, color, renderer);
	if (textImage == nullptr){
		return 1;
	}
	x = 10;
	y = 10;
	renderTexture(textImage, renderer, x, y);

	return 0;
}

TTF_Font* loadFont(const std::string &fontFile, int fontSize)
{
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr){
		logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}

	return font;
}

void closeFont(TTF_Font *font)
{
	TTF_CloseFont(font);
}