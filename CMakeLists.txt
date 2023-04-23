cmake_minimum_required(VERSION 3.12)
project(OpenCV_Arkanoid CXX)

SET(CMAKE_CXX_STANDARD 20)

find_package(Eigen3 REQUIRED CONFIG)
find_package(OpenCV REQUIRED)

add_executable(cv_game
    src/GameDefs.h
    src/GameSimulation.h
    src/GameSimulation.cpp
    src/GameExtensions.h
    src/IO.h
    src/IO.cpp
    src/main.cpp)

target_link_libraries(cv_game PUBLIC
    ${OpenCV_LIBS}
    Eigen3::Eigen
    )

