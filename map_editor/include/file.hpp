/*
** EPITECH PROJECT, 2020
** my_project
** File description:
** file
*/

#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>


#include "BossData.h"
#include "EnemyData.h"
#include "ExplosionData.h"
#include "Hitbox.h"
#include "MapData.h"
#include "ShotData.h"

#include "../../Client/json.hpp"
#include "ButtonElement.hpp"

#define UNIVERSAL_SCALE 3

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define SIZE_TILE 48
#define BUTTON_SIZE 100

// typedef struct enemy {
//     int type;
//     int x;
//     int y;
//     int velocity_x;
//     std::string move;
//     float move_time;
// } enemy;

typedef struct tile {
    int tile_top;
    int tile_bottom;
    std::vector<EnemyData> enemies;
    // std::vector<BossData> bosses;
} tile;

typedef struct game_data {
    std::string map_path;
    std::string parallax_path;

    // * tile
    int nmb_tiles;
    int selected_tile;
    std::vector<tile> tiles;
    bool selected_tile_is_top;

    // * enemy
    int id_enemy;
    int nmb_enemies;
    std::vector<EnemyData> enemies;
    std::vector<EnemyData> enemies_on_map;
    std::vector<sf::Texture> textures_enemy;

    // * wall
    int id_wall;
    int nmb_walls;
    std::vector<HitBox> wall_hitbox;
    std::vector<sf::Texture> textures_walls;
} game_data;
