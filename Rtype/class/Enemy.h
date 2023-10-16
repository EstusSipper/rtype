//
// Created by erwan on 10/15/23.
//

#pragma once

#include "../protocol.h"
#include "Cooldown.h"
#include "Missile.h"
#include <componentData.hpp>
#include <data.h>
#include <haze-core.hpp>
#include <net_data_channel.h>

using namespace asio::ip;
using namespace std::chrono_literals;

class Enemy {
public:
    Enemy(Haze::Engine &engine, network::data_channel<protocol::data> &channel);
    void build();
    void send();
    void shoot();
//    void sendUpdate();

public:// public members
    uint32_t _hp = 50; // useless
    Haze::Entity *_entity = nullptr;

private:
    std::vector<Missile> _missiles;
    Haze::Engine &_engine;
    network::data_channel<protocol::data> &_channel;
    Cooldown _missileCd{3000ms};
};
