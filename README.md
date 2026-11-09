# PCB Path Finder

## Team PCB:  Axel Vega-Garcia, Savannah Baker, Minh Hoang

Our project finds the shortest path between two points on a PCB grid while avoiding obstacles. It compares Dijkstra's algorithm and A* to see which one is faster.

## What you need

- C++17 compiler
- CMake 3.20+
- Internet the first time you build (CMake downloads SFML for you)

## How to build

You can use any IDE that supports CMake (CLion, Visual Studio, VS Code, etc). Just open the project folder and it should load CMakeLists.txt on its own.

Or from the terminal:

```
cmake -S . -B build
cmake --build build
```

This builds 3 programs:
- PCB_Visual - the SFML version with the graphical interface
- PCB_Path_Finder - the terminal version
- PCBTests - tests for both algorithms

## How to run

### Visual version

```
./build/PCB_Visual
```

Pick a tool on the left (set start, set end, paint obstacles, erase, paint cost, place chip), then click Run Dijkstra, Run A*, or Compare both. The results show up on the right.

Controls: scroll to zoom, left drag to move around, left click to use a tool, right drag to paint.

You can also type commands and hit Enter, like `size 40x30`, `chip 20x20`, `random 15 42`, `cost 5`, or `brush 3`.

Note: right now the font is loaded from C:/Windows/Fonts/consola.ttf, so the visual version only works on Windows. Use the terminal version on Mac/Linux.

### Terminal version

```
./build/PCB_Path_Finder
```

Use the menu to generate obstacles, set the start and end, add obstacles or costs, and run Dijkstra, A*, or both. You can change the board size too (try 400x300).

Board symbols: `.` open, `#` obstacle, `+` higher cost, `*` path, `S` start, `E` end

### Tests

```
./build/PCBTests
```

Runs 18 tests on both algorithms. It should say `18 passed, 0 failed` at the end.

On Windows the programs are in `build\Debug\` instead.

## Files

- Grid.h - the grid and obstacle generation
- Dijkstra.h / Dijkstra.cpp - Dijkstra's algorithm
- AStar.h / AStar.cpp - A*
- main.cpp - terminal version
- visual_main.cpp, PCBView.h/.cpp, BoardModel.h - SFML version
- Tests.cpp - tests
