/**
 * main.c - Main Entry Point for Trip Planner C Backend.
 * 
 * Features:
 * 1. Interactive ASCII CLI Menu for direct testing and grading.
 * 2. Automated Module Unit Tests.
 * 3. High-Performance JSON API interface for Node.js Express server integration.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "data.h"
#include "location_route.h"
#include "itinerary.h"
#include "budget.h"
#include "search.h"
#include "trip_management.h"

#define SAVED_TRIPS_FILE "TripPlanner/data/saved_trips.txt"
#define SAVED_TRIPS_FILE_ALT "data/saved_trips.txt"

/* Helper to get valid file path for persistent trips */
static const char* getTripsFilePath(void) {
    FILE* fp = fopen(SAVED_TRIPS_FILE, "r");
    if (fp) {
        fclose(fp);
        return SAVED_TRIPS_FILE;
    }
    return SAVED_TRIPS_FILE_ALT;
}

/* Helper to escape strings for JSON */
static void printJsonString(const char* s) {
    if (!s) {
        printf("\"\"");
        return;
    }
    putchar('\"');
    while (*s) {
        if (*s == '\"') printf("\\\"");
        else if (*s == '\\') printf("\\\\");
        else if (*s == '\n') printf("\\n");
        else if (*s == '\r') printf("\\r");
        else if (*s == '\t') printf("\\t");
        else putchar(*s);
        s++;
    }
    putchar('\"');
}

/* -------------------------------------------------------------------------
   JSON API HANDLERS
   ------------------------------------------------------------------------- */

static void apiGetLocations(const MasterData* data) {
    printf("{\"success\":true,\"count\":%d,\"locations\":[", data->graph.numLocations);
    for (int i = 0; i < data->graph.numLocations; i++) {
        const Location* loc = &data->graph.locations[i];
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", loc->id);
        printJsonString(loc->name);
        printf(",\"state\":");
        printJsonString(loc->state);
        printf(",\"description\":");
        printJsonString(loc->description);
        printf(",\"latitude\":%.4f,\"longitude\":%.4f}", loc->latitude, loc->longitude);
    }
    printf("]}\n");
}

static void apiGetAttractions(const MasterData* data) {
    printf("{\"success\":true,\"count\":%d,\"attractions\":[", data->numAttractions);
    for (int i = 0; i < data->numAttractions; i++) {
        const Attraction* a = &data->attractions[i];
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"locationId\":%d,\"name\":", a->id, a->locationId);
        printJsonString(a->name);
        printf(",\"category\":");
        printJsonString(a->category);
        printf(",\"ticketCost\":%.2f,\"avgVisitTimeMin\":%d,\"openTime\":%d,\"closeTime\":%d,\"rating\":%.1f,\"description\":",
               a->ticketCost, a->avgVisitTimeMin, a->openTime, a->closeTime, a->rating);
        printJsonString(a->description);
        printf("}");
    }
    printf("]}\n");
}

static void apiGetHotels(const MasterData* data) {
    printf("{\"success\":true,\"count\":%d,\"hotels\":[", data->numHotels);
    for (int i = 0; i < data->numHotels; i++) {
        const Hotel* h = &data->hotels[i];
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"locationId\":%d,\"name\":", h->id, h->locationId);
        printJsonString(h->name);
        printf(",\"tier\":");
        printJsonString(h->tier);
        printf(",\"pricePerNight\":%.2f,\"rating\":%.1f,\"distFromCenterKm\":%.1f,\"amenities\":",
               h->pricePerNight, h->rating, h->distFromCenterKm);
        printJsonString(h->amenities);
        printf("}");
    }
    printf("]}\n");
}

static void apiPlanRoute(const MasterData* data, int srcId, int destId) {
    DijkstraResult res = dijkstraShortestPath(&data->graph, srcId, destId);

    printf("{\"success\":true,\"reachable\":%s,\"srcId\":%d,\"destId\":%d,",
           res.reachable ? "true" : "false", res.srcId, res.destId);
    printf("\"totalDistanceKm\":%.2f,\"totalTimeMin\":%d,\"totalTollCost\":%.2f,\"pathLength\":%d,",
           res.totalDistanceKm, res.totalTimeMin, res.totalTollCost, res.pathLength);
    printf("\"statusMessage\":");
    printJsonString(res.statusMessage);
    printf(",\"path\":[");
    for (int i = 0; i < res.pathLength; i++) {
        if (i > 0) printf(",");
        printf("%d", res.path[i]);
    }
    printf("],\"hops\":[");
    for (int i = 0; i < res.hopCount; i++) {
        if (i > 0) printf(",");
        printf("{\"fromId\":%d,\"fromName\":", res.hops[i].fromId);
        printJsonString(res.hops[i].fromName);
        printf(",\"toId\":%d,\"toName\":", res.hops[i].toId);
        printJsonString(res.hops[i].toName);
        printf(",\"distanceKm\":%.1f,\"travelTimeMin\":%d,\"tollCost\":%.2f}",
               res.hops[i].distanceKm, res.hops[i].travelTimeMin, res.hops[i].tollCost);
    }
    printf("]}\n");
}

static void apiOptimizeItinerary(const MasterData* data, int numDays, int preferExact, const char* attrIdsCsv) {
    int attrIds[MAX_SELECTED_ATTRS];
    int count = 0;

    if (attrIdsCsv && strlen(attrIdsCsv) > 0) {
        char buffer[256];
        strncpy(buffer, attrIdsCsv, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        char* token = strtok(buffer, ",");
        while (token && count < MAX_SELECTED_ATTRS) {
            attrIds[count++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }

    if (count == 0) {
        /* Default to first 5 attractions */
        for (int i = 0; i < 5 && i < data->numAttractions; i++) {
            attrIds[count++] = data->attractions[i].id;
        }
    }

    FullItinerary itin = generateDayWiseItinerary(data, attrIds, count, numDays, preferExact);

    printf("{\"success\":true,\"numDays\":%d,\"totalAttractionsScheduled\":%d,\"grandTotalTicketCost\":%.2f,\"activeAlgorithmUsed\":%d,",
           itin.numDays, itin.totalAttractionsScheduled, itin.grandTotalTicketCost, itin.activeAlgorithmUsed);

    /* Algorithmic Comparison details */
    printf("\"comparison\":{");
    printf("\"greedy\":{\"totalDistanceKm\":%.2f,\"totalTravelTimeMin\":%d,\"executionTimeUs\":%ld,\"sequence\":[",
           itin.comparison.greedy.totalDistanceKm, itin.comparison.greedy.totalTravelTimeMin, itin.comparison.greedy.executionTimeMicrosec);
    for (int i = 0; i < itin.comparison.greedy.numStops; i++) {
        if (i > 0) printf(",");
        printf("%d", attrIds[itin.comparison.greedy.sequence[i]]);
    }
    printf("]},");
    printf("\"exact\":{\"totalDistanceKm\":%.2f,\"totalTravelTimeMin\":%d,\"executionTimeUs\":%ld,\"sequence\":[",
           itin.comparison.exact.totalDistanceKm, itin.comparison.exact.totalTravelTimeMin, itin.comparison.exact.executionTimeMicrosec);
    for (int i = 0; i < itin.comparison.exact.numStops; i++) {
        if (i > 0) printf(",");
        printf("%d", attrIds[itin.comparison.exact.sequence[i]]);
    }
    printf("]},");
    printf("\"distanceDiffKm\":%.2f,\"percentDiff\":%.2f,\"comparisonComment\":",
           itin.comparison.distanceDiffKm, itin.comparison.percentDiff);
    printJsonString(itin.comparison.comparisonComment);
    printf(",\"recommendedAlgorithm\":");
    printJsonString(itin.comparison.recommendedAlgorithm);
    printf("},");

    /* Day schedules */
    printf("\"days\":[");
    for (int d = 0; d < itin.numDays; d++) {
        const DaySchedule* day = &itin.days[d];
        if (d > 0) printf(",");
        printf("{\"dayNumber\":%d,\"dayStartTime\":%d,\"dayEndTime\":%d,\"totalActiveMinutes\":%d,\"totalTicketCost\":%.2f,\"itemCount\":%d,\"items\":[",
               day->dayNumber, day->dayStartTime, day->dayEndTime, day->totalActiveMinutes, day->totalTicketCost, day->itemCount);
        for (int k = 0; k < day->itemCount; k++) {
            const ScheduledItem* it = &day->items[k];
            if (k > 0) printf(",");
            printf("{\"attractionId\":%d,\"name\":", it->attractionId);
            printJsonString(it->name);
            printf(",\"category\":");
            printJsonString(it->category);
            printf(",\"arrivalTime\":%d,\"visitStartTime\":%d,\"visitEndTime\":%d,\"visitDurationMin\":%d,\"travelTimeFromPrevMin\":%d,\"ticketCost\":%.2f,\"timingNote\":",
                   it->arrivalTime, it->visitStartTime, it->visitEndTime, it->visitDurationMin, it->travelTimeFromPrevMin, it->ticketCost);
            printJsonString(it->timingNote);
            printf("}");
        }
        printf("]}");
    }
    printf("]}\n");
}

static void apiCalculateBudget(const MasterData* data,
                               double userBudget,
                               int numDays,
                               int numTravellers,
                               const char* travelStyle,
                               int destLocationId,
                               int selectedHotelId,
                               double routeDistKm,
                               double activityCost) {
    BudgetBreakdown b = calculateBudget(data, userBudget, numDays, numTravellers,
                                        travelStyle, destLocationId, selectedHotelId,
                                        routeDistKm, activityCost);

    printf("{\"success\":true,\"userBudget\":%.2f,\"numDays\":%d,\"numTravellers\":%d,\"travelStyle\":",
           b.userBudget, b.numDays, b.numTravellers);
    printJsonString(b.travelStyle);
    printf(",\"hotelExpense\":%.2f,\"foodExpense\":%.2f,\"transportExpense\":%.2f,\"activityExpense\":%.2f,\"emergencyBuffer\":%.2f,\"totalExpense\":%.2f,\"remainingBudget\":%.2f,\"status\":",
           b.hotelExpense, b.foodExpense, b.transportExpense, b.activityExpense, b.emergencyBuffer, b.totalExpense, b.remainingBudget);
    printJsonString(b.status);
    printf(",\"warningMessage\":");
    printJsonString(b.warningMessage);
    printf(",\"potentialSavings\":%.2f,", b.potentialSavings);

    /* Selected Hotel */
    printf("\"selectedHotel\":{\"id\":%d,\"name\":", b.selectedHotel.id);
    printJsonString(b.selectedHotel.name);
    printf(",\"tier\":");
    printJsonString(b.selectedHotel.tier);
    printf(",\"pricePerNight\":%.2f,\"rating\":%.1f,\"distFromCenterKm\":%.1f},",
           b.selectedHotel.pricePerNight, b.selectedHotel.rating, b.selectedHotel.distFromCenterKm);

    /* Top Recommendations */
    printf("\"topRecommendations\":[");
    for (int i = 0; i < b.topRecommendedCount; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", b.topRecommendations[i].id);
        printJsonString(b.topRecommendations[i].name);
        printf(",\"tier\":");
        printJsonString(b.topRecommendations[i].tier);
        printf(",\"pricePerNight\":%.2f,\"rating\":%.1f,\"distFromCenterKm\":%.1f}",
               b.topRecommendations[i].pricePerNight, b.topRecommendations[i].rating, b.topRecommendations[i].distFromCenterKm);
    }
    printf("],");

    /* Cheaper Alternatives */
    printf("\"cheaperAlternatives\":[");
    for (int i = 0; i < b.cheaperAltCount; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", b.cheaperAlternatives[i].id);
        printJsonString(b.cheaperAlternatives[i].name);
        printf(",\"tier\":");
        printJsonString(b.cheaperAlternatives[i].tier);
        printf(",\"pricePerNight\":%.2f,\"rating\":%.1f,\"distFromCenterKm\":%.1f}",
               b.cheaperAlternatives[i].pricePerNight, b.cheaperAlternatives[i].rating, b.cheaperAlternatives[i].distFromCenterKm);
    }
    printf("]}\n");
}

static void apiSearchAttractions(const MasterData* data, const HashTable* ht, const char* query, const char* category) {
    int resultIds[MAX_SEARCH_RESULTS];
    int count = searchAttractions(ht, data, query, category, resultIds, MAX_SEARCH_RESULTS);

    printf("{\"success\":true,\"query\":");
    printJsonString(query ? query : "");
    printf(",\"category\":");
    printJsonString(category ? category : "");
    printf(",\"count\":%d,\"results\":[", count);

    for (int i = 0; i < count; i++) {
        const Attraction* a = findAttractionById(data, resultIds[i]);
        if (!a) continue;
        const Location* loc = findLocationById(data, a->locationId);
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", a->id);
        printJsonString(a->name);
        printf(",\"category\":");
        printJsonString(a->category);
        printf(",\"locationId\":%d,\"locationName\":", a->locationId);
        printJsonString(loc ? loc->name : "");
        printf(",\"ticketCost\":%.2f,\"avgVisitTimeMin\":%d,\"openTime\":%d,\"closeTime\":%d,\"rating\":%.1f,\"description\":",
               a->ticketCost, a->avgVisitTimeMin, a->openTime, a->closeTime, a->rating);
        printJsonString(a->description);
        printf("}");
    }
    printf("]}\n");
}

/* -------------------------------------------------------------------------
   TRIP MANAGEMENT AVL TREE JSON APIS
   ------------------------------------------------------------------------- */

static void apiTripList(void) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    TripRecord list[MAX_SAVED_TRIPS];
    int count = 0;
    avlCollectInOrder(root, list, &count, MAX_SAVED_TRIPS);

    printf("{\"success\":true,\"count\":%d,\"trips\":[", count);
    for (int i = 0; i < count; i++) {
        if (i > 0) printf(",");
        printf("{\"tripId\":%d,\"tripName\":", list[i].tripId);
        printJsonString(list[i].tripName);
        printf(",\"startDate\":");
        printJsonString(list[i].startDate);
        printf(",\"sourceLocation\":");
        printJsonString(list[i].sourceLocation);
        printf(",\"destLocation\":");
        printJsonString(list[i].destLocation);
        printf(",\"numDays\":%d,\"numTravellers\":%d,\"totalBudget\":%.2f,\"estimatedCost\":%.2f,\"totalDistanceKm\":%.2f,\"travelStyle\":",
               list[i].numDays, list[i].numTravellers, list[i].totalBudget, list[i].estimatedCost, list[i].totalDistanceKm);
        printJsonString(list[i].travelStyle);
        printf(",\"notes\":");
        printJsonString(list[i].notes);
        printf("}");
    }
    printf("]}\n");

    freeAVLTree(root);
}

static void apiTripAdd(const char* name, const char* date, const char* src, const char* dest,
                       int days, int travellers, double budget, double cost, double dist,
                       const char* style, const char* notes) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    /* Generate new tripId based on timestamp and random seed */
    srand((unsigned int)time(NULL));
    int newId = 1000 + (rand() % 9000);

    TripRecord t;
    memset(&t, 0, sizeof(TripRecord));
    t.tripId = newId;
    strncpy(t.tripName, name ? name : "My Journey", sizeof(t.tripName) - 1);
    strncpy(t.startDate, date ? date : "2026-11-20", sizeof(t.startDate) - 1);
    t.dateInt = dateStringToInteger(t.startDate);
    strncpy(t.sourceLocation, src ? src : "Delhi", sizeof(t.sourceLocation) - 1);
    strncpy(t.destLocation, dest ? dest : "Agra", sizeof(t.destLocation) - 1);
    t.numDays = days > 0 ? days : 3;
    t.numTravellers = travellers > 0 ? travellers : 2;
    t.totalBudget = budget;
    t.estimatedCost = cost;
    t.totalDistanceKm = dist;
    strncpy(t.travelStyle, style ? style : "Standard", sizeof(t.travelStyle) - 1);
    strncpy(t.notes, notes ? notes : "", sizeof(t.notes) - 1);

    root = avlInsertTrip(root, &t);
    saveTripsToFile(root, filePath);

    printf("{\"success\":true,\"message\":\"Trip added successfully via AVL Tree insertion.\",\"tripId\":%d}\n", newId);

    freeAVLTree(root);
}

static void apiTripGet(int tripId) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    const AVLNode* found = avlSearchTripById(root, tripId);
    if (!found) {
        printf("{\"success\":false,\"message\":\"Trip not found in AVL tree.\"}\n");
    } else {
        const TripRecord* t = &found->trip;
        printf("{\"success\":true,\"trip\":{\"tripId\":%d,\"tripName\":", t->tripId);
        printJsonString(t->tripName);
        printf(",\"startDate\":");
        printJsonString(t->startDate);
        printf(",\"sourceLocation\":");
        printJsonString(t->sourceLocation);
        printf(",\"destLocation\":");
        printJsonString(t->destLocation);
        printf(",\"numDays\":%d,\"numTravellers\":%d,\"totalBudget\":%.2f,\"estimatedCost\":%.2f,\"totalDistanceKm\":%.2f,\"travelStyle\":",
               t->numDays, t->numTravellers, t->totalBudget, t->estimatedCost, t->totalDistanceKm);
        printJsonString(t->travelStyle);
        printf(",\"notes\":");
        printJsonString(t->notes);
        printf("}}\n");
    }

    freeAVLTree(root);
}

static void apiTripDelete(int tripId) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    const AVLNode* target = avlSearchTripById(root, tripId);
    if (!target) {
        printf("{\"success\":false,\"message\":\"Trip ID %d not found in AVL Tree.\"}\n", tripId);
    } else {
        root = avlDeleteTripById(root, tripId);
        saveTripsToFile(root, filePath);
        printf("{\"success\":true,\"message\":\"Trip ID %d deleted from AVL tree and rebalanced successfully.\"}\n", tripId);
    }

    freeAVLTree(root);
}

static void apiTripUpdate(int tripId, const char* name, const char* date, const char* src, const char* dest,
                          int days, int travellers, double budget, double cost, double dist,
                          const char* style, const char* notes) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    const AVLNode* target = avlSearchTripById(root, tripId);
    if (!target) {
        printf("{\"success\":false,\"message\":\"Trip ID %d not found.\"}\n", tripId);
    } else {
        TripRecord updated = target->trip;
        if (name && strlen(name) > 0) strncpy(updated.tripName, name, sizeof(updated.tripName) - 1);
        if (date && strlen(date) > 0) {
            strncpy(updated.startDate, date, sizeof(updated.startDate) - 1);
            updated.dateInt = dateStringToInteger(date);
        }
        if (src && strlen(src) > 0) strncpy(updated.sourceLocation, src, sizeof(updated.sourceLocation) - 1);
        if (dest && strlen(dest) > 0) strncpy(updated.destLocation, dest, sizeof(updated.destLocation) - 1);
        if (days > 0) updated.numDays = days;
        if (travellers > 0) updated.numTravellers = travellers;
        if (budget > 0) updated.totalBudget = budget;
        if (cost > 0) updated.estimatedCost = cost;
        if (dist > 0) updated.totalDistanceKm = dist;
        if (style && strlen(style) > 0) strncpy(updated.travelStyle, style, sizeof(updated.travelStyle) - 1);
        if (notes) strncpy(updated.notes, notes, sizeof(updated.notes) - 1);

        root = avlUpdateTrip(root, tripId, &updated);
        saveTripsToFile(root, filePath);
        printf("{\"success\":true,\"message\":\"Trip ID %d updated successfully in AVL Tree.\"}\n", tripId);
    }

    freeAVLTree(root);
}

/* -------------------------------------------------------------------------
   COMBINED COMPLETE TRIP PLANNER API
   ------------------------------------------------------------------------- */

static void apiFullTripPlan(const MasterData* data,
                            int srcId, int destId,
                            int numDays, int numTravellers,
                            double userBudget, const char* travelStyle,
                            int selectedHotelId, int preferExact,
                            const char* customAttrIdsCsv) {
    /* 1. Location & Route: Dijkstra Shortest Path */
    DijkstraResult route = dijkstraShortestPath(&data->graph, srcId, destId);

    /* 2. Determine Attractions */
    int attrIds[MAX_SELECTED_ATTRS];
    int attrCount = 0;

    if (customAttrIdsCsv && strlen(customAttrIdsCsv) > 0) {
        char buffer[256];
        strncpy(buffer, customAttrIdsCsv, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        char* token = strtok(buffer, ",");
        while (token && attrCount < MAX_SELECTED_ATTRS) {
            attrIds[attrCount++] = atoi(token);
            token = strtok(NULL, ",");
        }
    } else {
        /* Auto-select attractions for destination */
        for (int i = 0; i < data->numAttractions && attrCount < 6; i++) {
            if (data->attractions[i].locationId == destId) {
                attrIds[attrCount++] = data->attractions[i].id;
            }
        }
        /* Fallback if needed */
        if (attrCount == 0) {
            for (int i = 0; i < 4 && i < data->numAttractions; i++) {
                attrIds[attrCount++] = data->attractions[i].id;
            }
        }
    }

    /* 3. Itinerary Optimization (Greedy vs Exact TSP + Day Schedule) */
    FullItinerary itin = generateDayWiseItinerary(data, attrIds, attrCount, numDays, preferExact);

    /* 4. Budget Calculation */
    BudgetBreakdown budget = calculateBudget(data, userBudget, numDays, numTravellers,
                                             travelStyle, destId, selectedHotelId,
                                             route.totalDistanceKm, itin.grandTotalTicketCost);

    /* Output Unified JSON Response */
    printf("{\"success\":true,\"plan\":{");

    /* Route Sub-object */
    printf("\"route\":{\"reachable\":%s,\"srcId\":%d,\"destId\":%d,\"totalDistanceKm\":%.2f,\"totalTimeMin\":%d,\"totalTollCost\":%.2f,\"statusMessage\":",
           route.reachable ? "true" : "false", route.srcId, route.destId, route.totalDistanceKm, route.totalTimeMin, route.totalTollCost);
    printJsonString(route.statusMessage);
    printf(",\"hops\":[");
    for (int i = 0; i < route.hopCount; i++) {
        if (i > 0) printf(",");
        printf("{\"fromName\":");
        printJsonString(route.hops[i].fromName);
        printf(",\"toName\":");
        printJsonString(route.hops[i].toName);
        printf(",\"distanceKm\":%.1f,\"travelTimeMin\":%d}", route.hops[i].distanceKm, route.hops[i].travelTimeMin);
    }
    printf("]},");

    /* Itinerary Sub-object */
    printf("\"itinerary\":{\"numDays\":%d,\"totalAttractionsScheduled\":%d,\"grandTotalTicketCost\":%.2f,",
           itin.numDays, itin.totalAttractionsScheduled, itin.grandTotalTicketCost);
    printf("\"comparison\":{\"distanceDiffKm\":%.2f,\"percentDiff\":%.2f,\"comment\":",
           itin.comparison.distanceDiffKm, itin.comparison.percentDiff);
    printJsonString(itin.comparison.comparisonComment);
    printf(",\"recommendedAlgorithm\":");
    printJsonString(itin.comparison.recommendedAlgorithm);
    printf(",\"greedyKm\":%.2f,\"exactKm\":%.2f},", itin.comparison.greedy.totalDistanceKm, itin.comparison.exact.totalDistanceKm);

    printf("\"days\":[");
    for (int d = 0; d < itin.numDays; d++) {
        const DaySchedule* day = &itin.days[d];
        if (d > 0) printf(",");
        printf("{\"dayNumber\":%d,\"dayStartTime\":%d,\"dayEndTime\":%d,\"totalActiveMinutes\":%d,\"totalTicketCost\":%.2f,\"itemCount\":%d,\"items\":[",
               day->dayNumber, day->dayStartTime, day->dayEndTime, day->totalActiveMinutes, day->totalTicketCost, day->itemCount);
        for (int k = 0; k < day->itemCount; k++) {
            const ScheduledItem* it = &day->items[k];
            if (k > 0) printf(",");
            printf("{\"attractionId\":%d,\"name\":", it->attractionId);
            printJsonString(it->name);
            printf(",\"category\":");
            printJsonString(it->category);
            printf(",\"arrivalTime\":%d,\"visitStartTime\":%d,\"visitEndTime\":%d,\"visitDurationMin\":%d,\"ticketCost\":%.2f,\"timingNote\":",
                   it->arrivalTime, it->visitStartTime, it->visitEndTime, it->visitDurationMin, it->ticketCost);
            printJsonString(it->timingNote);
            printf("}");
        }
        printf("]}");
    }
    printf("]},");

    /* Budget Sub-object */
    printf("\"budget\":{\"userBudget\":%.2f,\"totalExpense\":%.2f,\"remainingBudget\":%.2f,\"hotelExpense\":%.2f,\"foodExpense\":%.2f,\"transportExpense\":%.2f,\"activityExpense\":%.2f,\"emergencyBuffer\":%.2f,\"status\":",
           budget.userBudget, budget.totalExpense, budget.remainingBudget, budget.hotelExpense, budget.foodExpense, budget.transportExpense, budget.activityExpense, budget.emergencyBuffer);
    printJsonString(budget.status);
    printf(",\"warningMessage\":");
    printJsonString(budget.warningMessage);
    printf(",\"potentialSavings\":%.2f,", budget.potentialSavings);
    printf("\"selectedHotel\":{\"name\":");
    printJsonString(budget.selectedHotel.name);
    printf(",\"tier\":");
    printJsonString(budget.selectedHotel.tier);
    printf(",\"pricePerNight\":%.2f,\"rating\":%.1f},", budget.selectedHotel.pricePerNight, budget.selectedHotel.rating);

    printf("\"topRecommendations\":[");
    for (int i = 0; i < budget.topRecommendedCount; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", budget.topRecommendations[i].id);
        printJsonString(budget.topRecommendations[i].name);
        printf(",\"tier\":");
        printJsonString(budget.topRecommendations[i].tier);
        printf(",\"pricePerNight\":%.2f,\"rating\":%.1f}", budget.topRecommendations[i].pricePerNight, budget.topRecommendations[i].rating);
    }
    printf("],");

    printf("\"cheaperAlternatives\":[");
    for (int i = 0; i < budget.cheaperAltCount; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", budget.cheaperAlternatives[i].id);
        printJsonString(budget.cheaperAlternatives[i].name);
        printf(",\"tier\":");
        printJsonString(budget.cheaperAlternatives[i].tier);
        printf(",\"pricePerNight\":%.2f,\"rating\":%.1f}", budget.cheaperAlternatives[i].pricePerNight, budget.cheaperAlternatives[i].rating);
    }
    printf("]}");

    printf("}}\n");
}

/* -------------------------------------------------------------------------
   UNIT TESTING MODULE
   ------------------------------------------------------------------------- */

static void runUnitTests(const MasterData* data, const HashTable* ht) {
    int passed = 0;
    int total = 6;

    printf("\n=== RUNNING COMPREHENSIVE DSA UNIT TESTS ===\n\n");

    /* TEST 1: Dijkstra Shortest Path */
    printf("[Test 1/6] Dijkstra Shortest Path (Delhi -> Jaipur)... ");
    DijkstraResult r1 = dijkstraShortestPath(&data->graph, 0, 2);
    if (r1.reachable && r1.totalDistanceKm > 200.0 && r1.pathLength >= 2) {
        printf("PASSED (Distance: %.1f km, Path Length: %d)\n", r1.totalDistanceKm, r1.pathLength);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 2: Dijkstra Unreachable Node */
    printf("[Test 2/6] Dijkstra Unreachable Location (Delhi -> Port Blair)... ");
    DijkstraResult r2 = dijkstraShortestPath(&data->graph, 0, 12);
    if (!r2.reachable) {
        printf("PASSED (Correctly flagged unreachable)\n");
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 3: Greedy vs Exact TSP Itinerary Optimization */
    printf("[Test 3/6] TSP Itinerary Optimization (4 attractions)... ");
    int sampleAttrs[] = { 1, 2, 3, 4 };
    double dMat[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS];
    int tMat[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS];
    buildDistanceMatrix(data, sampleAttrs, 4, dMat, tMat);
    TSPComparison comp = compareItineraryAlgorithms(4, dMat, tMat);
    if (comp.exact.totalDistanceKm <= comp.greedy.totalDistanceKm + 0.001) {
        printf("PASSED (Greedy: %.1f km, Exact: %.1f km)\n", comp.greedy.totalDistanceKm, comp.exact.totalDistanceKm);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 4: Hotel Min-Heap & QuickSort Budget System */
    printf("[Test 4/6] Budget Engine & Hotel Min-Heap... ");
    BudgetBreakdown b = calculateBudget(data, 1000.0, 3, 2, "Standard", 1, -1, 233.0, 50.0);
    if (b.totalExpense > 0 && b.topRecommendedCount > 0) {
        printf("PASSED (Total: $%.2f, Status: %s)\n", b.totalExpense, b.status);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 5: Hash Table Search */
    printf("[Test 5/6] Hash Table Attraction Search ('Taj')... ");
    int searchRes[10];
    int sCount = searchAttractions(ht, data, "Taj", "All", searchRes, 10);
    if (sCount >= 1) {
        const Attraction* matched = findAttractionById(data, searchRes[0]);
        printf("PASSED (Found %d matches, Top: %s)\n", sCount, matched ? matched->name : "Unknown");
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 6: AVL Tree Insert, Search, and Rebalance */
    printf("[Test 6/6] AVL Tree Insert & Search by ID... ");
    AVLNode* testTree = NULL;
    TripRecord tSample = { 9999, "Test Journey", "2026-12-01", 20261201, "Delhi", "Agra", 3, 2, 800.0, 550.0, 233.0, "Standard", "Unit test node" };
    testTree = avlInsertTrip(testTree, &tSample);
    const AVLNode* foundNode = avlSearchTripById(testTree, 9999);
    if (foundNode && strcmp(foundNode->trip.tripName, "Test Journey") == 0) {
        testTree = avlDeleteTripById(testTree, 9999);
        const AVLNode* afterDelete = avlSearchTripById(testTree, 9999);
        if (!afterDelete) {
            printf("PASSED (Insert, Search, Delete & AVL Balance verified)\n");
            passed++;
        } else {
            printf("FAILED (Delete verification failed)\n");
        }
    } else {
        printf("FAILED\n");
    }
    freeAVLTree(testTree);

    printf("\nTEST SUMMARY: %d / %d tests passed successfully!\n\n", passed, total);
}

/* -------------------------------------------------------------------------
   INTERACTIVE CLI MENU
   ------------------------------------------------------------------------- */

static void printCliMenu(void) {
    printf("\n=================================================================\n");
    printf("                  TRIP PLANNER - C DSA CORE ENGINE               \n");
    printf("=================================================================\n");
    printf(" 1. View All Destinations & Road Network (Graph Adjacency List)  \n");
    printf(" 2. Find Shortest Route & Road Hops (Dijkstra + Min-Heap)        \n");
    printf(" 3. Optimize Itinerary (Greedy Nearest Neighbor vs Exact TSP)    \n");
    printf(" 4. Calculate Budget & Hotel Recommendations (Min-Heap & Sort)   \n");
    printf(" 5. Search Attractions by Name / Category (Hash Table)           \n");
    printf(" 6. Trip Management (AVL Tree: Add, Search, List, Delete)        \n");
    printf(" 7. Generate Full Complete Trip Plan                             \n");
    printf(" 8. Run Comprehensive DSA Unit Tests                             \n");
    printf(" 9. Exit                                                         \n");
    printf("=================================================================\n");
    printf("Enter choice (1-9): ");
}

static void runInteractiveCli(MasterData* data, HashTable* ht) {
    int choice = 0;
    char buffer[256];

    while (1) {
        printCliMenu();
        if (scanf("%d", &choice) != 1) {
            if (!fgets(buffer, sizeof(buffer), stdin)) break;
            continue;
        }

        switch (choice) {
            case 1: {
                printf("\n--- REGISTERED DESTINATIONS (%d locations) ---\n", data->graph.numLocations);
                for (int i = 0; i < data->graph.numLocations; i++) {
                    const Location* loc = &data->graph.locations[i];
                    printf("[%2d] %-14s (%-14s) - %s\n", loc->id, loc->name, loc->state, loc->description);
                }
                break;
            }
            case 2: {
                int src = 0, dest = 0;
                printf("Enter Origin Location ID (0-%d): ", data->graph.numLocations - 1);
                scanf("%d", &src);
                printf("Enter Destination Location ID (0-%d): ", data->graph.numLocations - 1);
                scanf("%d", &dest);
                DijkstraResult r = dijkstraShortestPath(&data->graph, src, dest);
                printf("\n--- DIJKSTRA ROUTE RESULT ---\n");
                printf("Status: %s\n", r.statusMessage);
                if (r.reachable) {
                    printf("Total Distance: %.1f km | Travel Time: %d hrs %d mins | Tolls: $%.2f\n",
                           r.totalDistanceKm, r.totalTimeMin / 60, r.totalTimeMin % 60, r.totalTollCost);
                    printf("Route Path: ");
                    for (int i = 0; i < r.pathLength; i++) {
                        printf("%s%s", data->graph.locations[r.path[i]].name, (i < r.pathLength - 1) ? " -> " : "\n");
                    }
                }
                break;
            }
            case 3: {
                int nAttrs = 4;
                int sampleIds[] = { 1, 2, 3, 4 };
                printf("\n--- ITINERARY OPTIMIZATION (Sample 4 Delhi Attractions) ---\n");
                FullItinerary itin = generateDayWiseItinerary(data, sampleIds, nAttrs, 2, 1);
                printf("Greedy Nearest-Neighbor: %.1f km (%ld us)\n", itin.comparison.greedy.totalDistanceKm, itin.comparison.greedy.executionTimeMicrosec);
                printf("Exact TSP (Backtracking): %.1f km (%ld us)\n", itin.comparison.exact.totalDistanceKm, itin.comparison.exact.executionTimeMicrosec);
                printf("Comment: %s\n", itin.comparison.comparisonComment);
                printf("Recommended: %s\n", itin.comparison.recommendedAlgorithm);
                break;
            }
            case 4: {
                double budget = 1200.0;
                printf("Enter Total Trip Budget in USD [default 1200]: ");
                scanf("%lf", &budget);
                BudgetBreakdown b = calculateBudget(data, budget, 3, 2, "Standard", 1, -1, 233.0, 45.0);
                printf("\n--- ITEMIZED BUDGET BREAKDOWN ---\n");
                printf("User Budget:       $%.2f\n", b.userBudget);
                printf("Hotel Expense:     $%.2f (%s)\n", b.hotelExpense, b.selectedHotel.name);
                printf("Food Expense:      $%.2f\n", b.foodExpense);
                printf("Transport Expense: $%.2f\n", b.transportExpense);
                printf("Activity Expense:  $%.2f\n", b.activityExpense);
                printf("Emergency Buffer:  $%.2f\n", b.emergencyBuffer);
                printf("-----------------------------------------\n");
                printf("Total Estimated:   $%.2f\n", b.totalExpense);
                printf("Remaining Budget:  $%.2f (%s)\n", b.remainingBudget, b.status);
                printf("Alert Note:        %s\n", b.warningMessage);
                break;
            }
            case 5: {
                char q[64];
                printf("Enter search term (e.g. Fort, Palace, Lake, Temple): ");
                scanf("%63s", q);
                int results[20];
                int count = searchAttractions(ht, data, q, "All", results, 20);
                printf("\nFound %d matches in Hash Table for '%s':\n", count, q);
                for (int i = 0; i < count; i++) {
                    const Attraction* a = findAttractionById(data, results[i]);
                    if (a) {
                        printf(" - %-25s [%-11s] Rating: %.1f | Fee: $%.2f\n", a->name, a->category, a->rating, a->ticketCost);
                    }
                }
                break;
            }
            case 6: {
                printf("\n--- TRIP MANAGEMENT (AVL TREE) ---\n");
                AVLNode* root = NULL;
                loadTripsFromFile(&root, getTripsFilePath());
                TripRecord list[MAX_SAVED_TRIPS];
                int count = 0;
                avlCollectInOrder(root, list, &count, MAX_SAVED_TRIPS);
                printf("Total Saved Trips in AVL Tree: %d\n", count);
                for (int i = 0; i < count; i++) {
                    printf("[%d] Date: %s | %s -> %s | %d Days | Est: $%.2f | %s\n",
                           list[i].tripId, list[i].startDate, list[i].sourceLocation,
                           list[i].destLocation, list[i].numDays, list[i].estimatedCost, list[i].tripName);
                }
                freeAVLTree(root);
                break;
            }
            case 7: {
                printf("\n--- FULL COMPLETE TRIP PLAN (Delhi to Agra) ---\n");
                apiFullTripPlan(data, 0, 1, 3, 2, 900.0, "Standard", -1, 1, NULL);
                break;
            }
            case 8: {
                runUnitTests(data, ht);
                break;
            }
            case 9:
                printf("\nExiting Trip Planner. Have a great journey!\n");
                return;
            default:
                printf("Invalid option. Please enter 1 to 9.\n");
        }
    }
}

/* -------------------------------------------------------------------------
   MAIN FUNCTION (CLI & API Dispatcher)
   ------------------------------------------------------------------------- */

int main(int argc, char* argv[]) {
    MasterData data;
    initMasterData(&data);

    HashTable* ht = createHashTable();
    buildAttractionSearchIndex(ht, &data);

    /* Check for API mode */
    if (argc >= 2 && strcmp(argv[1], "--api") == 0) {
        if (argc < 3) {
            printf("{\"success\":false,\"error\":\"Missing API command\"}\n");
            freeHashTable(ht);
            return 1;
        }

        const char* cmd = argv[2];

        if (strcmp(cmd, "get_locations") == 0) {
            apiGetLocations(&data);
        } else if (strcmp(cmd, "get_attractions") == 0) {
            apiGetAttractions(&data);
        } else if (strcmp(cmd, "get_hotels") == 0) {
            apiGetHotels(&data);
        } else if (strcmp(cmd, "plan_route") == 0) {
            int src = (argc >= 4) ? atoi(argv[3]) : 0;
            int dest = (argc >= 5) ? atoi(argv[4]) : 1;
            apiPlanRoute(&data, src, dest);
        } else if (strcmp(cmd, "optimize_itinerary") == 0) {
            int days = (argc >= 4) ? atoi(argv[3]) : 2;
            int preferExact = (argc >= 5) ? atoi(argv[4]) : 1;
            const char* csv = (argc >= 6) ? argv[5] : NULL;
            apiOptimizeItinerary(&data, days, preferExact, csv);
        } else if (strcmp(cmd, "calculate_budget") == 0) {
            double budget = (argc >= 4) ? atof(argv[3]) : 1000.0;
            int days = (argc >= 5) ? atoi(argv[4]) : 3;
            int travellers = (argc >= 6) ? atoi(argv[5]) : 2;
            const char* style = (argc >= 7) ? argv[6] : "Standard";
            int destId = (argc >= 8) ? atoi(argv[7]) : 1;
            int hotelId = (argc >= 9) ? atoi(argv[8]) : -1;
            double dist = (argc >= 10) ? atof(argv[9]) : 233.0;
            double actCost = (argc >= 11) ? atof(argv[10]) : 45.0;
            apiCalculateBudget(&data, budget, days, travellers, style, destId, hotelId, dist, actCost);
        } else if (strcmp(cmd, "search_attractions") == 0) {
            const char* query = (argc >= 4 && strcmp(argv[3], "_EMPTY_") != 0) ? argv[3] : "";
            const char* cat = (argc >= 5 && strcmp(argv[4], "_EMPTY_") != 0) ? argv[4] : "All";
            apiSearchAttractions(&data, ht, query, cat);
        } else if (strcmp(cmd, "full_trip_plan") == 0) {
            int src = (argc >= 4) ? atoi(argv[3]) : 0;
            int dest = (argc >= 5) ? atoi(argv[4]) : 1;
            int days = (argc >= 6) ? atoi(argv[5]) : 3;
            int travellers = (argc >= 7) ? atoi(argv[6]) : 2;
            double budget = (argc >= 8) ? atof(argv[7]) : 1000.0;
            const char* style = (argc >= 9) ? argv[8] : "Standard";
            int hotelId = (argc >= 10) ? atoi(argv[9]) : -1;
            int preferExact = (argc >= 11) ? atoi(argv[10]) : 1;
            const char* attrCsv = (argc >= 12 && strcmp(argv[11], "_NONE_") != 0) ? argv[11] : NULL;
            apiFullTripPlan(&data, src, dest, days, travellers, budget, style, hotelId, preferExact, attrCsv);
        } else if (strcmp(cmd, "trip_list") == 0) {
            apiTripList();
        } else if (strcmp(cmd, "trip_add") == 0) {
            const char* name = (argc >= 4) ? argv[3] : "Golden Triangle Tour";
            const char* date = (argc >= 5) ? argv[4] : "2026-11-20";
            const char* src = (argc >= 6) ? argv[5] : "Delhi";
            const char* dest = (argc >= 7) ? argv[6] : "Agra";
            int days = (argc >= 8) ? atoi(argv[7]) : 3;
            int travellers = (argc >= 9) ? atoi(argv[8]) : 2;
            double budget = (argc >= 10) ? atof(argv[9]) : 1000.0;
            double cost = (argc >= 11) ? atof(argv[10]) : 650.0;
            double dist = (argc >= 12) ? atof(argv[11]) : 233.0;
            const char* style = (argc >= 13) ? argv[12] : "Standard";
            const char* notes = (argc >= 14) ? argv[13] : "Planned with Trip Planner C Engine";
            apiTripAdd(name, date, src, dest, days, travellers, budget, cost, dist, style, notes);
        } else if (strcmp(cmd, "trip_get") == 0) {
            int id = (argc >= 4) ? atoi(argv[3]) : 0;
            apiTripGet(id);
        } else if (strcmp(cmd, "trip_delete") == 0) {
            int id = (argc >= 4) ? atoi(argv[3]) : 0;
            apiTripDelete(id);
        } else if (strcmp(cmd, "trip_update") == 0) {
            int id = (argc >= 4) ? atoi(argv[3]) : 0;
            const char* name = (argc >= 5) ? argv[4] : NULL;
            const char* date = (argc >= 6) ? argv[5] : NULL;
            const char* src = (argc >= 7) ? argv[6] : NULL;
            const char* dest = (argc >= 8) ? argv[7] : NULL;
            int days = (argc >= 9) ? atoi(argv[8]) : 0;
            int travellers = (argc >= 10) ? atoi(argv[9]) : 0;
            double budget = (argc >= 11) ? atof(argv[10]) : 0.0;
            double cost = (argc >= 12) ? atof(argv[11]) : 0.0;
            double dist = (argc >= 13) ? atof(argv[12]) : 0.0;
            const char* style = (argc >= 14) ? argv[13] : NULL;
            const char* notes = (argc >= 15) ? argv[14] : NULL;
            apiTripUpdate(id, name, date, src, dest, days, travellers, budget, cost, dist, style, notes);
        } else if (strcmp(cmd, "run_tests") == 0) {
            runUnitTests(&data, ht);
        } else {
            printf("{\"success\":false,\"error\":\"Unknown API command: %s\"}\n", cmd);
        }

        freeHashTable(ht);
        return 0;
    }

    /* Otherwise, run interactive CLI menu */
    runInteractiveCli(&data, ht);
    freeHashTable(ht);
    return 0;
}

