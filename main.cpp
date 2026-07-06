#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "Game/game.hpp"

auto main(int argc, char* argv[]) -> int {
	Game game;
	game.Run();

	return 0;
}

