#ifndef RENDER_H
#define RENDER_H

#include <SDL.h>

// class Render
// {
// public:
// 	Render();
// 	~Render();

	void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h);
	void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y);
// };

#endif