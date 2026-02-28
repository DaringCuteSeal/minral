#include "prelude.hpp"
#include "cfg.hpp"
#include "game.hpp"

int main() {
	raylib::InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
	InitAudioDevice();
	SetMasterVolume(1.0);
	SetTargetFPS(FPS);
	SetRandomSeed(time(NULL));

	game_init();

	while (!raylib::Window::ShouldClose()) {
		game_update();
		game_draw();
	}

	return 0;
}
