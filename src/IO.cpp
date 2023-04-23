#include "IO.h"

#include <opencv2/imgproc.hpp>
#include <iostream>
#include <iomanip>
#include <thread>

namespace {
    constexpr int64_t kNecInSec = 1'000'000'000;

    constexpr std::chrono::steady_clock::duration fromRatePerSecond(double value) {
        return std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double, std::nano>(kNecInSec / value));
    }

    cv::Point toWindowCoords(const Eigen::Vector2f& point) {
        const Eigen::Vector2f renderCoords = point * InputOutput::kBordResolution;
        return cv::Point(renderCoords.x() + InputOutput::kWindowWidth / 2, renderCoords.y() + InputOutput::kBoardSize / 2 + InputOutput::kBorderSize + InputOutput::kHeaderSize);
    }
}

namespace InputOutput {
    IO::IO(int targetFps, std::reference_wrapper<Game::GameSimulation> game)
                : m_game(game)
                , m_frameDuration(fromRatePerSecond(targetFps)) {
    }

    void IO::render() {
        auto canvas = prepareBoard();
        renderPaddle(canvas);
        renderBall(canvas);
        renderObstacles(canvas);

        cv::imshow(m_windowName, canvas);
        m_game.get().paddle().get().setAcceleration(0);
        registerKey(cv::waitKeyEx(calculateWaitTime().count()));
        m_lastRenderTime = std::chrono::steady_clock::now();
    }

    std::chrono::milliseconds IO::calculateWaitTime() {
        auto durationSinceLast = std::chrono::steady_clock::now() - m_lastRenderTime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_frameDuration - durationSinceLast);
    }

    void IO::registerKey(int keyCode) {
        switch (keyCode) {
            case 537919515: /// ESC
                m_game.get().status().get().state = GameDefinitions::GameState::ended;
                break;
            case 537919520: /// Space
                if (m_game.get().status().get().state == GameDefinitions::GameState::waitingForPlayer) {
                    m_game.get().status().get().state = GameDefinitions::GameState::running;
                    m_game.get().ball().get().spawnBall(Eigen::Vector2f(m_game.get().paddle().get().properties().position, 1.0 - m_game.get().ball().get().properties().radius));
                }
                break;
            case 537984849: /// Left arrow
                if (m_game.get().status().get().state < GameDefinitions::GameState::ended) {
                    m_game.get().paddle().get().setAcceleration(-0.01);
                }
                break;
            case 537984851: /// Right arrow
                if (m_game.get().status().get().state < GameDefinitions::GameState::ended) {
                    m_game.get().paddle().get().setAcceleration(0.01);
                }
                break;
            case 537984852: /// Down arrow
                if (m_game.get().status().get().state < GameDefinitions::GameState::ended) {
                    m_game.get().paddle().get().setSpeed(0);
                }
            case -1: /// Nothing was pressed
                break;
            default:
                std::cout << "Unsupported key pressed: " << keyCode << std::endl;
                break;
        }

        //TODO: Investigate wierd rendering freeze
        //std::this_thread::sleep_for(calculateWaitTime());
    }

    cv::Mat IO::prepareBoard() const {
        cv::Mat canvas = cv::Mat::zeros(kWindowHeight, kWindowWidth, CV_8UC3);

        std::stringstream balls;
        balls << "Balls: " << std::setw(2) << std::setfill('0') << static_cast<uint64_t>(m_game.get().status().get().balls);

        std::stringstream score;
        score << "Score: " << std::setw(6) << std::setfill('0') << m_game.get().status().get().score;

        cv::putText(canvas, balls.str(), cv::Point(kBorderSize, kHeaderSize / 2 + 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 2);
        cv::putText(canvas, score.str(), cv::Point(kWindowWidth - 250, kHeaderSize / 2 + 10), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 2);

        cv::rectangle(canvas, cv::Point(0, kHeaderSize), cv::Point(2 * kBorderSize + kBoardSize, kHeaderSize + 2 * kBorderSize + kBoardSize), cv::Scalar(128, 128, 128), cv::FILLED);
        cv::rectangle(canvas, cv::Point(kBorderSize, kHeaderSize + kBorderSize), cv::Point(kBorderSize + kBoardSize, kWindowHeight), cv::Scalar(255, 255, 255), cv::FILLED);

        return canvas;
    }

    void IO::renderPaddle(cv::Mat& canvas) const {
        const int x1 = kWindowWidth / 2 + floor(kBordResolution * (m_game.get().paddle().get().properties().position - m_game.get().paddle().get().properties().size / 2.f));
        const int x2 = kWindowWidth / 2 + ceil(kBordResolution * (m_game.get().paddle().get().properties().position + m_game.get().paddle().get().properties().size / 2.f));

        const cv::Rect paddle(cv::Point(x1, kBorderSize + kHeaderSize + kBoardSize), cv::Point(x2, kWindowHeight));
        cv::rectangle(canvas, paddle, cv::Scalar(255, 0, 0), cv::FILLED, 0);
    }

    void IO::renderBall(cv::Mat& canvas) const {
        if (m_game.get().status().get().state != GameDefinitions::GameState::running) {
            return;
        }

        cv::circle(
            canvas,
            toWindowCoords(m_game.get().ball().get().properties().position),
            m_game.get().ball().get().properties().radius * kBordResolution,
            cv::Scalar(0, 0, 255),
            cv::FILLED, 0);
    }

    void IO::renderObstacles(cv::Mat& canvas) const {
        for (const auto& [id, obstacle] : m_game.get().obstacles().get()) {
            const auto p1 = toWindowCoords(obstacle->properties().position);
            const auto p2 = toWindowCoords(obstacle->properties().position + obstacle->properties().size);
            cv::rectangle(
                canvas,
                p1,
                p2,
                cv::Scalar(obstacle->properties().color.x(), obstacle->properties().color.y(), obstacle->properties().color.z()),
                cv::FILLED, 0
                );
        }
    }
}
