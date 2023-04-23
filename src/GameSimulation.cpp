#include "GameSimulation.h"

#include <cmath>
#include <iostream>
#include <utility>

namespace {
    Eigen::Vector2f reflect(const Eigen::Vector2f& dir, const Eigen::Vector2f& norm) {
        return dir - 2 * (dir.dot(norm)) * norm;
    }

    inline float distanceFromLineSegment(const Eigen::Vector2f& a,
                                         const Eigen::Vector2f& b,
                                         const Eigen::Vector2f& point) {
        double const lineLength = (b - a).norm();
        auto vector = point - a;
        auto lineDirection = (b - a) / lineLength;
        double const distance = vector.dot(lineDirection);

        if (distance <= (0.0)) {
            return vector.norm();
        }
        if (distance >= lineLength) {
            return (point - b).norm();
        }
        return (point - (a + (lineDirection * distance))).norm();
    }
}

namespace Game {
    void Paddle::step(float deltaT) {
        auto newSpeed = m_speed + m_acceleration * deltaT;
        auto deltaPosition = deltaT * m_speed;
        auto predictedPos = m_properties.position += deltaPosition;

        // Bounce
        auto edgePosition = predictedPos + std::copysign(m_properties.size / 2.f, predictedPos);
        if (std::abs(edgePosition) >= 1.f) {
            m_properties.position = std::copysign(1.f - m_properties.size / 2.f, edgePosition);
            auto newDeltaT = (edgePosition - std::copysign(1.f, predictedPos)) / m_speed;
            m_speed = -newSpeed / 2.f;
            step(newDeltaT);
        } else {
            m_speed = newSpeed;
            m_properties.position = predictedPos;
        }
    }

    std::optional<GameDefinitions::CollisionInfo> Ball::step(float deltaT, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>>& obstacles, const GameDefinitions::PaddleProperties& paddleProperties) {
        auto deltaPosition = deltaT * m_speed * m_speedDirection;
        const auto predictedPos = m_properties.position += deltaPosition;

        auto obstacleCollision = obstaclesCollide(predictedPos, obstacles);
        if (obstacleCollision.has_value()) {
            return obstacleCollision;
        }
        return areaCollide(predictedPos, paddleProperties);
    }

    void Ball::spawnBall(const Eigen::Vector2f& position) {
        m_speed = kDefaultBallSpeed;
        m_speedDirection = Eigen::Vector2f::Random();
        m_speedDirection.normalize();
        m_properties.position = position + m_speedDirection * std::numeric_limits<float>::epsilon();
    }

    float Ball::newDeltaT(const Eigen::Vector2f& predictedPos) const {
        return (predictedPos - m_properties.position).norm() / m_speed;
    }

    std::optional<GameDefinitions::CollisionInfo> Ball::obstaclesCollide(const Eigen::Vector2f& predictedPos, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>>& obstacles) {
        auto obstacle = obstacles.begin();

        while (obstacle != obstacles.end()) {
            Eigen::Vector2f const nearestPoint(std::clamp(predictedPos.x(), obstacle->second->properties().position.x(), obstacle->second->properties().position.x() + obstacle->second->properties().size.x()),
                                               std::clamp(predictedPos.y(), obstacle->second->properties().position.y(), obstacle->second->properties().position.y() + obstacle->second->properties().size.y()));

            Eigen::Vector2f const dir = nearestPoint - predictedPos;
            float overlap = m_properties.radius - dir.norm();

            if (std::isnan(overlap)) {
                overlap = 0;
            }
            if (overlap >= 0.f) {
                m_speedDirection = reflect(m_speedDirection, dir.normalized());
                m_properties.position = predictedPos - dir.normalized() * overlap;
                auto collisionInfo = obstacle->second->collisionInfo();
                collisionInfo.newDeltaT = newDeltaT(predictedPos);
                obstacles.erase(obstacle);
                return collisionInfo;
            }

            obstacle++;
        }
        return std::nullopt;
    }

    std::optional<GameDefinitions::CollisionInfo> Ball::areaCollide(const Eigen::Vector2f& predictedPos, const GameDefinitions::PaddleProperties& paddleProperties) {
        auto edgeXPosition = predictedPos.x() + std::copysign(m_properties.radius, predictedPos.x());
        auto edgeYPosition = predictedPos.y() + std::copysign(m_properties.radius, predictedPos.y());
        // Side bounce
        if (std::abs(edgeXPosition) >= 1.f) {
            m_properties.position = Eigen::Vector2f(std::copysign(1.f - m_properties.radius, edgeXPosition), predictedPos.y());
            m_speedDirection = reflect(m_speedDirection, Eigen::Vector2f(-1, 0));
            return GameDefinitions::CollisionInfo{newDeltaT(predictedPos), 0, 0, 0};
        }

        // Top bounce
        if (edgeYPosition <= -1.f) {
            m_properties.position = Eigen::Vector2f(predictedPos.x(), -1.f + m_properties.radius);
            m_speedDirection = reflect(m_speedDirection, Eigen::Vector2f(0, -1));
            return GameDefinitions::CollisionInfo{newDeltaT(predictedPos), 0, 0, 0};
        }

        // Paddle bounce
        if (edgeYPosition >= 1.f &&
            distanceFromLineSegment(Eigen::Vector2f(paddleProperties.position - paddleProperties.size / 2.f, 1.f), Eigen::Vector2f(paddleProperties.position + paddleProperties.size / 2.f, 1.f), predictedPos) < m_properties.radius) {
            m_properties.position = Eigen::Vector2f(predictedPos.x(), 1.f - m_properties.radius);
            m_speedDirection = reflect(m_speedDirection, Eigen::Vector2f(0, -1));
            return GameDefinitions::CollisionInfo{newDeltaT(predictedPos), 0, 0, 0};
        }

        m_properties.position = predictedPos;
        return std::nullopt;
    }

    void GameSimulation::step(float deltaT) {
        m_paddle.step(deltaT);

        if (m_status.state == GameDefinitions::GameState::running) {
            while (deltaT > 0) {
                auto collisionInfo = m_ball.step(deltaT, m_obstacles, m_paddle.properties());
                if (collisionInfo.has_value()) {
                    applyCollisionEffects(collisionInfo.value());
                    deltaT = collisionInfo->newDeltaT;
                } else {
                    deltaT = -1;
                }
            }

            evaluateGameConditions();
        }
    }


    GameSimulation::GameSimulation(uint8_t balls, std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>> obstacles)
                : m_status({balls, 0, GameDefinitions::GameState::waitingForPlayer})
                , m_obstacles(std::move(obstacles)) {
    }

    void GameSimulation::evaluateGameConditions() {
        if (m_status.state == GameDefinitions::GameState::running && m_ball.properties().position.y() >= 1.f) {
            m_status.balls--;
            if (m_status.balls) {
                m_status.state = GameDefinitions::GameState::waitingForPlayer;
            } else {
                m_status.state = GameDefinitions::GameState::loose;
            }
        }

        if (m_status.state == GameDefinitions::GameState::running && m_obstacles.empty()) {
            m_status.state = GameDefinitions::GameState::win;
        }
    }

    void GameSimulation::applyCollisionEffects(const GameDefinitions::CollisionInfo& collisionInfo) {
        m_status.score += collisionInfo.points;
        m_paddle.changeSizeBy(collisionInfo.paddleSize);
        m_ball.changeSpeedBy(collisionInfo.ballSpeed);
    }
}
