#include "Grid.h"
#include "Dijkstra.h"
#include "AStar.h"
#include <iostream>
#include <random>
#include <string>
#include <cstdlib>
#include <functional>


using Algo = std::function<PathResult(const Grid&, std::pair<int, int>, std::pair<int, int>)>;


static int passed = 0, failed = 0;


static void check(bool cond, const std::string& name) {
   if (cond) {
       ++passed;
       std::cout << "[PASS] " << name << "\n";
   } else {
       ++failed;
       std::cout << "[FAIL] " << name << "\n";
   }
}


static bool validPath(const Grid& g, const PathResult& r, std::pair<int, int> s, std::pair<int, int> e) {
   if (!r.found) return r.path.empty();
   if (r.path.empty() || r.path.front() != s || r.path.back() != e) return false;
   int cost = 0;
   for (size_t i = 0; i < r.path.size(); ++i) {
       auto [x, y] = r.path[i];
       if (!g.inBounds(x, y) || g.at(x, y).isBlocked) return false;
       if (i > 0) {
           auto [px, py] = r.path[i - 1];
           if (std::abs(x - px) + std::abs(y - py) != 1) return false;
           cost += g.at(x, y).movementCost;
       }
   }
   return cost == r.totalCost;
}


static Grid randomGrid(int w, int h, double blockedRatio, int maxCost, std::mt19937& rng) {
   Grid g(w, h);
   std::uniform_real_distribution<double> p(0.0, 1.0);
   std::uniform_int_distribution<int> c(1, maxCost);
   for (int y = 0; y < h; ++y)
       for (int x = 0; x < w; ++x) {
           g.at(x, y).isBlocked = p(rng) < blockedRatio;
           g.at(x, y).movementCost = c(rng);
       }
   return g;
}


static void runUnitTests(const std::string& label, Algo algo) {
   {
       Grid g(5, 5);
       auto r = algo(g, {0, 0}, {4, 4});
       check(r.found && r.totalCost == 8 && r.path.size() == 9 && validPath(g, r, {0, 0}, {4, 4}),
             label + ": open grid shortest path");
   }
   {
       Grid g(5, 5);
       for (int y = 0; y < 4; ++y) g.at(2, y).isBlocked = true;
       auto r = algo(g, {0, 0}, {4, 0});
       check(r.found && r.totalCost == 12 && validPath(g, r, {0, 0}, {4, 0}),
             label + ": detour around wall");
   }
   {
       Grid g(5, 5);
       for (int y = 0; y < 5; ++y) g.at(2, y).isBlocked = true;
       auto r = algo(g, {0, 0}, {4, 4});
       check(!r.found && r.path.empty(), label + ": no path through full wall");
   }
   {
       Grid g(5, 5);
       auto r = algo(g, {2, 2}, {2, 2});
       check(r.found && r.totalCost == 0 && r.path.size() == 1, label + ": start equals end");
   }
   {
       Grid g(5, 5);
       g.at(0, 0).isBlocked = true;
       auto r1 = algo(g, {0, 0}, {4, 4});
       auto r2 = algo(g, {4, 4}, {0, 0});
       check(!r1.found && !r2.found, label + ": blocked start or end");
   }
   {
       Grid g(5, 5);
       auto r = algo(g, {-1, 0}, {10, 10});
       check(!r.found, label + ": out-of-bounds input");
   }
   {
       Grid g(3, 3);
       g.at(1, 0).movementCost = 10;
       g.at(2, 0).movementCost = 10;
       auto r = algo(g, {0, 0}, {2, 0});
       check(r.found && r.totalCost == 13 && validPath(g, r, {0, 0}, {2, 0}),
             label + ": avoids expensive cells");
   }
}


static void runComparisonTests() {
   std::mt19937 rng(42);
   int mismatches = 0, invalid = 0;
   const int trials = 300;
   for (int t = 0; t < trials; ++t) {
       Grid g = randomGrid(30, 20, 0.25, 5, rng);
       std::uniform_int_distribution<int> rx(0, 29), ry(0, 19);
       std::pair<int, int> s{rx(rng), ry(rng)}, e{rx(rng), ry(rng)};
       auto d = dijkstra(g, s, e);
       auto a = aStar(g, s, e);
       if (d.found != a.found || (d.found && d.totalCost != a.totalCost)) ++mismatches;
       if (!validPath(g, d, s, e) || !validPath(g, a, s, e)) ++invalid;
   }
   check(mismatches == 0, "Dijkstra and A* agree on cost over 300 random grids");
   check(invalid == 0, "All paths from both algorithms are valid");
}


static void runBenchmark() {
   std::mt19937 rng(7);
   Grid g = randomGrid(400, 300, 0.20, 1, rng);
   std::pair<int, int> s{0, 0}, e{399, 299};
   g.at(s.first, s.second).isBlocked = false;
   g.at(e.first, e.second).isBlocked = false;


   auto d = dijkstra(g, s, e);
   auto a = aStar(g, s, e);


   std::cout << "\nBenchmark: 400 x 300 grid (120,000 vertices), 20% obstacles, (0,0) -> (399,299)\n";
   std::cout << "Algorithm  Found  Time(ms)  Explored  PathLength  Cost\n";
   std::cout << "Dijkstra   " << d.found << "      " << d.timeMs << "  " << d.nodesExplored
             << "     " << d.path.size() << "         " << d.totalCost << "\n";
   std::cout << "A*         " << a.found << "      " << a.timeMs << "  " << a.nodesExplored
             << "     " << a.path.size() << "         " << a.totalCost << "\n\n";


   check(d.found == a.found && d.totalCost == a.totalCost, "Benchmark: same optimal cost");
   check(validPath(g, d, s, e) && validPath(g, a, s, e), "Benchmark: both paths valid");
}


int main() {
   runUnitTests("Dijkstra", dijkstra);
   runUnitTests("A*", aStar);
   runComparisonTests();
   runBenchmark();
   std::cout << passed << " passed, " << failed << " failed\n";
   return failed == 0 ? 0 : 1;
}

