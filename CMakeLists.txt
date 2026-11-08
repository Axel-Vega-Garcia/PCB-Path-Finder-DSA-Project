cmake_minimum_required(VERSION 3.20)
project(PCB_Path_Finder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

add_executable(PCB_Path_Finder
        main.cpp
        Grid.h
        AStar.cpp
        AStar.h
        Dijkstra.cpp
        Dijkstra.h
)

add_executable(PCBTests
        Tests.cpp
        Dijkstra.cpp
        AStar.cpp
)

add_executable(PCB_Visual
        visual_main.cpp
        PCBView.cpp
        PCBView.h
        BoardModel.h
        Grid.h
        AStar.cpp
        AStar.h
        Dijkstra.cpp
        Dijkstra.h
)

target_link_libraries(PCB_Visual PRIVATE sfml-graphics sfml-window sfml-system)