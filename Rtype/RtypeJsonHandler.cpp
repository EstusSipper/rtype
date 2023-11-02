//
// Created by Niels Ouvrard on 31/10/2023.
//

#include "Rtype.hpp"

// ? is type useless because we can use the map key ?

// damage
// type
// life
// path_sprite
// hitBoxData
// shot_type
// fly
void json_fill_EnemyData(EnemyData &data, nlohmann::json jsonData)
{
    data.damage = 0;
    data.life = 1;
    data.path_sprite = "";
    data.hitBoxData = {};
    data.shot_type = -1;
    data.fly = true;
    data.explosion_type = 0;
    data.type = jsonData["type"];

    if (jsonData.contains("damage")) {
        data.damage = jsonData["damage"];
    }
    if (jsonData.contains("life")) {
        data.life = jsonData["life"];
    }
    if (jsonData.contains("path_sprite")) {
        data.path_sprite = jsonData["path_sprite"];
    }
    if (jsonData.contains("hitbox")) {
        if (jsonData["hitbox"].contains("x"))
            data.hitBoxData.x = jsonData["hitbox"]["x"];
        if (jsonData["hitbox"].contains("y"))
            data.hitBoxData.y = jsonData["hitbox"]["y"];
        if (jsonData["hitbox"].contains("width"))
            data.hitBoxData.width = jsonData["hitbox"]["width"];
        if (jsonData["hitbox"].contains("height"))
            data.hitBoxData.height = jsonData["hitbox"]["height"];
    }
    if (jsonData.contains("shot_type")) {
        data.shot_type = jsonData["shot_type"];
    }
    if (jsonData.contains("fly")) {
        data.fly = jsonData["fly"];
    }
    if (jsonData.contains("explosion_type")) {
        data.explosion_type = jsonData["explosion_type"];
    }
}

// type
// path_sprite
void json_fill_ExplosionData(ExplosionData &data, nlohmann::json jsonData)
{
    data.path_sprite = "";
    data.type = jsonData["type"];

    if (jsonData.contains("path_sprite")) {
        data.path_sprite = jsonData["path_sprite"];
    }
}

// type
// path_sprite
// hitBoxData
void json_fill_ShotData(ShotData &data, nlohmann::json jsonData)
{
    data.path_sprite = "";
    data.hitBoxData = {};
    data.type = jsonData["type"];
    data.velocity = 0;
    data.damage = 0;

    if (jsonData.contains("path_sprite")) {
        data.path_sprite = jsonData["path_sprite"];
    }
    if (jsonData.contains("hitbox")) {
        if (jsonData["hitbox"].contains("x"))
            data.hitBoxData.x = jsonData["hitbox"]["x"];
        if (jsonData["hitbox"].contains("y"))
            data.hitBoxData.y = jsonData["hitbox"]["y"];
        if (jsonData["hitbox"].contains("width"))
            data.hitBoxData.width = jsonData["hitbox"]["width"];
        if (jsonData["hitbox"].contains("height"))
            data.hitBoxData.height = jsonData["hitbox"]["height"];
    }
    if (jsonData.contains("velocity")) {
        data.velocity = jsonData["velocity"];
    }
    if (jsonData.contains("damage")) {
        data.damage = jsonData["damage"];
    }
}

// damage
// type
// life
// path_sprite
// hitBoxData
void json_fill_BossData(BossData &data, nlohmann::json jsonData)
{
    data.damage = 0;
    data.life = 1;
    data.path_sprite = "";
    data.hitBoxData = {};
    data.type = jsonData["type"];

    if (jsonData.contains("damage")) {
        data.damage = jsonData["damage"];
    }
    if (jsonData.contains("life")) {
        data.life = jsonData["life"];
    }
    if (jsonData.contains("path_sprite")) {
        data.path_sprite = jsonData["path_sprite"];
    }
    if (jsonData.contains("hitbox")) {
        if (jsonData["hitbox"].contains("x"))
            data.hitBoxData.x = jsonData["hitbox"]["x"];
        if (jsonData["hitbox"].contains("y"))
            data.hitBoxData.y = jsonData["hitbox"]["y"];
        if (jsonData["hitbox"].contains("width"))
            data.hitBoxData.width = jsonData["hitbox"]["width"];
        if (jsonData["hitbox"].contains("height"))
            data.hitBoxData.height = jsonData["hitbox"]["height"];
    }
}

// * Create functions


template<typename T>
void Rtype::createObject(std::ifstream &fileStream, const std::string &filePath, std::map<uint16_t, T> &objectMap, std::function<void(T &, const nlohmann::json &)> fillDataFunc)
{
    nlohmann::json jsonData;
    fileStream >> jsonData;

    if (!jsonData.contains("type")) {
        return;
    }
    T new_object_type;
    fillDataFunc(new_object_type, jsonData);
    uint16_t type = jsonData["type"];

    new_object_type.path_json = filePath;
    objectMap[type] = (new_object_type);
}

void Rtype::jsonHandler()
{
    // create enemies
    processJsonFilesInDirectory("assets/json_files/enemies", [this](std::ifstream &fileStream, const std::string &filePath) {
        this->createObject<EnemyData>(fileStream, filePath, _typeEntities.enemies, json_fill_EnemyData);
    });
    // create explosions
    processJsonFilesInDirectory("assets/json_files/explosions", [this](std::ifstream &fileStream, const std::string &filePath) {
        this->createObject<ExplosionData>(fileStream, filePath, _typeEntities.explosions, json_fill_ExplosionData);
    });
    // create shots
    processJsonFilesInDirectory("assets/json_files/shots", [this](std::ifstream &fileStream, const std::string &filePath) {
        this->createObject<ShotData>(fileStream, filePath, _typeEntities.shots, json_fill_ShotData);
    });
    // create bosses
    processJsonFilesInDirectory("assets/json_files/bosses", [this](std::ifstream &fileStream, const std::string &filePath) {
        this->createObject<BossData>(fileStream, filePath, _typeEntities.bosses, json_fill_BossData);
    });
}