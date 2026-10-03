#include <stdio.h>
#include <string.h>

#define MAX 10
#define INF 999999

/* ==================================================
   GRAPH STRUCTURES
   ================================================== */

struct Edge {
    int destination;
    int distance;
    int duration;
};

struct Location {
    char name[40];
    struct Edge edges[MAX];
    int edgeCount;
};

struct Location graph[MAX];

int locationCount = 0;


/* ==================================================
   PART 1: GRAPH + ADJACENCY LIST
   ================================================== */

/* Add location */
void addLocation(char name[]) {

    strcpy(graph[locationCount].name, name);
    graph[locationCount].edgeCount = 0;

    locationCount++;
}


/* Find location */
int findLocation(char name[]) {

    for (int i = 0; i < locationCount; i++) {

        if (strcmp(graph[i].name, name) == 0)
            return i;
    }

    return -1;
}


/* Add two-way road */
void addRoad(char source[],
             char destination[],
             int distance,
             int duration) {

    int s = findLocation(source);
    int d = findLocation(destination);

    /* Source -> Destination */

    graph[s].edges[graph[s].edgeCount].destination = d;
    graph[s].edges[graph[s].edgeCount].distance = distance;
    graph[s].edges[graph[s].edgeCount].duration = duration;

    graph[s].edgeCount++;


    /* Destination -> Source */

    graph[d].edges[graph[d].edgeCount].destination = s;
    graph[d].edges[graph[d].edgeCount].distance = distance;
    graph[d].edges[graph[d].edgeCount].duration = duration;

    graph[d].edgeCount++;
}


/* Display graph */
void displayGraph() {

    printf("\n========== PART 1: GRAPH ==========\n");

    for (int i = 0; i < locationCount; i++) {

        printf("%s -> ", graph[i].name);

        for (int j = 0; j < graph[i].edgeCount; j++) {

            int d = graph[i].edges[j].destination;

            printf("%s (%d km, %d min)",
                   graph[d].name,
                   graph[i].edges[j].distance,
                   graph[i].edges[j].duration);

            if (j < graph[i].edgeCount - 1)
                printf(", ");
        }

        printf("\n");
    }
}


/* ==================================================
   INTERACTIVE USER SELECTION
   ================================================== */

int selected[MAX];
int selectedCount = 0;


/* Add selected location */
void addSelectedLocation(int index) {

    selected[selectedCount] = index;
    selectedCount++;
}


/* Display available options */
void displayOptions() {

    printf("\n========================================\n");
    printf("       PUNE TOURIST GUIDE SYSTEM\n");
    printf("========================================\n");

    printf("\nHOTELS:\n");
    printf("1. Hotel Shreyas\n");
    printf("2. Hotel Saffron\n");

    printf("\nTOURIST DESTINATIONS:\n");
    printf("1. Shaniwar Wada\n");
    printf("2. Aga Khan Palace\n");
    printf("3. Sinhagad Fort\n");
    printf("4. Pataleshwar Cave Temple\n");
    printf("5. Raja Dinkar Kelkar Museum\n");

    printf("\nRESTAURANTS:\n");
    printf("1. Vaishali\n");
    printf("2. Shabree\n");
}


/* Interactive selection */
void userSelection() {

    int choice;
    int number;

    printf("\n========================================\n");
    printf("          SELECT YOUR HOTEL\n");
    printf("========================================\n");

    printf("1. Hotel Shreyas\n");
    printf("2. Hotel Saffron\n");

    printf("\nEnter hotel choice: ");
    scanf("%d", &choice);

    if (choice == 1)
        addSelectedLocation(findLocation("Hotel Shreyas"));
    else
        addSelectedLocation(findLocation("Hotel Saffron"));


    /* Tourist destinations */

    printf("\n========================================\n");
    printf("     SELECT TOURIST DESTINATIONS\n");
    printf("========================================\n");

    printf("How many destinations do you want to visit? (0-5): ");
    scanf("%d", &number);

    if (number > 5)
        number = 5;

    for (int i = 0; i < number; i++) {

        printf("\nAvailable destination:\n");

        printf("1. Shaniwar Wada\n");
        printf("2. Aga Khan Palace\n");
        printf("3. Sinhagad Fort\n");
        printf("4. Pataleshwar Cave Temple\n");
        printf("5. Raja Dinkar Kelkar Museum\n");

        printf("Choose destination %d: ", i + 1);
        scanf("%d", &choice);

        if (choice == 1)
            addSelectedLocation(findLocation("Shaniwar Wada"));

        else if (choice == 2)
            addSelectedLocation(findLocation("Aga Khan Palace"));

        else if (choice == 3)
            addSelectedLocation(findLocation("Sinhagad Fort"));

        else if (choice == 4)
            addSelectedLocation(findLocation("Pataleshwar Cave Temple"));

        else if (choice == 5)
            addSelectedLocation(findLocation("Raja Dinkar Kelkar Museum"));
    }


    /* Restaurants */

    printf("\n========================================\n");
    printf("        SELECT RESTAURANTS\n");
    printf("========================================\n");

    printf("How many restaurants do you want to visit? (0-2): ");
    scanf("%d", &number);

    if (number > 2)
        number = 2;

    for (int i = 0; i < number; i++) {

        printf("\n1. Vaishali\n");
        printf("2. Shabree\n");

        printf("Choose restaurant %d: ", i + 1);
        scanf("%d", &choice);

        if (choice == 1)
            addSelectedLocation(findLocation("Vaishali"));

        else if (choice == 2)
            addSelectedLocation(findLocation("Shabree"));
    }


    /* Display selected locations */

    printf("\n========================================\n");
    printf("       YOUR SELECTED LOCATIONS\n");
    printf("========================================\n");

    for (int i = 0; i < selectedCount; i++) {

        printf("%d. %s\n",
               i + 1,
               graph[selected[i]].name);
    }
}
/* ==================================================
   PART 2: MIN HEAP
   ================================================== */

struct HeapNode {

    int vertex;
    int distance;
};

struct HeapNode heap[MAX * MAX];

int heapSize = 0;


/* Swap heap nodes */
void swap(struct HeapNode *a,
          struct HeapNode *b) {

    struct HeapNode temp = *a;

    *a = *b;
    *b = temp;
}


/* Insert into Min Heap */
void push(int vertex,
          int distance) {

    int i = heapSize;

    heap[heapSize].vertex = vertex;
    heap[heapSize].distance = distance;

    heapSize++;

    while (i > 0) {

        int parent = (i - 1) / 2;

        if (heap[parent].distance <=
            heap[i].distance)

            break;

        swap(&heap[parent],
             &heap[i]);

        i = parent;
    }
}


/* Remove minimum */
struct HeapNode pop() {

    struct HeapNode result = heap[0];

    heap[0] = heap[--heapSize];

    int i = 0;

    while (1) {

        int left = 2 * i + 1;
        int right = 2 * i + 2;

        int smallest = i;

        if (left < heapSize &&
            heap[left].distance <
            heap[smallest].distance)

            smallest = left;


        if (right < heapSize &&
            heap[right].distance <
            heap[smallest].distance)

            smallest = right;


        if (smallest == i)
            break;

        swap(&heap[i],
             &heap[smallest]);

        i = smallest;
    }

    return result;
}


/* ==================================================
   PART 2: DIJKSTRA
   ================================================== */

void dijkstra(int source,
              int distance[],
              int parent[]) {

    for (int i = 0;
         i < locationCount;
         i++) {

        distance[i] = INF;
        parent[i] = -1;
    }

    distance[source] = 0;

    heapSize = 0;

    push(source, 0);


    while (heapSize > 0) {

        struct HeapNode current = pop();

        int u = current.vertex;

        /* Ignore outdated heap value */

        if (current.distance !=
            distance[u])

            continue;


        /* Check neighbours */

        for (int i = 0;
             i < graph[u].edgeCount;
             i++) {

            int v =
                graph[u]
                .edges[i]
                .destination;

            int weight =
                graph[u]
                .edges[i]
                .distance;

            int newDistance =
                distance[u] + weight;


            /* Relaxation */

            if (newDistance <
                distance[v]) {

                distance[v] =
                    newDistance;

                parent[v] = u;

                push(v,
                     newDistance);
            }
        }
    }
}


/* Print shortest path */
void printPath(int parent[],
               int vertex) {

    if (parent[vertex] == -1) {

        printf("%s",
               graph[vertex].name);

        return;
    }

    printPath(parent,
              parent[vertex]);

    printf(" -> %s",
           graph[vertex].name);
}


/* Create distance matrix */
void createDistanceMatrix(
    int matrix[MAX][MAX]) {

    int distance[MAX];
    int parent[MAX];

    /*
       Only selected locations are important
       for the itinerary.
    */

    for (int i = 0;
         i < selectedCount;
         i++) {

        int source = selected[i];

        dijkstra(source,
                 distance,
                 parent);

        for (int j = 0;
             j < selectedCount;
             j++) {

            int destination =
                selected[j];

            matrix[i][j] =
                distance[destination];
        }
    }
}


/* Display shortest paths between selected locations */

void displaySelectedShortestPaths() {

    int distance[MAX];
    int parent[MAX];

    printf("\n========================================\n");
    printf("     PART 2: SHORTEST PATHS\n");
    printf("========================================\n");

    for (int i = 0;
         i < selectedCount;
         i++) {

        dijkstra(selected[i],
                 distance,
                 parent);

        for (int j = i + 1;
             j < selectedCount;
             j++) {

            printf("\n%s -> %s\n",
                   graph[selected[i]].name,
                   graph[selected[j]].name);

            printPath(parent,
                      selected[j]);

            printf("\nDistance: %d km\n",
                   distance[selected[j]]);
        }
    }
}
/* ==================================================
   PART 3: GREEDY ITINERARY
   ================================================== */

void greedyTSP(
    int start,
    int matrix[MAX][MAX]) {

    int visited[MAX] = {0};

    int route[MAX + 1];

    int current = start;

    int totalDistance = 0;

    visited[start] = 1;

    route[0] = start;


    /* Find nearest unvisited location */

    for (int count = 1;
         count < selectedCount;
         count++) {

        int nearest = -1;

        int minimum = INF;


        for (int i = 0;
             i < selectedCount;
             i++) {

            if (!visited[i] &&
                matrix[current][i] <
                minimum) {

                minimum =
                    matrix[current][i];

                nearest = i;
            }
        }


        if (nearest == -1)
            break;


        visited[nearest] = 1;

        route[count] = nearest;

        totalDistance += minimum;

        current = nearest;
    }


    /* Return to hotel */

    route[selectedCount] = start;

    totalDistance +=
        matrix[current][start];


    /* Display */

    printf("\n========================================\n");
    printf("       PART 3: GREEDY ITINERARY\n");
    printf("========================================\n");

    printf("\nGreedy Itinerary:\n");

    for (int i = 0;
         i <= selectedCount;
         i++) {

        printf("%s",
               graph[selected[route[i]]].name);

        if (i < selectedCount)
            printf(" -> ");
    }

    printf("\n\nTotal Distance: %d km\n",
           totalDistance);
}
/* ==================================================
   PART 4: ACTUAL TSP
   ================================================== */

int tspBestDistance;

int tspBestRoute[MAX + 1];

int tspCurrentRoute[MAX + 1];

int tspVisited[MAX];

int tspStart;


/* Recursive TSP */

void tsp(int current,
         int count,
         int currentDistance,
         int matrix[MAX][MAX]) {

    /* All selected locations visited */

    if (count == selectedCount) {

        int totalDistance =
            currentDistance +
            matrix[current][tspStart];


        /* New best route */

        if (totalDistance <
            tspBestDistance) {

            tspBestDistance =
                totalDistance;


            for (int i = 0;
                 i < selectedCount;
                 i++) {

                tspBestRoute[i] =
                    tspCurrentRoute[i];
            }


            tspBestRoute[selectedCount] =
                tspStart;
        }

        return;
    }


    /* Try every unvisited location */

    for (int i = 0;
         i < selectedCount;
         i++) {

        if (!tspVisited[i]) {

            tspVisited[i] = 1;

            tspCurrentRoute[count] =
                i;


            tsp(
                i,
                count + 1,
                currentDistance +
                matrix[current][i],
                matrix
            );


            /* Backtracking */

            tspVisited[i] = 0;
        }
    }
}


/* Run actual TSP */

void runTSP(
    int start,
    int matrix[MAX][MAX]) {

    tspStart = start;

    tspBestDistance = INF;


    /* Reset visited */

    for (int i = 0;
         i < selectedCount;
         i++) {

        tspVisited[i] = 0;
    }


    /* Start */

    tspVisited[start] = 1;

    tspCurrentRoute[0] = start;


    /* Run TSP */

    tsp(
        start,
        1,
        0,
        matrix
    );


    /* Display */

    printf("\n========================================\n");
    printf("        PART 4: ACTUAL TSP\n");
    printf("========================================\n");

    printf("\nOptimized Itinerary:\n");

    for (int i = 0;
         i <= selectedCount;
         i++) {

        printf("%s",
               graph[
                   selected[
                       tspBestRoute[i]
                   ]
               ].name);

        if (i < selectedCount)
            printf(" -> ");
    }


    printf("\n\nMinimum Distance: %d km\n",
           tspBestDistance);
}