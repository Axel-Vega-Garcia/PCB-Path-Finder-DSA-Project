#pragma once
#include <vector>
#include <utility>

// Represents a single point on the PCB Board simulation
struct Cell {
    int x;
    int y;
    bool isBlocked;
    int movementCost;
};

// Return object for benchmarking both Dijkstra and A*
struct PathResult {
    bool found = false;
    std::vector<std::pair<int, int>> path;
    int totalCost = 0;
    long long nodesExplored = 0;
    double timeMs = 0.0;
};

// Uses a flattened 1D array to manage the 2D PCB for memory efficiency
class Grid {
public:
    // Initializes a grid of the specified width and height
    Grid(int width, int height) : w(width), h(height), cells(width * height) {
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                cells[y * w + x] = {x, y, false, 1};
    }

    int width() const { return w; }
    int height() const { return h; }

    // Checks if a given coordinate exists within the grid boundaries
    bool inBounds(int x, int y) const { return x >= 0 && x < w && y >= 0 && y < h; }

    // Access 1D array using row major mapping
    Cell& at(int x, int y) { return cells[y * w + x]; }
    const Cell& at(int x, int y) const { return cells[y * w + x]; }

    // Returns a list of valid, adjacent cells that are unblocked
    std::vector<std::pair<int, int>> neighbors(int x, int y) const {
        static const int dx[4] = {1, -1, 0, 0};
        static const int dy[4] = {0, 0, 1, -1};
        std::vector<std::pair<int, int>> out;
        out.reserve(4);
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i], ny = y + dy[i];
            if (inBounds(nx, ny) && !at(nx, ny).isBlocked)
                out.push_back({nx, ny});
        }
        return out;
    }

private:
    int w, h;
    std::vector<Cell> cells;
};