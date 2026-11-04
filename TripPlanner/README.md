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

All core algorithms are strictly implemented in modular C files. None are replaced by JavaScript libraries:

| Module | Data Structure | Algorithm | Time Complexity | Space Complexity | Source File |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1. Location & Route** | Graph (Adjacency List) + Min-Heap | **Dijkstra's Shortest Path** with unreachable destination detection | $O((V + E) \log V)$ | $O(V + E)$ | `backend/location_route.c` |
| **2. Itinerary Optimization** | Distance / Duration Matrix | **Greedy Nearest Neighbor** (Fast heuristic) | $O(N^2)$ | $O(N)$ | `backend/itinerary.c` |
| **2. Itinerary Optimization** | Recursive Call Tree | **Exact TSP (Backtracking)** with branch-and-bound pruning | $O(N!)$ worst-case | $O(N)$ stack depth | `backend/itinerary.c` |
| **2. Itinerary Scheduling** | Sequential Timeline | **Opening/Closing Hours & Active Limits** (9 hrs/day limit) | $O(N)$ | $O(N)$ | `backend/itinerary.c` |
| **3. Budget & Lodging** | Min-Heap Priority Queue | **Cheapest Accommodations Extraction** | $O(K \log N)$ | $O(N)$ | `backend/budget.c` |
| **3. Multi-Criteria Sort** | In-place Array | **QuickSort** (by Price, Rating, Distance) | $O(N \log N)$ avg | $O(\log N)$ stack | `backend/budget.c` |
| **4. Instant Search** | Hash Table with Separate Chaining | **djb2 Polynomial Hash Function** ($hash \times 33 + c$) | $O(1)$ average | $O(M)$ entries | `backend/search.c` |
| **5. Trip Management** | AVL Tree (Self-Balancing BST) | **Rotations (LL, RR, LR, RL)** for Insert, Search, Delete, Update | $O(\log N)$ | $O(N)$ | `backend/trip_management.c` |

---

## Project Structure

```
TripPlanner/
├── backend/
│   ├── data.h / data.c                     # Master dataset (13 destinations, 28 roads, 37 attractions, 36 hotels)
│   ├── location_route.h / location_route.c # Adjacency List Graph, Min-Heap, Dijkstra Shortest Path
│   ├── itinerary.h / itinerary.c           # Greedy Nearest-Neighbor, Exact TSP (Backtracking), Day Scheduler
│   ├── budget.h / budget.c                 # Budget breakdown, Hotel Min-Heap, QuickSort sorting
│   ├── search.h / search.c                 # Hash Table with Separate Chaining & djb2 hashing
│   ├── trip_management.h / trip_management.c # Self-Balancing AVL Tree (CRUD & file persistence)
│   ├── main.c                              # CLI interactive menu & JSON API dispatcher
│   └── trip_planner.exe                    # Compiled C executable
├── frontend/
│   ├── index.html                          # Semantic HTML5 dashboard layout
│   ├── style.css                           # Modern, responsive styling with clean design system
│   └── script.js                           # State controller, API integrations & UI renderers
├── server/
│   ├── server.js                           # Express.js REST API bridging Node.js and C backend
│   └── package.json                        # Node dependencies (express, cors)
├── data/
│   └── saved_trips.txt                     # Persistent AVL tree trip records
└── README.md                               # Project documentation & compilation instructions
```

---

## Compilation & Execution Guide

### Prerequisites
- **GCC Compiler** (MinGW-w64 on Windows or `gcc` on Linux/macOS)
- **Node.js** (v18+) & **npm**

### 1. Compile the C DSA Backend
From the project root:

```bash
gcc -Wall -Wextra -O2 -I"TripPlanner/backend" \
    TripPlanner/backend/data.c \
    TripPlanner/backend/location_route.c \
    TripPlanner/backend/itinerary.c \
    TripPlanner/backend/budget.c \
    TripPlanner/backend/search.c \
    TripPlanner/backend/trip_management.c \
    TripPlanner/backend/main.c \
    -o TripPlanner/backend/trip_planner.exe -lm
```

### 2. Run C Backend Standalone Tests or Interactive Menu

**Run Automated Unit Tests:**
```bash
./TripPlanner/backend/trip_planner.exe --api run_tests
```

**Run Interactive Terminal Menu:**
```bash
./TripPlanner/backend/trip_planner.exe
```

### 3. Launch the Web Application Server

```bash
cd TripPlanner/server
npm install
node server.js
```

Open your browser at:
```
http://localhost:3000
```

---

## API Endpoints Reference

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/api/health` | Service health status |
| `GET` | `/api/locations` | List all graph locations and coordinates |
| `GET` | `/api/attractions` | List all attractions |
| `GET` | `/api/hotels` | List all hotels |
| `POST` | `/api/route` | Compute Dijkstra shortest path between `srcId` and `destId` |
| `POST` | `/api/itinerary` | Benchmark Greedy vs Exact TSP and day-wise schedule |
| `POST` | `/api/budget` | Itemized budget calculation with hotel Min-Heap extraction |
| `GET` | `/api/search` | Query attractions from C Hash Table (`?query=...&category=...`) |
| `POST` | `/api/plan-trip` | Full unified trip planning (Route + TSP + Budget + Hotels) |
| `GET` | `/api/trips` | Chronological list of trips via AVL Tree in-order traversal |
| `POST` | `/api/trips` | Insert new trip record into AVL Tree with automatic rebalancing |
| `GET` | `/api/trips/:id` | Search trip by ID in AVL Tree ($O(\log N)$) |
| `PUT` | `/api/trips/:id` | Update trip record in AVL Tree |
| `DELETE` | `/api/trips/:id` | Delete trip record from AVL Tree and perform rotation balance |
| `GET` | `/api/tests` | Execute C unit test suite and return live terminal report |

---

## Key Features Overview

1. **Location & Route Management**:
   - Computes minimum distance and driving duration between any two cities using Dijkstra's algorithm.
   - Detects unreachable vertices (e.g., Port Blair island) and displays informative error alerts.
   - Outlines road interchanges, transit durations, and highway tolls.

2. **Itinerary Optimization**:
   - Simultaneously calculates **Greedy Nearest Neighbor** ($O(N^2)$) and **Exact TSP** ($O(N!)$) using backtracking.
   - Benchmarks both solutions side-by-side with execution time in microseconds ($\mu s$).
   - Generates day-by-day schedules accounting for opening times, closing times, visit durations, and daily active caps.

3. **Budget & Smart Recommendations**:
   - Itemizes lodging, food, transport, activity tickets, and 7% emergency contingency.
   - Computes remaining balance and evaluates financial health ("Within Budget", "Near Limit", "Over Budget").
   - Min-Heap priority queue automatically extracts cheaper hotel alternatives and calculates exact potential savings.

4. **Hash Table Search**:
   - Indexed using the djb2 rolling polynomial hash algorithm into 127 buckets with separate chaining.
   - Instant search across attraction names, keyword tokens, and categories (Historical, Nature, Cultural, Adventure, Religious, Food).

5. **Trip Management (AVL Tree)**:
   - Self-balancing binary search tree maintains logarithmic search, insert, and deletion times ($O(\log N)$).
   - In-order traversal displays saved trips in chronological order by departure date.
   - Automatically synchronizes with disk storage (`data/saved_trips.txt`).

