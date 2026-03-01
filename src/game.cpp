#include "prelude.hpp"
#include "cfg.hpp"
#include <raylib.h>
#include <dbg.h>
#include <unordered_map>
#include <utility>

#define nl '\n'
#define REF_MAP_HEIGHT 51
#define REF_MAP_WIDTH 50
#define ACTUAL_TILE_PIXELS 128

// Game file assets begin here
#define RESOURCES_FILE_PATH "res/res.png"
#define BGMUSIC_FILE_PATH "res/music.wav"
std::unordered_map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> resources = {
	{"player_walk_1", {{192, 210}, {16, 16}}},
	{"player_walk_2", {{211, 210}, {16, 16}}},
	{"player_walk_3", {{230, 210}, {16, 16}}},
	{"player_walk_4", {{249, 210}, {16, 16}}},
	{"player_walk_5", {{268, 210}, {16, 16}}},
	{"player_walk_6", {{287, 210}, {16, 16}}},
	{"player_walk_7", {{306, 210}, {16, 16}}},
	{"hook", {{239, 118}, {8, 7}}},
	{"wall1", {{113, 362}, {16, 16}}},
	{"wall2", {{134, 349}, {16, 16}}},
	{"wall3", {{134, 371}, {16, 16}}},
	{"surface", {{70, 453}, {16, 16}}},
};

Texture2D assets;

// Classes etc begin here
enum RefMapAttr {
	BLANK_SPACE = 0,
	WALL = 1,
	PLATFORM = 2,
	TUNNEL = 3,
	SURFACE = 4,
	CORE = 5,
	BUCKET = 6,
	PLATFORM_SPIKE = 7
};

class RefMap {
public:
	RefMapAttr data[REF_MAP_HEIGHT][REF_MAP_WIDTH];
};

class Player {
private:
public:
	Vector2 position;
	Vector2 position_refmap;
	Player(Vector2 position) {
		this->position = position;
	};

	void draw() {


	}
};

// Game functions begin here
void map_divide_recurse(std::vector<int>& rows, int start, int end) {
	if (start >= end) {
		return;
	}
	// we can pick the start and end too.
	int picked = GetRandomValue(start, end);
	rows.push_back(picked);
	map_divide_recurse(rows, start+2, picked-2);
	map_divide_recurse(rows, picked+2, end-2);
	
}

RefMap generate_map() {
	RefMap refmap;

	for (int i=0; i<REF_MAP_HEIGHT; i++) {
		for (int j=0; j<REF_MAP_WIDTH; j++) {
			refmap.data[i][j] = BLANK_SPACE;
		}
	}


	// draw the platforms
	std::vector<int> rows_marked;
	map_divide_recurse(rows_marked, 3, REF_MAP_HEIGHT - 3);

	// probability (percent) that a given platform tile becomes a spike
	const int SPIKE_CHANCE = 10; // 10% chance per tile

	// add mandatory ones
	for (int a=0; a<REF_MAP_WIDTH; a++) {
		refmap.data[1][a] = SURFACE;
	}
	rows_marked.push_back(1);

	for (int a=0; a<REF_MAP_WIDTH; a++) {
		refmap.data[REF_MAP_HEIGHT - 1][a] = CORE;
	}
	rows_marked.push_back(REF_MAP_HEIGHT - 1);

	refmap.data[0][0] = BUCKET;

	// sort everything
	std::sort(rows_marked.begin(), rows_marked.end());

	// draw the connectors
	for (int i = 0; i < int(rows_marked.size()); i++) {
		if (refmap.data[rows_marked[i]][0] != CORE && refmap.data[rows_marked[i]][0] != SURFACE) {
			for (int a=0; a<REF_MAP_WIDTH; a++) {
				// decide whether this tile becomes a spike
				if (GetRandomValue(0, 99) < SPIKE_CHANCE) {
					refmap.data[rows_marked[i]][a] = PLATFORM_SPIKE;
				} else {
					refmap.data[rows_marked[i]][a] = PLATFORM;
				}
			}
		}

		// kinda sloppy imagine being so unlucky
		// but probability of getting the same spot just 10 times in a row is basically 3/50^10 so like- idk how unlucky you have to be to get that
		// sorry why am i so paranoid ..
		if (i==rows_marked.size()-1) continue;
		for (int c=0; c<GetRandomValue(2, 9); c++) {
			int random_hit = GetRandomValue(0, REF_MAP_WIDTH-1);
			// tho if the REF_MAP_RADIUS is super small u might loop a while here
			while (refmap.data[rows_marked[i]+1][random_hit] == TUNNEL || refmap.data[rows_marked[i]+1][random_hit-1] == TUNNEL || refmap.data[rows_marked[i]+1][random_hit+1] == TUNNEL) {
				random_hit = GetRandomValue(0, REF_MAP_WIDTH-1);
			}
			for (int j=rows_marked[i]+1; j < rows_marked[i+1]; j++) {
				refmap.data[j][random_hit] = TUNNEL;
			}
		}
	}
	
	return refmap;
}

// Sprite or other globals begin here
Player player(raylib::Vector2(ACTUAL_TILE_PIXELS, SCREEN_HEIGHT/2.0f));
raylib::Camera2D camera(raylib::Vector2(SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f), player.position);

// Game functions begin here
void game_init() {

	std::cout << "(DBG) GENERATED MAP: " << nl;

	auto map = generate_map();
	for (int y = 0; y < REF_MAP_HEIGHT; y++) {
		for (int x = 0; x < REF_MAP_WIDTH; x++) {
			char c;

			switch (map.data[y][x]) {
				case BLANK_SPACE: c = '.'; break;
				case WALL:        c = '#'; break;
				case PLATFORM:    c = '-'; break;
			case PLATFORM_SPIKE: c = '^'; break;
				case TUNNEL:      c = '|'; break;
				case SURFACE:     c = '"'; break;
				case BUCKET:      c = 'v'; break;
				case CORE:        c = '*'; break;
				default:          c = '?'; break;
			}

			std::cout << c;
		}
		std::cout << '\n';
	}
	int spacing = 0;


	// load assets
	// SetTextureFilter(, TEXTURE_FILTER_POINT).

}

void game_draw() {
	BeginDrawing();

	ClearBackground(RAYWHITE);

	BeginMode2D(camera);

	DrawRectangle(0, 0, 100, 100, BLUE);


	EndMode2D();

	EndDrawing();
}

void game_update() {
	camera.target = raylib::Vector2(std::clamp(player.position.x + 8, float(SCREEN_WIDTH/2.0), float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH - SCREEN_WIDTH/2.0)), std::clamp(player.position.y + 8, float(SCREEN_WIDTH/2.0), float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH - SCREEN_HEIGHT/2.0)));

	if (IsKeyDown(KEY_RIGHT)) {
		if (player.position.x <= ACTUAL_TILE_PIXELS * REF_MAP_WIDTH) player.position.x += 10.0;
	}
	if (IsKeyDown(KEY_LEFT)) {
		 player.position.x -= 10.0;
	}

	player.position = raylib::Vector2(std::clamp(player.position.x, float(0.0), float(ACTUAL_TILE_PIXELS*REF_MAP_WIDTH)), std::clamp(player.position.y, float(0.0), float(ACTUAL_TILE_PIXELS*REF_MAP_WIDTH)));


}
