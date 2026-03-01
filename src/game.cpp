#include "prelude.hpp"
#include "cfg.hpp"
#include <raylib.h>
#include <dbg.h>
#include <unordered_map>
#include <utility>

#define nl '\n'
#define REF_MAP_HEIGHT 51 // 0->20 nothing, 70->90 the core
#define REF_MAP_WIDTH 50

// Game file assets begin here
#define RESOURCES_FILE_PATH "res/res.png"
#define BGMUSIC_FILE_PATH "res/music.wav"
//name, coord y-x, width height
std::unordered_map<std::string, std::pair<std::pair<int, int>, std::pair<int, int>>> resources = {
	{"player_walk_1", {{192, 210}, {16, 16}}},
	{"player_walk_2", {{211, 210}, {16, 16}}},
	{"player_walk_3", {{230, 210}, {16, 16}}},
	{"player_walk_4", {{249, 210}, {16, 16}}},
	{"player_walk_5", {{268, 210}, {16, 16}}},
	{"player_walk_6", {{287, 210}, {16, 16}}},
	{"player_walk_7", {{306, 210}, {16, 16}}},
	{"player_walk_7", {{306, 210}, {16, 16}}},
	{"player_idle1", {{125,195}, {16,16}}},
	{"player_idle2", {{441,221}, {16,16}}},
	{"hook", {{239, 118}, {8, 7}}},
	{"wall1", {{113, 362}, {16, 16}}},
	{"wall2", {{134, 349}, {16, 16}}},
	{"wall3", {{134, 371}, {16, 16}}},
	{"surface", {{70, 453}, {16, 16}}},
	{"core_earth", {{80,343}, {16,16}}},
	{"lava", {{81,369}, {16,16}}},
	{"spike", {66,378}, {9,7}}},
	{"hook", {{237,117}, {8,7}}},
	{"gem_on_profile", {246,160}, {14,9}}},
	{"playerhealth_bar_1full",{202,161}, {36,10}}},
	{"playerhealth_bar_2", {202,149},{36,10}}},
	{"playerhealth_bar_3",{202,138},{36,10}}},
	{"playerhealth_bar_4",{203,173},{36,10}}},
	{"playerhealth_bar_5",{203,185},{36,10}}},
	{"playerhealth_bar_6empty",{203,195},{36,10}}},
	{"timer_1full", {15,306},{45,17}}},
	{"timer_2",{15,325},{45,17}}},
	{"timer_3",{15,346},{45,15}}},
	{"timer_4",{15,365},{45,15}}},
	{"timer_5",{14,386},{45,14}}},
	{"timer_6empty", {14,410}, {45,9}}},
	{"spider_enemy_idle1", {251,359},{16,16}}},
	{"spider_enemy_idle2",{270,359},{16,16}}},
	{"mouse_enemy_walk1",{474,277},{17,12}}},
	{"mouse_enemy_walk2",{475,322},{18,11}}},
	{"mouse_enemy_roll",{439,358},{15,15}}},
	{"play_button_aka_dig", {31,23},{74,25}}},
	{"leaderboard_button",{31,52}, {74,25}}},
	

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
};

class RefMap {
public:
	RefMapAttr data[REF_MAP_HEIGHT][REF_MAP_WIDTH];
};

class Player {
private:
public:
	Vector2 position;
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
	map_divide_recurse(rows_marked, 2, REF_MAP_HEIGHT - 3);

	// add mandatory ones
	for (int a=0; a<REF_MAP_WIDTH; a++) {
		refmap.data[0][a] = SURFACE;
	}
	rows_marked.push_back(0);

	for (int a=0; a<REF_MAP_WIDTH; a++) {
		refmap.data[REF_MAP_HEIGHT - 1][a] = CORE;
	}
	rows_marked.push_back(REF_MAP_HEIGHT - 1);

	// sort everything
	std::sort(rows_marked.begin(), rows_marked.end());

	// draw the connectors
	for (int i = 0; i < int(rows_marked.size()); i++) {
		if (refmap.data[rows_marked[i]][0] != CORE && refmap.data[i][0] != SURFACE) {
			for (int a=0; a<REF_MAP_WIDTH; a++) {
				refmap.data[rows_marked[i]][a] = PLATFORM;
			}
		}

		// kinda sloppy imagine being so unlucky
		// but probability of getting the same spot just 10 times in a row is basically 3/50^10 so like- idk how unlucky you have to be to get that
		// sorry why am i so paranoid ..
		if (i==rows_marked.size()-1) continue;
		for (int c=0; c<GetRandomValue(1, 5); c++) {
			int random_hit = GetRandomValue(0, REF_MAP_WIDTH-1);
			// tho if the REF_MAP_RADIUS is super small u might loop a while here
			while (refmap.data[rows_marked[i]+1][random_hit] == TUNNEL || refmap.data[rows_marked[i]+1][random_hit-1] == TUNNEL || refmap.data[rows_marked[i]+1][random_hit+1] == TUNNEL) {
				random_hit = GetRandomValue(0, REF_MAP_WIDTH-1);
			}
			for (int j=rows_marked[i]+1; j < rows_marked[i+1]; j++) {
				dbg(j, random_hit, rows_marked);
				refmap.data[j][random_hit] = TUNNEL;
			}
		}
	}
	
	return refmap;
}

// Sprite or other globals begin here
Player player(raylib::Vector2(0.0, 0.0));
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
				case TUNNEL:      c = '|'; break;
				case SURFACE:     c = '"'; break;
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

	EndMode2D();

	EndDrawing();
}

void game_update() {
	camera.target = player.position;

}
