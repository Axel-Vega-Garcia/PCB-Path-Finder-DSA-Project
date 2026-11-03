cmake_minimum_required(VERSION 3.20)
project(PCB_Path_Finder)

set(CMAKE_CXX_STANDARD 14)

add_executable(PCB_Path_Finder AStar.cpp AStar.h)