//
// Created by thego on 11/3/2026.
//

#ifndef PCB_PATH_FINDER_DSA_PROJECT_ASTAR_H
#define PCB_PATH_FINDER_DSA_PROJECT_ASTAR_H

struct Node {
    // Base fields for the project
    int x;
    int y;
    bool isBlocked;
    int movementCost;

    // Specific variables for A* pathfinding
    int gCost; // Cost from start node to this node: g(n)
    int hCost; // Heuristic estimated cost to the destination: h(n)
    Node* parent; // A pointer to a previous node to retrace the final path

    // Helper function to get the total f(n) cost
    int getFCost() const {
        return gCost + hCost;
    }

    // Constructor for initialization
    Node(int x_coord, int y_coord, bool blocked, int cost)
        : x(x_coord), y(y_coord), isBlocked(blocked), movementCost(cost),
          gCost(0), hCost(0), parent(nullptr) {}
};

#endif //PCB_PATH_FINDER_DSA_PROJECT_ASTAR_H