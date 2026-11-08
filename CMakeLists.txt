cmake_minimum_required(VERSION 3.20)

project(PCB_Path_Finder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PCB_Path_Finder
    main.cpp
    Grid.h
    AStar.cpp
    AStar.h
    Dijkstra.cpp
    Dijkstra.h
)