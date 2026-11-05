#pragma once
#include "Grid.h"

// Performs Dijkstra's shortest path algorithm to find the lowest cost path
PathResult dijkstra(const Grid& grid, std::pair<int, int> start, std::pair<int, int> end);

