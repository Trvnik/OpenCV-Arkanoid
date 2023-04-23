#pragma once
#include <functional>
#include <utility>
#include "GameDefs.h"

namespace Game {
    class ObstacleBase {
    public:
        virtual ~ObstacleBase() = default;
        explicit ObstacleBase(GameDefinitions::ObstacleProperties properties) : m_properties(std::move(properties)) {};

        const GameDefinitions::ObstacleProperties& properties() const {
            return m_properties;
        }
        virtual GameDefinitions::CollisionInfo collisionInfo() = 0;

    protected:
        GameDefinitions::ObstacleProperties m_properties{};
    };

    class Paddle {
    public:
        const GameDefinitions::PaddleProperties& properties() const {
            return m_properties;
        }
        void setSpeed(float speed) {
            m_speed = speed;
        }
        void setAcceleration(float acceleration) {
            m_acceleration = acceleration;
        }
        void changeSizeBy(float modifier) {
            m_properties.size += modifier;
        }
        void step(float deltaT);


    private:
        float m_speed{0};
        float m_acceleration{0};
        GameDefinitions::PaddleProperties m_properties{};
    };

    class Ball {
    public:
        static constexpr float kDefaultBallSpeed = 0.04;
        const GameDefinitions::BallProperties& properties() const {
            return m_properties;
        }
        void changeSpeedBy(float modifier) {
            m_speed += modifier;
        }
        std::optional<GameDefinitions::CollisionInfo> step(float deltaT, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>>& obstacles, const GameDefinitions::PaddleProperties& paddleProperties);
        void spawnBall(const Eigen::Vector2f& position);

    private:
        float newDeltaT(const Eigen::Vector2f& predictedPos) const;
        std::optional<GameDefinitions::CollisionInfo> obstaclesCollide(const Eigen::Vector2f& predictedPos, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>>& obstacles);
        std::optional<GameDefinitions::CollisionInfo> areaCollide(const Eigen::Vector2f& predictedPos, const GameDefinitions::PaddleProperties& paddleProperties);

        float m_speed{0};
        Eigen::Vector2f m_speedDirection{0, 0};
        GameDefinitions::BallProperties m_properties{};
    };

    class GameSimulation {
    public:
        GameSimulation(uint8_t balls, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>> obstacles);
        std::reference_wrapper<GameDefinitions::GameStatus> status() {
            return m_status;
        }
        std::reference_wrapper<Game::Paddle> paddle() {
            return m_paddle;
        }
        std::reference_wrapper<Game::Ball> ball() {
            return m_ball;
        }
        std::reference_wrapper<std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>>> obstacles() {
            return m_obstacles;
        }

        void step(float deltaT);

    private:
        void evaluateGameConditions();
        void applyCollisionEffects(const GameDefinitions::CollisionInfo& collisionInfo);

        GameDefinitions::GameStatus m_status;
        Game::Paddle m_paddle{};
        Game::Ball m_ball{};
        std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>> m_obstacles;
    };
}
