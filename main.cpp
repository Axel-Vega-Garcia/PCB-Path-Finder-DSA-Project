#include <iostream>
#include "AStar.h"
#include "Grid.h"

int main() {
    // Create a 10x10 simulated PCB grid
    Grid testGrid(10, 10);

    testGrid.at(4, 4).isBlocked = true;
    testGrid.at(4, 5).isBlocked = true;
    testGrid.at(4, 6).isBlocked = true;

    // Define start and end coordinates
    std::pair<int, int> start = {0, 0};
    std::pair<int, int> end = {9, 9};

    std::cout << "Starting A* Search...\n";

    PathResult result = aStar(testGrid, start, end);

    // Print the results
    if (result.found) {
        std::cout << "--- Path Found! ---\n";
        std::cout << "Total Cost (Path Length): " << result.totalCost << "\n";
        std::cout << "Blocks Explored: " << result.nodesExplored << "\n";
        std::cout << "Execution Time: " << result.timeMs << " ms\n";
    } else {
        std::cout << "No valid path found.\n";
    }

    return 0;
}