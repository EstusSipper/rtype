/*
** ooooooooo.           ooooooooooooo
** `888   `Y88.         8'   888   `8
**  888   .d88'              888      oooo    ooo oo.ooooo.   .ooooo.
**  888ooo88P'               888       `88.  .8'   888' `88b d88' `88b
**  888`88b.    8888888      888        `88..8'    888   888 888ooo888
**  888  `88b.               888         `888'     888   888 888    .o
** o888o  o888o             o888o         .8'      888bod8P' `Y8bod8P'
**                                    .o..P'       888
**                                    `Y8P'       o888o
**
** R-Type
*/

#include "Rtype.hpp"

Rtype::Rtype(asio::io_context &context)
    : _channel(context), _engine(60)
{
    std::srand(std::time(0));
    _engine.init();
    _engine.setInfoInputs({std::vector<Haze::InputType>(), std::vector<Haze::InputType>(), Haze::MouseType::NOTHING, 0, 0}, 0);
    _engine.setInfoInputs({std::vector<Haze::InputType>(), std::vector<Haze::InputType>(), Haze::MouseType::NOTHING, 0, 0}, 1);
    _engine.setInfoInputs({std::vector<Haze::InputType>(), std::vector<Haze::InputType>(), Haze::MouseType::NOTHING, 0, 0}, 2);
    _engine.setInfoInputs({std::vector<Haze::InputType>(), std::vector<Haze::InputType>(), Haze::MouseType::NOTHING, 0, 0}, 3);
    _engine.setInfoInputs({std::vector<Haze::InputType>(), std::vector<Haze::InputType>(), Haze::MouseType::NOTHING, 0, 0}, 4);

    _background = std::make_unique<Paralax>(_engine, _channel);
}

Rtype::~Rtype() = default;

void printInfoInputs(const Haze::info_inputs_weak &info)
{
    for (const auto &input: info.pressedInputs) {
        std::cout << static_cast<int>(input) << " ";
    }
    std::cout << std::endl;

    std::cout << "Released Inputs: ";
    for (const auto &input: info.releasedInputs) {
        std::cout << static_cast<int>(input) << " ";
    }
    std::cout << std::endl;

    std::cout << "Mouse Type: " << static_cast<int>(info.mouseType) << std::endl;
    std::cout << "Mouse Coordinates: (" << info.x << ", " << info.y << ")" << std::endl;
}

void Rtype::checkInactiveClients()
{
    for (auto &player: _players) {
        if (player->_remote && player->_remote->activityCd.IsReady()) {
            player->_remote = nullptr;
        }
    }
}

void Rtype::stop()
{
    _running = false;
}

uint32_t Rtype::getPlayerID(const udp::endpoint &endpoint)
{
    for (auto &player: _players) {
        if (player->_remote && player->_remote->endpoint == endpoint) {
            return player->_id;
        }
    }
    return 0;
}

void Rtype::sendEverything(udp::endpoint &to)
{
    _background->send();
    for (auto &wall: _walls) {
        wall->send();
    }

    /**
     * Checking for missiles entity may be useless as it is
     * cleaned up in the player and enemy's update method
     * cf: here
     */
    for (auto &player: _players) {
        if (player->_entity) {
            player->send();
        }
        for (auto &missile: player->_missiles) {
            if (missile->_entity) {// <-- here
                missile->send();
            }
        }
    }
    for (auto &enemy: _enemies) {
        if (enemy->_entity) {
            enemy->send();
        }
        for (auto &missile: enemy->_missiles) {
            if (missile->_entity) {// <-- here
                missile->send();
            }
        }
    }
}

void Rtype::updateMap()
{
}

void Rtype::createMap()
{
    // Load sprite data for the walls from "ground.json"
    std::ifstream groundFile("assets/AnimationJSON/ground.json");
    nlohmann::json groundData;

    if (!groundFile.is_open()) {
        std::cerr << "Error: Could not open 'ground.json' for reading" << std::endl;
        return;
    }
    groundFile >> groundData;
    groundFile.close();

    // Load map data from "map.json"
    std::ifstream mapFile("assets/AnimationJSON/map.json");
    nlohmann::json mapData;
    if (!mapFile.is_open()) {
        std::cerr << "Error: Could not open 'map.json' for reading" << std::endl;
        return;
    }
    mapFile >> mapData;
    mapFile.close();

    // Retrieve the array of tiles from the map data
    nlohmann::json mapTiles = mapData["map"];
    uint16_t tileIndex = 0;

    // Iterate through each tile in the map
    for (const auto &tile: mapTiles) {
        // Create and position the top wall
        _walls.emplace_back(std::make_unique<Wall>(_engine, _channel, groundData, (48 * 3) * tileIndex, 0, false));
        _walls.back()->build(tile["tile_top"]);

        // Create and position the bottom wall
        _walls.emplace_back(std::make_unique<Wall>(_engine, _channel, groundData, (48 * 3) * tileIndex, 600, true));
        _walls.back()->build(tile["tile_bottom"]);

        // Print the tile index
        std::cout << "Tile " << tileIndex << " created." << std::endl;
        tileIndex++;
    }

    std::cout << "Map successfully created." << std::endl;
}

void Rtype::start()
{
    _running = true;
    _background->build();

    createMap();

    _enemies.emplace_back(std::make_unique<Enemy>(_engine, _channel));
    _enemies.back()->build();

    /**
      * Update Cycle
      */
    while (_running) {
        // Process messages
        size_t count = 0;
        while (count < 5 && !_channel.getIncoming().empty()) {
            auto msg = _channel.getIncoming().pop_front();
            onReceive(msg.target, msg.data);
            count++;
        }
        checkInactiveClients();

        // Process the input received
        _engine.update();

        // Update the state of the non player entity
        update();

        // Send all entities update to clients
        sendUpdate();
    }
}

void Rtype::onReceive(udp::endpoint from, network::datagram<protocol::data> content)
{
    // refresh player's activity
    for (auto &player: _players) {
        if (player->_remote && player->_remote->endpoint == from)
            player->_remote->activityCd.Activate();
    }

    // process datagram
    switch (content.header.id) {
        case protocol::data::join: {
            for (auto &player: _players) {
                if (!player->_remote) {
                    player->_remote = std::make_unique<Player::Remote>(from);
                    _channel.getGroup().insert(from);
                    sendEverything(from);
                }
            }
            if (_players.size() < 4) {
                _channel.getGroup().insert(from);
                sendEverything(from);
                _players.emplace_back(std::make_unique<Player>(_engine, _channel, _players.size() + 1));
                _players.back()->_remote = std::make_unique<Player::Remote>(from);
                _players.back()->build();
            }
            return;
        }

        case protocol::data::input: {
            Haze::info_inputs_weak info{};
            std::memcpy(&info, content.body.data(), content.header.size);
            //            printInfoInputs(info);

            Haze::info_inputs inputs;
            for (auto &key: info.pressedInputs) {
                if (key != Haze::NO) {
                    inputs.inputsPressed.push_back(key);
                }
            }
            for (auto &key: info.releasedInputs) {
                if (key != Haze::NO) {
                    inputs.inputsReleased.push_back(key);
                }
            }
            inputs.mouseType = info.mouseType;
            inputs.x = info.x;
            inputs.y = info.y;

            uint32_t id = getPlayerID(from);
            if (!id) return;
            _engine.setInfoInputs(inputs, id);
            break;
        }
        default: {
            break;
        }
    }
}

asio::ip::udp::endpoint Rtype::getEndpoint() const
{
    return _channel.getEndpoint();
}

void Rtype::sendUpdate()
{
    _background->sendUpdate();
    for (auto &player: _players) {
        if (player->_entity) {
            player->sendUpdate();
        }
    }
    //_background->sendUpdate();
    for (auto &wall: _walls) {
        wall->sendUpdate();
    }
}

void Rtype::update()
{
    /**
     * Enemy & Missiles' cleanup cycle
     */
    bool enemyToCleanup = false;
    for (auto &enemy: _enemies) {
        // Cleanup unreachable missiles
        enemy->update();
        if (enemy->_isDead) {
            // * create explosion

            _explosions.emplace_back(std::make_unique<Explosion>(_engine, _channel, enemy->_pos_x, enemy->_pos_y));
            _explosions.back()->build();
            _explosions.back()->send();
            std::cout << "Explosion created" << std::endl;
            enemy->_isDead = false;
        }
        // Cleanup unreachable enemies
        if (enemy->_entity) {
            auto enemyPos = dynamic_cast<Haze::Position *>(enemy->_entity->getComponent("Position"));
            if (enemyPos->x <= -50) {
                _channel.sendGroup(RType::message::deleteEntity(enemy->_entity->getId()));
                enemy->_entity->addComponent(new Haze::Destroy());
                enemy->_entity = nullptr;
            }
        }

        // Cleanup enemy Instance if it contains nothing
        if (!enemy->_entity && enemy->_missiles.empty()) {
            enemy.reset();
            enemyToCleanup = true;
        }
    }
    if (enemyToCleanup)
        _enemies.erase(std::remove(_enemies.begin(), _enemies.end(), nullptr), _enemies.end());

    // Trigger enemy actions
    for (auto &enemy: _enemies) {
        if (enemy->_entity) {
            enemy->shoot();
        }
    }

    for (auto &explosion: _explosions) {
        if (explosion->_time_to_destroyCd.IsReady()) {
            explosion->destroy();
            explosion = nullptr;
            std::cout << "explosion->_time_to_destroyCd.IsReady == true\n";
        }
    }
    // * remove explosions null
    _explosions.erase(std::remove_if(_explosions.begin(), _explosions.end(), [](const std::unique_ptr<Explosion> &explosion) {
                          return explosion == nullptr;
                      }),
                      _explosions.end());

    /**
     * Enemy & Missiles' cleanup cycle
     */
    bool playerToCleanup = false;
    for (auto &player: _players) {
        // Cleanup unreachable missiles
        player->update();

        // Cleanup player Instance if it contains nothing
        if (!player->_entity && player->_missiles.empty()) {
            player.reset();
            playerToCleanup = true;
        }
    }
    if (playerToCleanup)
        _players.erase(std::remove(_players.begin(), _players.end(), nullptr), _players.end());

    /**
     * Background's motion cycle
     */
    _background->update();

    /**
     * Spawn random enemies
     */
    if (_enemySpawnCD.IsReady()) {
        std::chrono::milliseconds newDuration((std::rand() % 6 + 3) * 1000);
        _enemySpawnCD.setDuration(newDuration);
        _enemySpawnCD.Activate();

        int32_t enemyNb = std::rand() % 3 + 1;
        for (int32_t i = 0; i < enemyNb; i++) {
            _enemies.emplace_back(std::make_unique<Enemy>(_engine, _channel));
            _enemies.back()->build();
        }
    }
}
