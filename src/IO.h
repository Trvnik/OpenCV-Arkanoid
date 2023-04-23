#pragma once
#include <opencv2/highgui.hpp>

#include "GameSimulation.h"

namespace InputOutput {
    constexpr int kBorderSize = 10;
    constexpr int kBoardSize = 600;
    constexpr float kBordResolution =  kBoardSize / 2.f;
    constexpr int kHeaderSize = 100;

    constexpr int kWindowHeight = 2 * kBorderSize + kHeaderSize + kBoardSize;
    constexpr int kWindowWidth = 2 * kBorderSize + kBoardSize;

    class IO {
    public:
        IO(int targetFps, std::reference_wrapper<Game::GameSimulation> game);

        void render();
        void registerKey(int keyCode);

    private:
        std::chrono::milliseconds calculateWaitTime();
        cv::Mat prepareBoard() const;
        void renderPaddle(cv::Mat& canvas) const;
        void renderBall(cv::Mat& canvas) const;
        void renderObstacles(cv::Mat& canvas) const;

        std::reference_wrapper<Game::GameSimulation> m_game;
        std::string m_windowName{"Arkanoid"};

        std::chrono::steady_clock::duration m_frameDuration;
        std::chrono::steady_clock::time_point m_lastRenderTime{};
    };
}