#pragma once

#include <utility>
#include <iostream>

#include "GameSimulation.h"

namespace Game {
    class Obstacle : public ObstacleBase{
    public:
        Obstacle(GameDefinitions::ObstacleProperties properties)
                    : ObstacleBase(std::move(properties)) {
        }

        GameDefinitions::CollisionInfo collisionInfo() override {
            return GameDefinitions::CollisionInfo{0, m_properties.color.sum(), 0, 0};
        }
    };


    class SpeedIncrease : public ObstacleBase {
    public:
        SpeedIncrease(GameDefinitions::ObstacleProperties properties)
                    : ObstacleBase(std::move(properties)) {
            m_properties.color = {0, 0, 0};
        }

        GameDefinitions::CollisionInfo collisionInfo() override {
            return GameDefinitions::CollisionInfo{0, 0, 0, Game::Ball::kDefaultBallSpeed};
        }
    };

    class PaddleIncrease : public ObstacleBase {
    public:
        PaddleIncrease(GameDefinitions::ObstacleProperties properties)
                    : ObstacleBase(std::move(properties)) {
            m_properties.color = {255, 0, 0};
        }

        GameDefinitions::CollisionInfo collisionInfo() override {
            return GameDefinitions::CollisionInfo{0, 0, 0.2, 0};
        }
    };


}