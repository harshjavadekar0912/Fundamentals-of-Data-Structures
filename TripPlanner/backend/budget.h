/**
 * budget.h - Budget calculation, Min-Heap cheapest hotel recommendations,
 * multi-criteria sorting (Price, Rating, Distance), and budget alerts.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef BUDGET_H
#define BUDGET_H

#include "data.h"

#define MAX_RECOMMENDED_HOTELS 6

/* --- Hotel Min Heap for Extracting Lowest Prices --- */
typedef struct {
    int hotelId;
    double price;
} HotelHeapNode;

typedef struct {
    int size;
    int capacity;
    HotelHeapNode* array;
} HotelMinHeap;

/* --- Itemized Budget Breakdown --- */
typedef struct {
    double userBudget;
    int numDays;
    int numTravellers;
    char travelStyle[16];          /* Budget, Standard, Luxury */
    int selectedHotelId;
    Hotel selectedHotel;
    double hotelExpense;
    double foodExpense;
    double transportExpense;
    double activityExpense;
    double emergencyBuffer;
    double totalExpense;
    double remainingBudget;
    char status[32];               /* "Within Budget", "Near Limit", "Over Budget" */
    char warningMessage[512];
    int topRecommendedCount;
    Hotel topRecommendations[3];
    int cheaperAltCount;
    Hotel cheaperAlternatives[3];
    double potentialSavings;
} BudgetBreakdown;

/* Hotel Min-Heap Prototypes */
HotelMinHeap* createHotelMinHeap(int capacity);
void freeHotelMinHeap(HotelMinHeap* heap);
void insertHotelMinHeap(HotelMinHeap* heap, int hotelId, double price);
HotelHeapNode extractMinHotel(HotelMinHeap* heap);

/* Sorting Prototypes (C QuickSort implementations) */
void sortHotelsByPrice(Hotel* arr, int n, int ascending);
void sortHotelsByRating(Hotel* arr, int n, int ascending);
void sortHotelsByDistance(Hotel* arr, int n, int ascending);

/* Core Budget Calculation Prototypes */
BudgetBreakdown calculateBudget(const MasterData* data,
                                double userBudget,
                                int numDays,
                                int numTravellers,
                                const char* travelStyle,
                                int destLocationId,
                                int selectedHotelId,
                                double routeDistanceKm,
                                double activityTicketTotal);

#endif /* BUDGET_H */

