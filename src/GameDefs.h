#pragma once

#include <cstdint>
#include <Eigen/Eigen>
#include <opencv2/core.hpp>


namespace GameDefinitions{
    /// Statest for FSM to allow ony one ball to be present in game field
    enum class GameState : uint8_t {
        running = 0,            ///< ball is in the game
        waitingForPlayer = 1,   ///< No ball is in the game
        ended = 128,            ///< Closed by player
        win = 129,              ///< Player has won the game
        loose = 130,            ///< Player has lost the game
    };

    /// Information about size and position of the paddle on board
    struct PaddleProperties {
        float position{0};
        float size{0.5};
    };

    /// Information about size and position of the ball on board
    struct BallProperties {
        Eigen::Vector2f position{0,0};
        float radius {0.05f};
    };

    /// Information about size, position and color of a obstacle on board
    struct ObstacleProperties {
        Eigen::Vector2f position{0,0};
        Eigen::Vector2f size{1, 1};
        Eigen::Vector3i color {255,0,255};
    };

    /// Information about collision and its consequences
    struct CollisionInfo {
        float newDeltaT {0};    /// Remaining simulation time
        int points {0};         /// Points obtained from collision
        float paddleSize {0};   /// Paddle size modifier
        float ballSpeed {0};    /// Ball speed modifier
    };

    /// Representation of game state
    struct GameStatus {
        uint8_t balls{3};                               /// Players remaining attempts
        uint64_t score{0};                              /// Players current score
        GameState state{GameState::waitingForPlayer};   /// State of FSM controlling game start/end condition
    };
}
