#pragma once
#include "Grid.h"
#include <utility>
#include <cmath>

PathResult aStar(const Grid& grid, std::pair<int, int> start, std::pair<int, int> end);

// Heuristic: Manhattan distance for 4-directional movement
inline int calculateHeuristic(int x1, int y1, int x2, int y2) {
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}