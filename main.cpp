#include "Grid.h"
#include "Dijkstra.h"
#include "AStar.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace {
struct EndOfInput {};
template <typename... Values>
void readInput(const std::string& prompt, Values&... values) {
    while (true) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) throw EndOfInput{};
        std::istringstream input(line);
        if ((input >> ... >> values) && (input >> std::ws).eof()) return;
        std::cout << "invalid input.\n";
    }
}

void displayBoard(const Grid& grid, Position start, Position end, const PathResult& result) {
    std::string picture;
    picture.reserve(grid.size() + grid.height());
    for (int y = 0; y < grid.height(); ++y) {
        for (int x = 0; x < grid.width(); ++x) {
            const Cell& cell = grid.at(x, y);
            picture += cell.isBlocked ? '#' : (cell.movementCost > 1 ? '+' : '.');
        }
        picture += '\n';
    }
    auto mark = [&](Position p, char symbol) {
        grid.at(p.first, p.second); // Check bounds before indexing.
        picture[static_cast<std::size_t>(p.second) * (grid.width() + 1) + p.first] = symbol;
    };
    if (result.found)
        for (Position p : result.path) mark(p, '*');
    mark(start, 'S');
    mark(end, start == end ? 'B' : 'E');

    const auto blocked = grid.blockedCount();
    std::cout << "\nBoard: " << grid.width() << " x " << grid.height()
              << "\nTotal positions: " << grid.size()
              << "\nAvailable: " << grid.size() - blocked << " | Blocked: " << blocked
              << "\nStart: (" << start.first << ", " << start.second << ")"
              << " | End: (" << end.first << ", " << end.second << ")"
              << "\nFull board:\n" << picture
              << ". open  # obstacle  + weighted cell  * path  S start  E end  B both\n";
}

void validateResult(const Grid& grid, Position start, Position end, const PathResult& result) {

    if (!result.found && result.path.empty()) return;
    if (!result.found || result.path.empty() || result.path.front() != start || result.path.back() != end)
        throw std::runtime_error("Returned path has incorrect status or endpoints.");

    long long cost = 0;
    for (std::size_t i = 0; i < result.path.size(); ++i) {

        auto [x, y] = result.path[i];

        if (!grid.inBounds(x, y) || grid.at(x, y).isBlocked) throw std::runtime_error("Returned path enters an invalid or blocked cell.");
        if (i == 0) continue;

        auto [px, py] = result.path[i - 1];

        if (std::abs(x - px) + std::abs(y - py) != 1) throw std::runtime_error("Returned path contains an invalid move.");
        
        cost += grid.at(x, y).movementCost;
    }
    if (cost != result.totalCost) throw std::runtime_error("Returned total cost does not match the path.");
}

PathResult runSearch(const Grid& grid, Position start, Position end, bool useAStar) {

    PathResult result = (useAStar ? aStar : dijkstra)(grid, start, end);
    validateResult(grid, start, end, result);
    
    std::cout << '\n' << (useAStar ? "A*" : "Dijkstra")
              << (result.found ? ": path found\n" : ": no path found\n");

    if (result.found)
        std::cout << "Path length: " << result.path.size() - 1 << " moves\n"
                  << "Total movement cost: " << result.totalCost << '\n';
                  
    std::cout << "Blocks explored: " << result.nodesExplored << '\n'
              << "Execution time: " << std::fixed << std::setprecision(3) << result.timeMs << " ms\n";

    return result;
}

} // namespace

int main() {
    try {
        Grid grid(40, 30); //grid size
        Position start{0, 0}, end{grid.width() - 1, grid.height() - 1};
        PathResult lastResult;
        grid.generateObstacles(0.15, 42, start, end);
        displayBoard(grid, start, end, lastResult);

        while (true) {
            int choice;
            std::cout << R"(
1 Display board
2 Generate obstacles
3 Set start and end
4 Toggle obstacle
5 Set movement cost
6 Run Dijkstra
7 Run A*
8 Compare both algorithms
9 Change board size
0 Exit
)";
            readInput("Choice: ", choice);
            try {
                if (choice == 0) return 0;
                if (choice < 0 || choice > 9) {

                    std::cout << "Choose a number from 0 through 9.\n";
                    continue;
                }
                if (choice == 2) { // Generate obstacles using user-defined percentage and seed

                    double percentage;
                    long long seed;
                    readInput("Obstacle percentage and integer seed ex: 15 42 (15\% obstacles and seed 42): ", percentage, seed);
                    
                    if (seed < 0 || seed > std::numeric_limits<std::uint32_t>::max())
                        throw std::invalid_argument("Seed must be between 0 and 4294967295.");
                    
                    grid.generateObstacles(percentage / 100.0, static_cast<std::uint32_t>(seed), start, end);

                } else if (choice == 3) { // Set new start and end positions

                    Position newStart, newEnd;
                    readInput("Start x y and end x y: ", newStart.first, newStart.second, newEnd.first, newEnd.second);
                    if (!grid.inBounds(newStart.first, newStart.second) || !grid.inBounds(newEnd.first, newEnd.second))
                        throw std::out_of_range("Endpoints must be inside the board.");

                    grid.setBlocked(newStart.first, newStart.second, false);
                    grid.setBlocked(newEnd.first, newEnd.second, false);

                    start = newStart;
                    end = newEnd;

                    std::cout << "Endpoints updated and cleared.\n";

                } else if (choice == 4) { // Toggle obstacle state for a specific cell

                    int x, y;
                    readInput("Cell x y: ", x, y);

                    if (Position{x, y} == start || Position{x, y} == end) throw std::invalid_argument("Cannot block the start or end.");
                    
                    Cell& cell = grid.at(x, y);
                    cell.isBlocked = !cell.isBlocked;

                    std::cout << "Obstacle updated.\n";

                } else if (choice == 5) {// Set movement cost for a specific cell

                    int x, y, cost;

                    readInput("Cell x y and movement cost [1..100]: ", x, y, cost);
                    grid.setMovementCost(x, y, cost);

                    std::cout << "Movement cost updated.\n";

                } else if (choice >= 6 && choice <= 8) {// Run Dijkstra, A*, or compare both algorithms

                    PathResult result = runSearch(grid, start, end, choice == 7);

                    if (choice == 8) {

                        PathResult other = runSearch(grid, start, end, true);

                        if (result.found != other.found)
                            std::cout << "algorithms disagree about whether a path exists.\n";

                        else if (result.found && result.totalCost != other.totalCost)
                            std::cout << "algorithms returned different path costs.\n";

                        result = std::move(other);

                    }
                    lastResult = std::move(result);

                } else if (choice == 9) {// Change board size and reset obstacles

                    int width, height;
                    char separator;
                    readInput("New board size (columns x rows, ex: 40x30): ", width, separator, height);

                    if (separator != 'x' && separator != 'X')
                        throw std::invalid_argument("Use x between the dimensions, ex: 40x30.");

                    Grid newGrid(width, height);
                    Position newEnd{width - 1, height - 1};
                    newGrid.generateObstacles(0.15, 42, {0, 0}, newEnd);
                    grid = std::move(newGrid);

                    start = {0, 0};
                    end = newEnd;
                    std::cout << "Board resized. Obstacles reset to 15% with seed 42.\n";
                }
                // Clear the old path after a successful edit, then display if requested.
                if ((choice >= 2 && choice <= 5) || choice == 9) lastResult = {};

                if (choice == 1 || choice == 2 || choice >= 6) displayBoard(grid, start, end, lastResult);

            } catch (const std::exception& error) { std::cout << "Error: " << error.what() << '\n';}
        }
    } catch (const EndOfInput&) { std::cout << "\nInput closed.\n";

    } catch (const std::exception& error) { 

        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}