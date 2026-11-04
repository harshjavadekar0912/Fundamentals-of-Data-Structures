/**
 * location_route.c - Min Heap Priority Queue and Dijkstra's Shortest Path Algorithm.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 * 
 * ALGORITHM SPECIFICATION:
 * - Data Structure: Adjacency List for Graph, Min-Heap Priority Queue for Dijkstra.
 * - Time Complexity: O((V + E) log V) where V = number of locations, E = number of roads.
 * - Space Complexity: O(V) auxiliary space for heap, distances, and parent pointers.
 */

#include "location_route.h"

/* -------------------------------------------------------------------------
   MIN-HEAP PRIORITY QUEUE IMPLEMENTATION
   ------------------------------------------------------------------------- */

static MinHeapNode* newMinHeapNode(int vertex, double dist) {
    MinHeapNode* node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    node->vertex = vertex;
    node->dist = dist;
    return node;
}

MinHeap* createMinHeap(int capacity) {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->pos = (int*)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (MinHeapNode**)malloc(capacity * sizeof(MinHeapNode*));
    for (int i = 0; i < capacity; i++) {
        minHeap->pos[i] = -1;
    }
    return minHeap;
}

static void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->dist < minHeap->array[smallest]->dist) {
        smallest = left;
    }
    if (right < minHeap->size && minHeap->array[right]->dist < minHeap->array[smallest]->dist) {
        smallest = right;
    }

    if (smallest != idx) {
        MinHeapNode* smallestNode = minHeap->array[smallest];
        MinHeapNode* idxNode = minHeap->array[idx];

        /* Update positions */
        minHeap->pos[smallestNode->vertex] = idx;
        minHeap->pos[idxNode->vertex] = smallest;

        /* Swap nodes in heap array */
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

int isEmptyMinHeap(const MinHeap* minHeap) {
    return (minHeap->size == 0);
}

MinHeapNode* extractMin(MinHeap* minHeap) {
    if (isEmptyMinHeap(minHeap)) return NULL;

    MinHeapNode* root = minHeap->array[0];
    MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    /* Update position of last node and root */
    minHeap->pos[root->vertex] = -1; /* removed */
    minHeap->pos[lastNode->vertex] = 0;

    minHeap->size--;
    minHeapify(minHeap, 0);

    return root;
}

void decreaseKey(MinHeap* minHeap, int vertex, double dist) {
    int i = minHeap->pos[vertex];
    if (i < 0 || i >= minHeap->size) return;

    minHeap->array[i]->dist = dist;

    /* Up-heapify while parent key is greater */
    while (i > 0 && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist) {
        int parentIdx = (i - 1) / 2;
        minHeap->pos[minHeap->array[i]->vertex] = parentIdx;
        minHeap->pos[minHeap->array[parentIdx]->vertex] = i;
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[parentIdx]);
        i = parentIdx;
    }
}

int isInMinHeap(const MinHeap* minHeap, int vertex) {
    if (vertex < 0 || vertex >= minHeap->capacity) return 0;
    return (minHeap->pos[vertex] >= 0 && minHeap->pos[vertex] < minHeap->size);
}

void freeMinHeap(MinHeap* minHeap) {
    if (!minHeap) return;
    for (int i = 0; i < minHeap->size; i++) {
        if (minHeap->array[i]) free(minHeap->array[i]);
    }
    free(minHeap->array);
    free(minHeap->pos);
    free(minHeap);
}

/* -------------------------------------------------------------------------
   DIJKSTRA'S SHORTEST PATH ALGORITHM
   ------------------------------------------------------------------------- */

DijkstraResult dijkstraShortestPath(const Graph* g, int srcId, int destId) {
    DijkstraResult res;
    memset(&res, 0, sizeof(DijkstraResult));
    res.srcId = srcId;
    res.destId = destId;

    /* Input Validation */
    if (!g || g->numLocations <= 0) {
        res.reachable = 0;
        snprintf(res.statusMessage, sizeof(res.statusMessage), "Error: Graph is empty or uninitialized.");
        return res;
    }

    if (srcId < 0 || srcId >= g->numLocations || destId < 0 || destId >= g->numLocations) {
        res.reachable = 0;
        snprintf(res.statusMessage, sizeof(res.statusMessage), "Error: Invalid source or destination ID (%d -> %d).", srcId, destId);
        return res;
    }

    /* Edge case: Source is Destination */
    if (srcId == destId) {
        res.reachable = 1;
        res.totalDistanceKm = 0.0;
        res.totalTimeMin = 0;
        res.totalTollCost = 0.0;
        res.pathLength = 1;
        res.path[0] = srcId;
        res.hopCount = 0;
        snprintf(res.statusMessage, sizeof(res.statusMessage), "Origin and destination are the same location.");
        return res;
    }

    int V = g->numLocations;
    double dist[MAX_LOCATIONS];
    int parent[MAX_LOCATIONS];

    for (int v = 0; v < V; v++) {
        dist[v] = INF_DIST;
        parent[v] = -1;
    }

    MinHeap* minHeap = createMinHeap(V);

    for (int v = 0; v < V; v++) {
        minHeap->array[v] = newMinHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
    minHeap->size = V;

    /* Distance to source is 0 */
    dist[srcId] = 0.0;
    decreaseKey(minHeap, srcId, 0.0);

    /* Process vertices in Priority Queue */
    while (!isEmptyMinHeap(minHeap)) {
        MinHeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;
        double d = minNode->dist;
        free(minNode);

        if (d >= INF_DIST) {
            /* Remaining vertices are unreachable from source */
            break;
        }

        if (u == destId) {
            /* Early exit: reached destination with optimal distance */
            break;
        }

        /* Traverse all adjacent roads/edges of u */
        EdgeNode* crawl = g->adjList[u];
        while (crawl != NULL) {
            int v = crawl->destinationId;
            if (isInMinHeap(minHeap, v) && dist[u] != INF_DIST && dist[u] + crawl->distanceKm < dist[v]) {
                dist[v] = dist[u] + crawl->distanceKm;
                parent[v] = u;
                decreaseKey(minHeap, v, dist[v]);
            }
            crawl = crawl->next;
        }
    }

    freeMinHeap(minHeap);

    /* Check if Destination was Reached */
    if (dist[destId] >= INF_DIST || parent[destId] == -1) {
        res.reachable = 0;
        snprintf(res.statusMessage, sizeof(res.statusMessage),
                 "No road path exists between %s and %s (Unreachable destination).",
                 g->locations[srcId].name, g->locations[destId].name);
        return res;
    }

    /* Reconstruct Shortest Path */
    int tempPath[MAX_LOCATIONS];
    int count = 0;
    int curr = destId;
    while (curr != -1) {
        tempPath[count++] = curr;
        curr = parent[curr];
    }

    res.reachable = 1;
    res.pathLength = count;
    for (int i = 0; i < count; i++) {
        res.path[i] = tempPath[count - 1 - i];
    }
    res.totalDistanceKm = dist[destId];

    /* Calculate hops, cumulative time, and tolls */
    res.hopCount = count - 1;
    res.totalTimeMin = 0;
    res.totalTollCost = 0.0;

    for (int i = 0; i < res.hopCount; i++) {
        int u = res.path[i];
        int v = res.path[i + 1];

        res.hops[i].fromId = u;
        strncpy(res.hops[i].fromName, g->locations[u].name, MAX_NAME_LEN - 1);
        res.hops[i].toId = v;
        strncpy(res.hops[i].toName, g->locations[v].name, MAX_NAME_LEN - 1);

        /* Find edge between u and v */
        EdgeNode* edge = g->adjList[u];
        while (edge != NULL) {
            if (edge->destinationId == v) {
                res.hops[i].distanceKm = edge->distanceKm;
                res.hops[i].travelTimeMin = edge->travelTimeMin;
                res.hops[i].tollCost = edge->tollCost;
                res.totalTimeMin += edge->travelTimeMin;
                res.totalTollCost += edge->tollCost;
                break;
            }
            edge = edge->next;
        }
    }

    snprintf(res.statusMessage, sizeof(res.statusMessage),
             "Optimal route calculated successfully: %d stops, %.1f km, %d hrs %d mins.",
             res.pathLength, res.totalDistanceKm, res.totalTimeMin / 60, res.totalTimeMin % 60);

    return res;
}

void addLocationToGraph(Graph* g, const Location* loc) {
    if (!g || !loc || g->numLocations >= MAX_LOCATIONS) return;
    int idx = g->numLocations;
    g->locations[idx] = *loc;
    g->locations[idx].id = idx;
    g->adjList[idx] = NULL;
    g->numLocations++;
}

void addRoadToGraph(Graph* g, int srcId, int destId, double distKm, int timeMin, double toll) {
    if (!g || srcId < 0 || srcId >= g->numLocations || destId < 0 || destId >= g->numLocations) return;

    EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e1->destinationId = destId;
    e1->distanceKm = distKm;
    e1->travelTimeMin = timeMin;
    e1->tollCost = toll;
    e1->next = g->adjList[srcId];
    g->adjList[srcId] = e1;

    EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
    e2->destinationId = srcId;
    e2->distanceKm = distKm;
    e2->travelTimeMin = timeMin;
    e2->tollCost = toll;
    e2->next = g->adjList[destId];
    g->adjList[destId] = e2;
}

