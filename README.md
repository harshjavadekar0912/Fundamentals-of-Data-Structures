# TripPlanner - Full-Stack Travel Itinerary & Budget Optimization System

A complete **Trip Planner Web Application built from scratch**, powered by a high-performance **C Core Backend** implementing fundamental Data Structures and Algorithms (DSA), coupled with a **Node.js Express REST API**, and a modern, responsive **Frontend Dashboard** (HTML5, CSS3, ES6 JavaScript).

---

## Architecture & Technology Stack

```
+-------------------------------------------------------+
|               Modern Web Frontend UI                  |
|         (HTML5, Responsive CSS3, JavaScript ES6)      |
+---------------------------+---------------------------+
                            |
                     HTTP JSON REST
                            |
+---------------------------v---------------------------+
|              Node.js + Express API Server             |
|                  (TripPlanner/server)                 |
+---------------------------+---------------------------+
                            |
                Process IPC / JSON Streams
                            |
+---------------------------v---------------------------+
|              Modular C DSA Backend Engine             |
|                 (TripPlanner/backend)                 |
|  - Graph Adjacency List & Min-Heap (Dijkstra)         |
|  - Greedy Nearest Neighbor vs Exact Backtracking TSP  |
|  - Hotel Min-Heap & QuickSort Multi-Criteria          |
|  - djb2 Separate-Chaining Hash Table Search           |
|  - Self-Balancing AVL Binary Search Tree (Trips CRUD) |
+-------------------------------------------------------+
```

---

## Data Structures & Algorithms Implemented

All core algorithms are strictly implemented in modular C files:

| Module | Data Structure | Algorithm | Time Complexity | Space Complexity | Source File |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Location & Route** | Graph (Adjacency List) + Min-Heap | **Dijkstra's Shortest Path** with unreachable destination detection | $O((V + E) \log V)$ | $O(V + E)$ | `TripPlanner/backend/location_route.c` |
| **2. Itinerary Optimization** | Distance / Duration Matrix | **Greedy Nearest Neighbor** (Fast heuristic) | $O(N^2)$ | $O(N)$ | `TripPlanner/backend/itinerary.c` |
| **2. Itinerary Optimization** | Recursive Call Tree | **Exact TSP (Backtracking)** with branch-and-bound pruning | $O(N!)$ worst-case | $O(N)$ stack depth | `TripPlanner/backend/itinerary.c` |
| **2. Itinerary Scheduling** | Sequential Timeline | **Opening/Closing Hours & Active Limits** (9 hrs/day limit) | $O(N)$ | $O(N)$ | `TripPlanner/backend/itinerary.c` |
| **3. Budget & Lodging** | Min-Heap Priority Queue | **Cheapest Accommodations Extraction** | $O(K \log N)$ | $O(N)$ | `TripPlanner/backend/budget.c` |
| **3. Multi-Criteria Sort** | In-place Array | **QuickSort** (by Price, Rating, Distance) | $O(N \log N)$ avg | $O(\log N)$ stack | `TripPlanner/backend/budget.c` |
| **4. Instant Search** | Hash Table with Separate Chaining | **djb2 Polynomial Hash Function** ($hash \times 33 + c$) | $O(1)$ average | $O(M)$ entries | `TripPlanner/backend/search.c` |
| **5. Trip Management** | AVL Tree (Self-Balancing BST) | **Rotations (LL, RR, LR, RL)** for Insert, Search, Delete, Update | $O(\log N)$ | $O(N)$ | `TripPlanner/backend/trip_management.c` |

---

## Compilation & Execution Guide

### 1. Compile the C DSA Backend
From the repository root:

```bash
gcc -Wall -Wextra -O2 -I"TripPlanner/backend" TripPlanner/backend/data.c TripPlanner/backend/location_route.c TripPlanner/backend/itinerary.c TripPlanner/backend/budget.c TripPlanner/backend/search.c TripPlanner/backend/trip_management.c TripPlanner/backend/main.c -o TripPlanner/backend/trip_planner.exe -lm
```

### 2. Run Standalone Unit Tests
```bash
./TripPlanner/backend/trip_planner.exe --api run_tests
```

### 3. Run Web Server
```bash
cd TripPlanner/server
npm install
node server.js
```

Open your browser at:
```
http://localhost:3000
```

