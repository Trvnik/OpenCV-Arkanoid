#include "IO.h"
#include "GameExtensions.h"

#include <iostream>
#include <random>

constexpr int fps = 30;
constexpr int simulationsPerFrame = 10;

int main(int, char* []) {

    srand(std::chrono::system_clock::now().time_since_epoch().count());

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution uniform(0, 255);

    std::unordered_map<uint64_t,std::unique_ptr<Game::ObstacleBase>> obstacles;
    obstacles.reserve(7*4);
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 4; ++j) {
            GameDefinitions::ObstacleProperties const properties{
                {-0.85 + i*0.25 , -0.85 + j*0.25},
                {0.2,0.2},
                {uniform(generator), uniform(generator), uniform(generator)}
            };

            std::unique_ptr<Game::ObstacleBase> ptr;
            if (i == 3 && j == 1) {
                ptr = std::make_unique<Game::SpeedIncrease>(properties);
            } else if (i == 3 && j == 3 ) {
                ptr = std::make_unique<Game::PaddleIncrease>(properties);
            } else {
                ptr = std::make_unique<Game::Obstacle>(properties);
            }
            uint64_t id = reinterpret_cast<uint64_t>((ptr.get()));
            obstacles.emplace(id, std::move(ptr));
        }
    }

    Game::GameSimulation game{3, std::move(obstacles)};
    InputOutput::IO window{fps, game};

    while(game.status().get().state < GameDefinitions::GameState::ended) {
        window.render();
        for(auto i = 0; i < simulationsPerFrame; i++) {
            game.step(1/static_cast<float>(simulationsPerFrame));
        }
    }

    switch (game.status().get().state) {
        case GameDefinitions::GameState::ended:
            std::cout << "Player has ended the game" << std::endl;
            break;
        case GameDefinitions::GameState::win:
            std::cout << "Player has won the game with score: " << game.status().get().score  << std::endl;
            break;
        case GameDefinitions::GameState::loose:
            std::cout << "Player has lost the game with score: " << game.status().get().score << std::endl;
            break;
        default:
            std::cout << "Game has ended unexpectedly." << std::endl;
            break;
    }

    return 0;
}