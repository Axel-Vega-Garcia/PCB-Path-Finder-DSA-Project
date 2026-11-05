#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

using Position = std::pair<int, int>;

struct Cell {
    int x, y;
    bool isBlocked = false;
    int movementCost = 1;
};

struct PathResult {
    bool found = false;
    std::vector<Position> path;
    int totalCost = 0;
    long long nodesExplored = 0;
    double timeMs = 0.0;
};

class Grid {
    int w, h;
    std::vector<Cell> cells;

    std::size_t index(int x, int y) const {
        if (!inBounds(x, y)) throw std::out_of_range("cell is outside the board.");
        return static_cast<std::size_t>(y) * w + x;
    }

public:
    static constexpr int MAX_CELLS = 4'000'000;
    static constexpr int MAX_MOVEMENT_COST = 100;

    Grid(int width, int height) : w(width), h(height) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("dimensions must be positive.");
        if (w > MAX_CELLS / h) throw std::invalid_argument("grid is too large.");
        cells.resize(static_cast<std::size_t>(w) * h);
        for (int i = 0; i < w * h; ++i) cells[i] = {i % w, i / w};
    }

    int width() const { return w; }
    int height() const { return h; }
    std::size_t size() const { return cells.size(); }
    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < w && y < h; }
    Cell& at(int x, int y) { return cells[index(x, y)]; }
    const Cell& at(int x, int y) const { return cells[index(x, y)]; }
    void setBlocked(int x, int y, bool blocked) { at(x, y).isBlocked = blocked; }

    void setMovementCost(int x, int y, int cost) {
        if (cost < 1 || cost > MAX_MOVEMENT_COST)
            throw std::invalid_argument("cost must be between 1 and 100.");
        at(x, y).movementCost = cost;
    }

    std::size_t blockedCount() const {return std::count_if(cells.begin(), cells.end(),[](const Cell& cell) { return cell.isBlocked; });}

    std::vector<Position> neighbors(int x, int y) const {
        if (!inBounds(x, y) || at(x, y).isBlocked) return {};
        const Position adjacent[] = {{x + 1, y}, {x - 1, y}, {x, y + 1}, {x, y - 1}};
        std::vector<Position> result;
        result.reserve(4);
        for (auto [nx, ny] : adjacent)
            if (inBounds(nx, ny) && !at(nx, ny).isBlocked) result.push_back({nx, ny});
        return result;
    }

    void generateObstacles(double density, std::uint32_t seed, Position start, Position end) {
        if (!std::isfinite(density) || density < 0 || density > 1)
            throw std::invalid_argument("must be between 0 and 100");
        if (!inBounds(start.first, start.second) || !inBounds(end.first, end.second))
            throw std::out_of_range("must be inside the board");

        std::mt19937 generator(seed); //random numgen with fixed seed
        const auto threshold = static_cast<std::uint64_t>(density * 4294967296.0);
        for (Cell& cell : cells)
            cell.isBlocked = generator() < threshold;
        setBlocked(start.first, start.second, false);
        setBlocked(end.first, end.second, false);
    }
};