#ifndef TEXT_H
#define TEXT_H

#include <SDL_ttf.h>
#include <iostream>

SDL_Texture* renderText(const std::string &message, TTF_Font *font,
SDL_Color color, SDL_Renderer *renderer);

int drawText(char* msg, int num, SDL_Renderer *renderer, SDL_Texture *textImage, TTF_Font *font, SDL_Color color, int x, int y);

TTF_Font* loadFont(const std::string &fontFile, int fontSize);
void closeFont(TTF_Font *font);


#endif