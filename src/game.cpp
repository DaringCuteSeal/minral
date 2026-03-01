#include "prelude.hpp"
#include "cfg.hpp"
#include <raylib.h>
#include <raymath.h>
#include <dbg.h>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <vector>
#include <iostream>

#define nl '\n'
#define REF_MAP_HEIGHT 51
#define REF_MAP_WIDTH 50
#define ACTUAL_TILE_PIXELS 128 // 16px * 8x scale

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
    {"leaderboard_button",   {31, 52, 74, 25}},
    {"leaderboard_time",     {209, 44, 161, 94}},
    {"results",              {50, 96, 161, 94}},
    {"leaderboard_kills",    {231, 174, 161, 94}},
    {"cordtum",              {30, 3, 72, 18}}
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
        Vector2 velocity = {0, 0};
        bool isGrounded = false;

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
            // Drawing from top-left for easier collision alignment
            DrawTexturePro(assets_file, source, dest, {0, 0}, 0.0f, WHITE);
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
    const int SPIKE_CHANCE = 10; 

    for (int a = 0; a < REF_MAP_WIDTH; a++) refmap.data[0][a] = SURFACE;
    for (int a = 0; a < REF_MAP_WIDTH; a++) refmap.data[REF_MAP_HEIGHT - 1][a] = CORE;

    for (int row_idx : rows_marked) {
        for (int a = 0; a < REF_MAP_WIDTH; a++) {
            if (GetRandomValue(0, 99) < SPIKE_CHANCE) {
                refmap.data[row_idx][a] = PLATFORM_SPIKE;
            } else {
                refmap.data[row_idx][a] = PLATFORM;
            }
        }
    }

    for (size_t i = 0; i < rows_marked.size(); i++) {
        if (i == rows_marked.size() - 1) continue;
        for (int c = 0; c < GetRandomValue(1, 3); c++) {
            int random_hit = GetRandomValue(1, REF_MAP_WIDTH - 2);
            for (int j = rows_marked[i] + 1; j < rows_marked[i+1]; j++) {
                refmap.data[j][random_hit] = TUNNEL;
            }
        }
    }
    return refmap;
}

// Sprite or other globals
RefMap current_map;
Player player(Vector2{ACTUAL_TILE_PIXELS, -100}); 
Camera2D camera = { 0 };

// Game functions
void game_init() {
    std::cout << "(DBG) GENERATED MAP: " << nl;
    camera.zoom = 1.0;
    camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

    current_map = generate_map();
    assets_file = LoadTexture(RESOURCES_FILE_PATH);
    SetTextureFilter(assets_file, TEXTURE_FILTER_POINT);
}

void game_draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera);
    
    // 9x9 Tile Rendering (Culling)
    int pX = (int)(player.position.x / ACTUAL_TILE_PIXELS);
    int pY = (int)(player.position.y / ACTUAL_TILE_PIXELS);

    for (int y = pY - 4; y <= pY + 4; y++) {
        for (int x = pX - 4; x <= pX + 4; x++) {
            if (x < 0 || x >= REF_MAP_WIDTH || y < 0 || y >= REF_MAP_HEIGHT) continue;
            
            RefMapAttr attr = current_map.data[y][x];
            if (attr == BLANK_SPACE) continue;

            std::string asset_key = "wall1"; 
            if (attr == SURFACE) asset_key = "surface";
            if (attr == CORE) asset_key = "core_earth";
            if (attr == TUNNEL) asset_key = "wall2";
            if (attr == PLATFORM_SPIKE) asset_key = "spike";

            AssetRect ar = assets[asset_key];
            Rectangle src = {(float)ar.x, (float)ar.y, (float)ar.w, (float)ar.h};
            Rectangle dst = {(float)x * ACTUAL_TILE_PIXELS, (float)y * ACTUAL_TILE_PIXELS, 16 * 8.0f, 16 * 8.0f};
            
            DrawTexturePro(assets_file, src, dst, {0,0}, 0.0f, WHITE);
        }
    }

    player.draw();
    EndMode2D();
    EndDrawing();
}

void game_update() {
    // Basic Movement Logic
    float moveSpeed = 10.0f;
    if (IsKeyDown(KEY_RIGHT)) player.velocity.x = moveSpeed;
    else if (IsKeyDown(KEY_LEFT)) player.velocity.x = -moveSpeed;
    else player.velocity.x = 0;

    // Gravity
    player.velocity.y += 0.8f; 
    if (IsKeyPressed(KEY_SPACE) && player.isGrounded) player.velocity.y = -20.0f;

    // Resolve X Collision
    player.position.x += player.velocity.x;
    
    // Resolve Y Collision
    player.position.y += player.velocity.y;
    
    // Tile-based Floor Collision (Simplified)
    int footX = (int)((player.position.x + 64) / ACTUAL_TILE_PIXELS);
    int footY = (int)((player.position.y + 128) / ACTUAL_TILE_PIXELS);
    
    if (footY < REF_MAP_HEIGHT && current_map.data[footY][footX] != BLANK_SPACE) {
        player.position.y = (footY * ACTUAL_TILE_PIXELS) - 128;
        player.velocity.y = 0;
        player.isGrounded = true;
    } else {
        player.isGrounded = false;
    }

    // Camera Clamp & Follow
    camera.target = Vector2{ 
        std::clamp(player.position.x + 64, (float)SCREEN_WIDTH/2, (float)(REF_MAP_WIDTH * ACTUAL_TILE_PIXELS - SCREEN_WIDTH/2)),
        std::clamp(player.position.y + 64, player.position.y + 64, (float)(REF_MAP_HEIGHT * ACTUAL_TILE_PIXELS - SCREEN_HEIGHT/2))
    };
}
