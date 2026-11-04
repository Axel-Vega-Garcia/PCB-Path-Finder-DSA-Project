#include "Dijkstra.h"
#include <queue>
#include <limits>
#include <chrono>
#include <algorithm>
#include <functional>


PathResult dijkstra(const Grid& grid, std::pair<int, int> start, std::pair<int, int> end) {
   auto t0 = std::chrono::high_resolution_clock::now();
   PathResult result;

    // Helper lambda function to stop timer and return result of final benchmark
   auto finish = [&]() {
       auto t1 = std::chrono::high_resolution_clock::now();
       result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
       return result;
   };

    // Ensure validity of start/end points and check boundaries and obstacles
   if (!grid.inBounds(start.first, start.second) || !grid.inBounds(end.first, end.second))
       return finish();
   if (grid.at(start.first, start.second).isBlocked || grid.at(end.first, end.second).isBlocked)
       return finish();

    // Initialize dimensions and 1D index mapping for memory efficiency
   const int W = grid.width();
   const int N = W * grid.height();
   const int INF = std::numeric_limits<int>::max();
   auto id = [W](int x, int y) { return y * W + x; };

    // Tracks arrays for shortest distance, path reconstruction, and visited states
   std::vector<int> dist(N, INF);
   std::vector<int> prev(N, -1);
   std::vector<char> visited(N, 0);

    // Min-priority queue to always expand the node with the lowest current distance
   using Entry = std::pair<int, int>;
   std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;


   int s = id(start.first, start.second);
   int e = id(end.first, end.second);
   dist[s] = 0;
   pq.push({0, s});

    // Dijkstra core loop
   while (!pq.empty()) {
       auto [d, u] = pq.top();
       pq.pop();
       // Skips if this node was already processed fully
       if (visited[u]) continue;
       visited[u] = 1;
       ++result.nodesExplored;
       // Exits if the destination is reached, search ends early
       if (u == e) break;


       int ux = u % W, uy = u / W;
       // Iterates through all valid, unblocked neighbors
       for (auto [nx, ny] : grid.neighbors(ux, uy)) {
           int v = id(nx, ny);
           if (visited[v]) continue;
           int nd = d + grid.at(nx, ny).movementCost; // Calculates tentative distance to neighbor
           // Updates arrays and push to queue, if a shorter path is found
           if (nd < dist[v]) {
               dist[v] = nd;
               prev[v] = u;
               pq.push({nd, v});
           }
       }
   }

    // Retrace the steps backwards from the destination to construct the final path
   if (dist[e] != INF) {
       result.found = true;
       result.totalCost = dist[e];
       for (int cur = e; cur != -1; cur = prev[cur])
           result.path.push_back({cur % W, cur / W});
       std::reverse(result.path.begin(), result.path.end());
   }


   return finish();
}

