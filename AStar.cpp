#include "AStar.h"
#include <queue>
#include <chrono>
#include <algorithm>

PathResult aStar(const Grid& grid, std::pair<int, int> start, std::pair<int, int> end) {
    auto t0 = std::chrono::high_resolution_clock::now();
    PathResult result;

    // Boundary and obstacle checks identical to Dijkstra
    if (!grid.inBounds(start.first, start.second) || !grid.inBounds(end.first, end.second))
        return result;
    if (grid.at(start.first, start.second).isBlocked || grid.at(end.first, end.second).isBlocked)
        return result;

    int W = grid.width();
    int N = W * grid.height();
    
    // Convert 2D coordinates to 1D index
    auto id = [W](int x, int y) { return y * W + x; };
    int s = id(start.first, start.second);
    int e = id(end.first, end.second);

    // Cost tracking arrays (gCost) and closed set to prevent re-expanding nodes
    const int INF = 1e9;
    std::vector<int> gCost(N, INF);
    std::vector<int> prev(N, -1);
    std::vector<bool> closed(N, false);
    gCost[s] = 0;

    // Priority Queue sorting by fCost = gCost + hCost
    using Entry = std::pair<int, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;

    pq.push({calculateHeuristic(start.first, start.second, end.first, end.second), s});

    while (!pq.empty()) {
        auto [current_fCost, u] = pq.top();
        pq.pop();

        // Skip if we have already found a shorter path and expanded this node
        if (closed[u]) continue;
        closed[u] = true;

        result.nodesExplored++;

        if (u == e) break; // Destination reached

        int ux = u % W, uy = u / W;

        // Iterate through valid neighbors in Grid.h
        for (auto [nx, ny] : grid.neighbors(ux, uy)) {
            int v = id(nx, ny);
            int tentative_gCost = gCost[u] + grid.at(nx, ny).movementCost;

            if (tentative_gCost < gCost[v]) {
                gCost[v] = tentative_gCost;
                prev[v] = u;
                
                int hCost = calculateHeuristic(nx, ny, end.first, end.second);
                pq.push({gCost[v] + hCost, v});
            }
        }
    }

    // Path reconstruction matching Dijkstra's logic
    if (gCost[e] != INF) {
        result.found = true;
        result.totalCost = gCost[e];
        for (int cur = e; cur != -1; cur = prev[cur]) {
            result.path.push_back({cur % W, cur / W});
        }
        std::reverse(result.path.begin(), result.path.end());
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    
    return result;
}