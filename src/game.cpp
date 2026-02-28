#include "prelude.hpp"
#include "cfg.hpp"
#include <bitset>

#define REF_MAP_RADIUS 50

class ReferenceMap {
	bool data[50][50];
};

class Player {
private:
public:
	Vector2 position;
	Player(Vector2 position) {
		this->position = position;
	};
};

void map_divide_recurse(bool* arr, size_t start, size_t end) {
	if (start >= end) {
		return;
	} else if (GetRandomValue(1, 100) <= 20) {
		return;
	}
	// we can pick the start and end too.
	size_t picked = GetRandomValue(start, end);
	arr[picked] = true;
	map_divide_recurse(arr, start, picked-1);
	map_divide_recurse(arr, picked+1, end);
	
}

ReferenceMap generate_map() {
	bool data[50][50];
	for (size_t i=0; i<REF_MAP_RADIUS; i++) {
		for (size_t j=0; i<REF_MAP_RADIUS; j++) {
			data[i][j] = false;
		}
	}
	bool marked_rows[50];
	map_divide_recurse(marked_rows, 0, REF_MAP_RADIUS);
}

Player player(raylib::Vector2(0.0, 0.0));
raylib::Camera2D camera(raylib::Vector2(SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f), player.position);

void game_init() {

	int spacing = 0;

}

void game_draw() {
	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginMode2D(camera);

	EndMode2D();

	EndDrawing();
}

void game_update() {
	camera.target = player.position;

}
