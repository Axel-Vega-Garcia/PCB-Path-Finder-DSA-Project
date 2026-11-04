#include "Dijkstra.h"
#include <queue>
#include <limits>
#include <chrono>
#include <algorithm>
#include <functional>


PathResult dijkstra(const Grid& grid, std::pair<int, int> start, std::pair<int, int> end) {
   auto t0 = std::chrono::high_resolution_clock::now();
   PathResult result;


   auto finish = [&]() {
       auto t1 = std::chrono::high_resolution_clock::now();
       result.timeMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
       return result;
   };


   if (!grid.inBounds(start.first, start.second) || !grid.inBounds(end.first, end.second))
       return finish();
   if (grid.at(start.first, start.second).isBlocked || grid.at(end.first, end.second).isBlocked)
       return finish();


   const int W = grid.width();
   const int N = W * grid.height();
   const int INF = std::numeric_limits<int>::max();
   auto id = [W](int x, int y) { return y * W + x; };


   std::vector<int> dist(N, INF);
   std::vector<int> prev(N, -1);
   std::vector<char> visited(N, 0);


   using Entry = std::pair<int, int>;
   std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;


   int s = id(start.first, start.second);
   int e = id(end.first, end.second);
   dist[s] = 0;
   pq.push({0, s});


   while (!pq.empty()) {
       auto [d, u] = pq.top();
       pq.pop();
       if (visited[u]) continue;
       visited[u] = 1;
       ++result.nodesExplored;
       if (u == e) break;


       int ux = u % W, uy = u / W;
       for (auto [nx, ny] : grid.neighbors(ux, uy)) {
           int v = id(nx, ny);
           if (visited[v]) continue;
           int nd = d + grid.at(nx, ny).movementCost;
           if (nd < dist[v]) {
               dist[v] = nd;
               prev[v] = u;
               pq.push({nd, v});
           }
       }
   }


   if (dist[e] != INF) {
       result.found = true;
       result.totalCost = dist[e];
       for (int cur = e; cur != -1; cur = prev[cur])
           result.path.push_back({cur % W, cur / W});
       std::reverse(result.path.begin(), result.path.end());
   }


   return finish();
}

