# TripPlanner - Smart Travel Itinerary & Budget Planner

An advanced, full-stack **Trip Planner Web Application**, powered by a high-performance **C Core Backend** implementing fundamental Data Structures and Algorithms (DSA), connected via a **Node.js Express REST API**, and presented through a modern, responsive **Frontend Dashboard** featuring a complete **Dark Theme** and full **Indian Rupee (INR ₹)** support.

---

## What's New in this Version

1. **City: Pune & Single City Selection**:
   - Added **Pune, Maharashtra** with 14 realistic attractions (Shaniwar Wada, Aga Khan Palace, Sinhagad Fort, Dagdusheth Temple, Raja Kelkar Museum, FC Road Food Trail, Koregaon Park Cafes, etc.) and hotels across all budget tiers.
   - Simplified user experience: The traveler enters **1 destination city only**; the system plans the comprehensive city exploration itinerary and sightseeing transit circuit.
   - Tripled sample data with **121 realistic attractions** and **117 curated hotels** across 18 major Indian destinations (exceeding 3x of original dataset).

2. **Complete Dark Theme**:
   - Polished dark mode with automatic `localStorage` preference persistence.
   - Smooth light/dark toggle in the top navigation bar.
   - High-contrast, beautiful typography and color balance across all components.

3. **Indian Currency (INR / ₹) Everywhere**:
   - All hotel rates, food allowances, transit expenses, monument entry fees, and budget health checks are in Indian Rupees (`₹`).
   - Standard Indian number formatting (e.g. `₹25,000`, `₹1,50,000`).

4. **Exact TSP Optimization Only**:
   - Itinerary optimization strictly uses the **Exact TSP algorithm** with recursion and backtracking (incorporating branch-and-bound lower bound distance pruning).
   - Removed all Greedy heuristic comparisons and benchmark tables from the user interface.

5. **Clean Travel-Focused User Experience**:
   - All internal DSA implementations (Adjacency List Graph, Min-Heap, Exact TSP, djb2 Hash Table, AVL Tree) work behind the scenes.
   - The user interface is clean, modern, and focused strictly on travel planning without academic jargon.

---

## Technology Stack

- **Backend Core**: Modular C (C11) compiled with GCC.
- **API Server**: Node.js (v18+) & Express.js.
- **Data Exchange**: JSON streams via child process execution.
- **Frontend UI**: HTML5, Responsive CSS3 (Light/Dark themes), Modern JavaScript (ES6+).
- **Persistent Storage**: Pipe-delimited flat file synchronized with an AVL Self-Balancing Tree (`data/saved_trips.txt`).

---

## Core Algorithms Implemented in C

| Module | Data Structure | Algorithm | Complexity | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Route & Transit** | Adjacency List Graph + Min-Heap | **Dijkstra's Algorithm** | $O((V + E) \log V)$ | Computes shortest distance and driving duration between locations. |
| **Itinerary Optimization** | Permutation Tree | **Exact TSP (Backtracking)** | $O(N!)$ worst-case, pruned | Determines the global optimal sequence of attractions to minimize transit. |
| **Itinerary Scheduling** | Sequential Timeline | **Opening/Closing & Active Limits** | $O(N)$ | Enforces attraction opening/closing times, visit durations, and daily 9-hour active travel bounds. |
| **Lodging Recommendations**| Min-Heap Priority Queue | **Extract-Min Cheapest Hotels** | $O(K \log N)$ | Extracts lowest-priced accommodations to surface cost-saving alternatives. |
| **Hotel Sorting** | In-place Array | **QuickSort** | $O(N \log N)$ avg | Dynamically sorts hotels by Price, Rating, or Distance to city center. |
| **Search Engine** | Hash Table with Separate Chaining | **djb2 Polynomial Hashing** | $O(1)$ average | High-speed keyword and category lookup across attractions. |
| **Trip Management** | AVL Tree (Self-Balancing BST) | **Rotations (LL, RR, LR, RL)** | $O(\log N)$ | Fast insert, search, update, delete, and in-order chronological traversal. |

---

## Compilation & Execution Guide

### 1. Compile the C Core Backend
From the project root:
```powershell
gcc -Wall -Wextra -O2 -I"TripPlanner\backend" TripPlanner\backend\data.c TripPlanner\backend\location_route.c TripPlanner\backend\itinerary.c TripPlanner\backend\budget.c TripPlanner\backend\search.c TripPlanner\backend\trip_management.c TripPlanner\backend\main.c -o TripPlanner\backend\trip_planner.exe -lm
```

### 2. Run Automated Unit Tests (Optional)
```powershell
.\TripPlanner\backend\trip_planner.exe --api run_tests
```

### 3. Launch the Web Application Server
```powershell
cd TripPlanner\server
node server.js
```

Open your browser at:
```
http://localhost:3000
```
