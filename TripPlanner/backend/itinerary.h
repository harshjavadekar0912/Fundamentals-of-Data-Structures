/**
 * itinerary.h - Itinerary optimization using Exact TSP with recursion/backtracking,
 * and constraint-based day-wise scheduling.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef ITINERARY_H
#define ITINERARY_H

#include "data.h"
#include "location_route.h"

#define MAX_SELECTED_ATTRS 16
#define MAX_DAYS 14
#define MAX_SCHEDULE_PER_DAY 10
#define DEFAULT_DAY_START_TIME 900  /* 09:00 AM */
#define MAX_DAILY_ACTIVE_MINUTES 540 /* 9 hours max active time per day */

/* --- Exact TSP Optimization Result --- */
typedef struct {
    int numStops;
    int sequence[MAX_SELECTED_ATTRS];      /* Attraction sequence in optimal visiting order */
    double totalDistanceKm;
    int totalTravelTimeMin;
    int isOptimal;                         /* 1 for exact optimal solution */
} TSPResult;

/* --- Day-wise Scheduled Attraction Item --- */
typedef struct {
    int attractionId;
    char name[MAX_NAME_LEN];
    char category[32];
    int arrivalTime;                       /* HHMM, e.g. 915 */
    int visitStartTime;                   /* HHMM, e.g. 930 if waiting for opening */
    int visitEndTime;                     /* HHMM, e.g. 1100 */
    int visitDurationMin;
    int travelTimeFromPrevMin;
    double ticketCost;                    /* in INR (₹) */
    char timingNote[128];
} ScheduledItem;

/* --- Single Day Itinerary Schedule --- */
typedef struct {
    int dayNumber;
    int itemCount;
    ScheduledItem items[MAX_SCHEDULE_PER_DAY];
    int dayStartTime;                      /* e.g. 900 */
    int dayEndTime;                        /* e.g. 1745 */
    int totalActiveMinutes;
    double totalTicketCost;               /* in INR (₹) */
} DaySchedule;

/* --- Complete Generated Itinerary Plan --- */
typedef struct {
    int numDays;
    DaySchedule days[MAX_DAYS];
    int totalAttractionsScheduled;
    double grandTotalTicketCost;          /* in INR (₹) */
    TSPResult optimalOrder;
    double totalDistanceKm;
    int totalTravelTimeMin;
} FullItinerary;

/* Function Prototypes */
void buildDistanceMatrix(const MasterData* data, const int* attrIds, int count,
                         double distMatrix[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS],
                         int timeMatrix[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS]);

TSPResult solveExactTSP(int count,
                        const double distMatrix[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS],
                        const int timeMatrix[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS]);

FullItinerary generateDayWiseItinerary(const MasterData* data,
                                       const int* attrIds,
                                       int count,
                                       int numDays);

#endif /* ITINERARY_H */
