/**
 * data.h - Data models, constants, and master dataset declarations.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LOCATIONS 30
#define MAX_ROADS 100
#define MAX_ATTRACTIONS 100
#define MAX_HOTELS 60
#define INF_DIST 99999999.0
#define MAX_NAME_LEN 64
#define MAX_DESC_LEN 256

/* --- Location Structure --- */
typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char state[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    double latitude;
    double longitude;
} Location;

/* --- Road (Graph Edge) Structure --- */
typedef struct EdgeNode {
    int destinationId;
    double distanceKm;
    int travelTimeMin;
    double tollCost;
    struct EdgeNode* next;
} EdgeNode;

/* --- Graph (Adjacency List) --- */
typedef struct {
    int numLocations;
    Location locations[MAX_LOCATIONS];
    EdgeNode* adjList[MAX_LOCATIONS];
} Graph;

/* --- Tourist Attraction Structure --- */
typedef struct {
    int id;
    int locationId;
    char name[MAX_NAME_LEN];
    char category[32];       /* Historical, Nature, Cultural, Adventure, Religious, Food */
    double ticketCost;       /* in USD */
    int avgVisitTimeMin;     /* e.g., 90, 120 */
    int openTime;            /* HHMM format, e.g., 900 for 09:00 */
    int closeTime;           /* HHMM format, e.g., 1800 for 18:00 */
    double rating;           /* 0.0 - 5.0 */
    char description[MAX_DESC_LEN];
} Attraction;

/* --- Hotel Structure --- */
typedef struct {
    int id;
    int locationId;
    char name[MAX_NAME_LEN];
    char tier[16];           /* Budget, Standard, Luxury */
    double pricePerNight;    /* in USD */
    double rating;           /* 0.0 - 5.0 */
    double distFromCenterKm;
    char amenities[128];
} Hotel;

/* --- Master Dataset Repository --- */
typedef struct {
    Graph graph;
    Attraction attractions[MAX_ATTRACTIONS];
    int numAttractions;
    Hotel hotels[MAX_HOTELS];
    int numHotels;
} MasterData;

/* Function Prototypes */
void initMasterData(MasterData* data);
const Location* findLocationById(const MasterData* data, int id);
const Location* findLocationByName(const MasterData* data, const char* name);
const Attraction* findAttractionById(const MasterData* data, int id);
const Hotel* findHotelById(const MasterData* data, int id);

#endif /* DATA_H */

