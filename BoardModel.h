#pragma once
#include "Grid.h"
#include "AStar.h"
#include "Dijkstra.h"

#include <algorithm>
#include <string>

enum class Tool { Start, End, Block, Erase, Weight, Chip };

struct Component {
    int x, y, w, h;
    std::string name;
    bool contains(Position p) const {return p.first >= x && p.second >= y && p.first < x + w && p.second < y + h;}
};

class BoardModel {
    int nextChip = 1;

    bool editable(Position p) const {return p != start && p != end && boardCell(p);}
    bool reject(const char* message) {status = message; return false;}
    bool edited(std::string message) {
        hasDijkstra = hasAStar = false;
        dijkstraResult = astarResult = {};
        status = std::move(message);
        return true;
    }

    void blockComponent(const Component& part, bool blocked) {
        for (int y = part.y; y < part.y + part.h; ++y)
            for (int x = part.x; x < part.x + part.w; ++x)
                if (editable({x, y})) grid.setBlocked(x, y, blocked);
    }

public:
    Grid grid{400, 300};
    Position start{24, 150}, end{372, 75};
    std::vector<Component> components;
    PathResult dijkstraResult, astarResult;
    bool hasDijkstra = false, hasAStar = false;
    std::string status = "Choose a start and end.";

    BoardModel() { preset(); }
    bool boardCell(Position p) const {
        auto [x, y] = p;
        if (!grid.inBounds(x, y)) return false;
        const int w = grid.width(), h = grid.height();
        if (std::min(w, h) < 60) return true;
        const int r = std::max(3, std::min(w, h) / 35);
        const int cx = std::clamp(x, r, w - 1 - r);
        const int cy = std::clamp(y, r, h - 1 - r);
        return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r;
    }
    void applyOutline() {
        for (int y = 0; y < grid.height(); ++y)
            for (int x = 0; x < grid.width(); ++x)
                if (!boardCell({x, y})) grid.setBlocked(x, y, true);
    }
    void preset() {
        clear(true);
        nextChip = 1;
        if (grid.width() >= 20 && grid.height() >= 20) {
            const int layout[][4] = {
                {19, 25, 15, 25},
                {45, 44, 18, 18},
                {70, 18, 12, 18},
                {24, 73, 12, 11},
                {73, 72, 8, 10},
                {48, 18, 5, 10}
            };
            for (const auto& box : layout) {
                Component part{
                    grid.width() * box[0] / 100,
                    grid.height() * box[1] / 100,
                    std::max(2, grid.width() * box[2] / 100),
                    std::max(2, grid.height() * box[3] / 100),
                    "U" + std::to_string(nextChip++)
                };
                blockComponent(part, true);
                components.push_back(part);
            }
        }
        status = "PCB layout ready.";
    }

    void resize(int width, int height) {
        if (width < 1 || height < 1 || width > 2000 || height > 2000) throw std::invalid_argument("Use 1 to 2000 columns and rows.");
        grid = Grid(width, height);
        preset();
    }
    void clear(bool resetEndpoints = false) {
        grid = Grid(grid.width(), grid.height());
        components.clear();

        if (resetEndpoints) {
            start = {grid.width() * 6 / 100, grid.height() / 2};
            end = {grid.width() * 93 / 100, grid.height() / 4};
        }

        applyOutline();
        for (auto p : {start, end}) grid.setBlocked(p.first, p.second, false);
        edited("Board cleared.");
    }
    void randomize(double percentage, std::uint32_t seed) {
        grid.generateObstacles(percentage / 100.0, seed, start, end);
        components.clear();
        applyOutline();
        edited("Obstacles generated with seed " + std::to_string(seed) + ".");
    }
    bool select(Tool tool, Position p) {
        if (!boardCell(p) || grid.at(p.first, p.second).isBlocked) return reject("Choose an open cell or pad.");
        (tool == Tool::Start ? start : end) = p;
        return edited("Endpoint updated.");
    }
    bool chip(Position p, int w, int h) {
        if (w < 1 || h < 1 || w > grid.width() || h > grid.height()) return reject("Chip dimensions must be positive and fit the board.");
        auto existing = std::find_if(components.begin(), components.end(),[&](const Component& part) {return part.contains(p);});
        const bool resizing = existing != components.end();
        if (resizing) p = {existing->x + existing->w / 2, existing->y + existing->h / 2};
        Component part{
            std::clamp(p.first - w / 2, 0, grid.width() - w),
            std::clamp(p.second - h / 2, 0, grid.height() - h),
            w, h, resizing ? existing->name : "U" + std::to_string(nextChip)
        };
        for (int y = part.y; y < part.y + h; ++y)
            for (int x = part.x; x < part.x + w; ++x)
                if (!editable({x, y}) || (grid.at(x, y).isBlocked && !(resizing && existing->contains({x, y}))))
                    return reject("Place the chip on a clear area.");
        if (resizing) {
            blockComponent(*existing, false);
            *existing = part;
        } else {
            ++nextChip;
            components.push_back(part);
        }
        blockComponent(part, true);
        return edited(resizing ? "Chip resized." : "Chip placed.");
    }
    bool brush(Tool tool, Position p, int radius, int cost) {
        if (!boardCell(p)) return false;
        bool changed = false;
        if (tool == Tool::Erase) {
            components.erase(std::remove_if(components.begin(), components.end(), [&](const Component& part) {
                const bool overlap = p.first + radius >= part.x && p.first - radius < part.x + part.w && p.second + radius >= part.y && p.second - radius < part.y + part.h;
                if (overlap) {
                    blockComponent(part, false);
                    changed = true;
                } return overlap;}),components.end()
            );
        }
        for (int y = p.second - radius; y <= p.second + radius; ++y)
            for (int x = p.first - radius; x <= p.first + radius; ++x) {
                if (!editable({x, y})) continue;
                Cell& cell = grid.at(x, y);
                if (tool == Tool::Weight && cell.isBlocked) continue;
                const bool blocked = tool == Tool::Block;
                const int value = tool == Tool::Weight ? cost : blocked ? cell.movementCost : 1;

                if (cell.isBlocked != blocked || cell.movementCost != value) {
                    if (cell.movementCost != value) grid.setMovementCost(x, y, value);
                    cell.isBlocked = blocked;
                    changed = true;
                }
            }
        return changed && edited("Board edited.");
    }
    bool valid(const PathResult& result) const {
        if (!result.found) return result.path.empty();
        if (result.path.empty() || result.path.front() != start || result.path.back() != end) return false;
        long long cost = 0;

        for (std::size_t i = 0; i < result.path.size(); ++i) {
            auto [x, y] = result.path[i];
            if (!grid.inBounds(x, y) || grid.at(x, y).isBlocked) return false;
            if (i) {
                auto [px, py] = result.path[i - 1];
                if (std::abs(x - px) + std::abs(y - py) != 1) return false;
                cost += grid.at(x, y).movementCost;
            }
        }
        return cost == result.totalCost;
    }
    void run(int algorithms) {
        PathResult d, a;
        if (algorithms & 1) d = dijkstra(grid, start, end);
        if (algorithms & 2) a = aStar(grid, start, end);
        if (((algorithms & 1) && !valid(d)) || ((algorithms & 2) && !valid(a))) throw std::runtime_error("Algorithm returned invalid route.");

        dijkstraResult = std::move(d);
        astarResult = std::move(a);
        hasDijkstra = algorithms & 1;
        hasAStar = algorithms & 2;

        if (algorithms == 3 && (dijkstraResult.found != astarResult.found || (astarResult.found && dijkstraResult.totalCost != astarResult.totalCost)))
            status = "Algorithms disagree.";
        else if ((hasDijkstra && dijkstraResult.found) || (hasAStar && astarResult.found))
            status = algorithms == 3 ? "Both algorithms found the same minimum cost." : "Route found.";
        else status = "No route exists.";
    }
};