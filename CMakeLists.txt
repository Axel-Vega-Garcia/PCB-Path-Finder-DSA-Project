cmake_minimum_required(VERSION 3.20)
project(PCB_Path_Finder)

set(CMAKE_CXX_STANDARD 14)

add_executable(PCB_Path_Finder main.cpp AStar.cpp AStar.h Grid.h Dijkstra.cpp Dijkstra.h)