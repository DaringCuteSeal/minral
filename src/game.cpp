#include "prelude.hpp"
#include "cfg.hpp"
#include <raylib.h>
#include <dbg.h>
#include <unordered_map>
#include <utility>

#define nl '\n'
#define REF_MAP_HEIGHT 51 // 0->20 nothing, 70->90 the core
#define REF_MAP_WIDTH 50
#define ACTUAL_TILE_PIXELS 128

// Game file assets begin here
#define RESOURCES_FILE_PATH "res/res.png"
#define BGMUSIC_FILE_PATH "res/music.wav"

// Struct for cleaner asset rectangles
struct AssetRect {
	int x;
	int y;
	int w;
	int h;
};

// Assets map using struct
std::unordered_map<std::string, AssetRect> assets = {
	{"player_walk_1",        {192, 210, 16, 16}},
	{"player_walk_2",        {211, 210, 16, 16}},
	{"player_walk_3",        {230, 210, 16, 16}},
	{"player_walk_4",        {249, 210, 16, 16}},
	{"player_walk_5",        {268, 210, 16, 16}},
	{"player_walk_6",        {287, 210, 16, 16}},
	{"player_walk_7",        {306, 210, 16, 16}},
	{"player_idle1",         {125, 195, 16, 16}},
	{"player_idle2",         {441, 221, 16, 16}},
	{"hook",                 {237, 117, 8, 7}},
	{"wall1",                {113, 362, 16, 16}},
	{"wall2",                {134, 349, 16, 16}},
	{"wall3",                {134, 371, 16, 16}},
	{"surface",              {70, 453, 16, 16}},
	{"core_earth",           {80, 343, 16, 16}},
	{"lava",                 {81, 369, 16, 16}},
	{"spike",                {66, 378, 9, 7}},
	{"gem_on_profile",       {246, 160, 14, 9}},
	{"playerhealth_bar_1full",{202, 161, 36, 10}},
	{"playerhealth_bar_2",   {202, 149, 36, 10}},
	{"playerhealth_bar_3",   {202, 138, 36, 10}},
	{"playerhealth_bar_4",   {203, 173, 36, 10}},
	{"playerhealth_bar_5",   {203, 185, 36, 10}},
	{"playerhealth_bar_6empty",{203, 195, 36, 10}},
	{"timer_1full",          {15, 306, 45, 17}},
	{"timer_2",              {15, 325, 45, 17}},
	{"timer_3",              {15, 346, 45, 15}},
	{"timer_4",              {15, 365, 45, 15}},
	{"timer_5",              {14, 386, 45, 14}},
	{"timer_6empty",         {14, 410, 45, 9}},
	{"spider_enemy_idle1",   {251, 359, 16, 16}},
	{"spider_enemy_idle2",   {270, 359, 16, 16}},
	{"mouse_enemy_walk1",    {474, 277, 17, 12}},
	{"mouse_enemy_walk2",    {475, 322, 18, 11}},
	{"mouse_enemy_roll",     {439, 358, 15, 15}},
	{"play_button_aka_dig",  {31, 23, 74, 25}},
	{"leaderboard_button",   {31, 52, 74, 25}}
};

Texture2D assets_file;

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
	public:
		Vector2 position;
		Vector2 position_refmap;

		Player(Vector2 position) {
			this->position = position;
		}

		void draw() {
			AssetRect ar = assets["player_walk_1"];

			Rectangle source = { (float)ar.x, (float)ar.y, (float)ar.w, (float)ar.h };

			float scale = 8.0f;
			Rectangle dest = { 
				this->position.x, 
				this->position.y, 
				ar.w * scale, 
				ar.h * scale 
			};

			Vector2 origin = { (ar.w * scale) / 2.0f, (ar.h * scale) / 2.0f };

			DrawTexturePro(assets_file, source, dest, origin, 0.0f, WHITE);
		}
};

// Game functions begin here
void map_divide_recurse(std::vector<int>& rows, int start, int end) {
	if (start >= end) return;

	int picked = GetRandomValue(start, end);
	rows.push_back(picked);
	map_divide_recurse(rows, start + 2, picked - 2);
	map_divide_recurse(rows, picked + 2, end - 2);
}

RefMap generate_map() {
	RefMap refmap;
	for (int i = 0; i < REF_MAP_HEIGHT; i++)
		for (int j = 0; j < REF_MAP_WIDTH; j++)
			refmap.data[i][j] = BLANK_SPACE;

	std::vector<int> rows_marked;
	map_divide_recurse(rows_marked, 2, REF_MAP_HEIGHT - 3);

	const int SPIKE_CHANCE = 10; // 10% chance per tile

	// Mandatory rows
	for (int a = 0; a < REF_MAP_WIDTH; a++) refmap.data[0][a] = SURFACE;
	rows_marked.push_back(0);

	for (int a = 0; a < REF_MAP_WIDTH; a++) refmap.data[REF_MAP_HEIGHT - 1][a] = CORE;
	rows_marked.push_back(REF_MAP_HEIGHT - 1);

	std::sort(rows_marked.begin(), rows_marked.end());

	// Draw platforms and tunnels
	for (int i = 0; i < int(rows_marked.size()); i++) {
		if (refmap.data[rows_marked[i]][0] != CORE && refmap.data[i][0] != SURFACE) {
			for (int a = 0; a < REF_MAP_WIDTH; a++) {
				if (GetRandomValue(0, 99) < SPIKE_CHANCE) {
					refmap.data[rows_marked[i]][a] = PLATFORM_SPIKE;
				} else {
					refmap.data[rows_marked[i]][a] = PLATFORM;
				}
			}
		}

		if (i == rows_marked.size() - 1) continue;
		for (int c = 0; c < GetRandomValue(1, 5); c++) {
			int random_hit = GetRandomValue(0, REF_MAP_WIDTH - 1);
			while (refmap.data[rows_marked[i] + 1][random_hit] == TUNNEL ||
					refmap.data[rows_marked[i] + 1][random_hit - 1] == TUNNEL ||
					refmap.data[rows_marked[i] + 1][random_hit + 1] == TUNNEL) {
				random_hit = GetRandomValue(0, REF_MAP_WIDTH - 1);
			}
			for (int j = rows_marked[i] + 1; j < rows_marked[i + 1]; j++) {
				dbg(j, random_hit, rows_marked);
				refmap.data[j][random_hit] = TUNNEL;
			}
		}
	}

	return refmap;
}

// Sprite or other globals
Player player(raylib::Vector2(0.0, 0.0));
raylib::Camera2D camera(raylib::Vector2(SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f), player.position);

// Game functions
void game_init() {
	std::cout << "(DBG) GENERATED MAP: " << nl;
	camera.zoom = 1.0;

	auto map = generate_map();
	for (int y = 0; y < REF_MAP_HEIGHT; y++) {
		for (int x = 0; x < REF_MAP_WIDTH; x++) {
			char c;
			switch (map.data[y][x]) {
				case BLANK_SPACE: c = '.'; break;
				case WALL: c = '#'; break;
				case PLATFORM: c = '-'; break;
				case PLATFORM_SPIKE: c = '^'; break;
				case TUNNEL: c = '|'; break;
				case SURFACE: c = '"'; break;
				case CORE: c = '*'; break;
				default: c = '?'; break;
			}
			std::cout << c;
		}
		std::cout << '\n';
	}

	assets_file = LoadTexture(RESOURCES_FILE_PATH);
	SetTextureFilter(assets_file, TEXTURE_FILTER_POINT);
}

void game_draw() {
	BeginDrawing();
	ClearBackground(RAYWHITE);
	BeginMode2D(camera);
	player.draw();
	EndMode2D();
	EndDrawing();
}

void game_update() {
	camera.target = raylib::Vector2(
			std::clamp(player.position.x + 8, float(SCREEN_WIDTH/2.0), float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH - SCREEN_WIDTH/2.0)),
			std::clamp(player.position.y + 8, float(SCREEN_WIDTH/2.0), float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH - SCREEN_HEIGHT/2.0))
			);

	if (IsKeyDown(KEY_RIGHT) && player.position.x <= ACTUAL_TILE_PIXELS * REF_MAP_WIDTH) player.position.x += 10.0;
	if (IsKeyDown(KEY_LEFT)) player.position.x -= 10.0;

	player.position = raylib::Vector2(
			std::clamp(player.position.x, 0.0f, float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH)),
			std::clamp(player.position.y, 0.0f, float(ACTUAL_TILE_PIXELS * REF_MAP_WIDTH))
			);
}
