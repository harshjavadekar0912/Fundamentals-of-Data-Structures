/**
 * trip_management.h - AVL Tree (Self-Balancing Binary Search Tree) for managing saved trips.
 * Supports add, search, update, delete, sorted in-order traversal, and file persistence.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef TRIP_MANAGEMENT_H
#define TRIP_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SAVED_TRIPS 100

/* --- Saved Trip Record Structure --- */
typedef struct {
    int tripId;
    char tripName[64];
    char startDate[16];          /* "YYYY-MM-DD" */
    int dateInt;                 /* YYYYMMDD integer key for AVL BST ordering */
    char sourceLocation[64];
    char destLocation[64];
    int numDays;
    int numTravellers;
    double totalBudget;
    double estimatedCost;
    double totalDistanceKm;
    char travelStyle[16];
    char notes[256];
} TripRecord;

/* --- AVL Tree Node Structure --- */
typedef struct AVLNode {
    TripRecord trip;
    int height;
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

/* Helper Date Converter: "2026-11-15" -> 20261115 */
int dateStringToInteger(const char* dateStr);

/* AVL Tree Functions */
int getAVLHeight(const AVLNode* node);
int getAVLBalanceFactor(const AVLNode* node);
AVLNode* avlInsertTrip(AVLNode* root, const TripRecord* trip);
const AVLNode* avlSearchTripByDate(const AVLNode* root, int dateInt);
const AVLNode* avlSearchTripById(const AVLNode* root, int tripId);
AVLNode* avlDeleteTripById(AVLNode* root, int tripId);
AVLNode* avlUpdateTrip(AVLNode* root, int tripId, const TripRecord* updatedTrip);
void avlCollectInOrder(const AVLNode* root, TripRecord* list, int* count, int maxCount);
void freeAVLTree(AVLNode* root);

/* File Persistence Functions */
int loadTripsFromFile(AVLNode** root, const char* filename);
int saveTripsToFile(const AVLNode* root, const char* filename);

#endif /* TRIP_MANAGEMENT_H */

