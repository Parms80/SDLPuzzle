#include "util.h"
#include <iostream>
#include <SDL.h>

void logSDLError(std::ostream &os, const std::string &msg){
	os << msg << " error: " << SDL_GetError() << std::endl;
}
