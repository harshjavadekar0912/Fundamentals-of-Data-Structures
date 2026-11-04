/**
 * location_route.h - Graph route management, Min-Heap priority queue,
 * and Dijkstra's Shortest Path Algorithm.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef LOCATION_ROUTE_H
#define LOCATION_ROUTE_H

#include "data.h"

/* --- Min Heap Node for Priority Queue in Dijkstra --- */
typedef struct {
    int vertex;
    double dist;
} MinHeapNode;

/* --- Min Heap Data Structure --- */
typedef struct {
    int size;
    int capacity;
    int* pos;               /* Keeps track of position of vertices in heap array for decreaseKey */
    MinHeapNode** array;
} MinHeap;

/* --- Route Step / Hop Structure --- */
typedef struct {
    int fromId;
    char fromName[MAX_NAME_LEN];
    int toId;
    char toName[MAX_NAME_LEN];
    double distanceKm;
    int travelTimeMin;
    double tollCost;
} RouteHop;

/* --- Dijkstra Shortest Path Result --- */
typedef struct {
    int reachable;          /* 1 if reachable, 0 if unreachable */
    int srcId;
    int destId;
    double totalDistanceKm;
    int totalTimeMin;
    double totalTollCost;
    int pathLength;
    int path[MAX_LOCATIONS];
    int hopCount;
    RouteHop hops[MAX_LOCATIONS];
    char statusMessage[128];
} DijkstraResult;

/* Min-Heap Priority Queue Prototypes */
MinHeap* createMinHeap(int capacity);
void freeMinHeap(MinHeap* minHeap);
int isEmptyMinHeap(const MinHeap* minHeap);
MinHeapNode* extractMin(MinHeap* minHeap);
void decreaseKey(MinHeap* minHeap, int vertex, double dist);
int isInMinHeap(const MinHeap* minHeap, int vertex);

/* Route & Graph Prototypes */
DijkstraResult dijkstraShortestPath(const Graph* g, int srcId, int destId);
void addLocationToGraph(Graph* g, const Location* loc);
void addRoadToGraph(Graph* g, int srcId, int destId, double distKm, int timeMin, double toll);

#endif /* LOCATION_ROUTE_H */

