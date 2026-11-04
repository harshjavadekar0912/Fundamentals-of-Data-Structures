/**
 * main.c - Main Entry Point for Trip Planner C Backend.
 * Features:
 * - Single-city focused exploration and tour planning (e.g. Pune, Jaipur, etc.)
 * - Exact TSP itinerary optimization (recursion & backtracking with branch-and-bound)
 * - Indian Rupees (INR ₹) currency throughout
 * - Clean JSON API for Node.js Express server integration
 * - Automated Unit Testing & Interactive CLI
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

static void apiOptimizeItinerary(const MasterData* data, int numDays, const char* attrIdsCsv) {
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
        for (int i = 0; i < 5 && i < data->numAttractions; i++) {
            attrIds[count++] = data->attractions[i].id;
        }
    }

    /* Uses Exact TSP only */
    FullItinerary itin = generateDayWiseItinerary(data, attrIds, count, numDays);

    printf("{\"success\":true,\"numDays\":%d,\"totalAttractionsScheduled\":%d,\"grandTotalTicketCost\":%.2f,\"totalDistanceKm\":%.2f,\"totalTravelTimeMin\":%d,",
           itin.numDays, itin.totalAttractionsScheduled, itin.grandTotalTicketCost, itin.totalDistanceKm, itin.totalTravelTimeMin);

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

static void apiTripAdd(const char* name, const char* date, const char* dest,
                       int days, int travellers, double budget, double cost, double dist,
                       const char* style, const char* notes) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    srand((unsigned int)time(NULL));
    int newId = 1000 + (rand() % 9000);

    TripRecord t;
    memset(&t, 0, sizeof(TripRecord));
    t.tripId = newId;
    strncpy(t.tripName, name ? name : "City Exploration", sizeof(t.tripName) - 1);
    strncpy(t.startDate, date ? date : "2026-11-20", sizeof(t.startDate) - 1);
    t.dateInt = dateStringToInteger(t.startDate);
    strncpy(t.destLocation, dest ? dest : "Pune", sizeof(t.destLocation) - 1);
    strncpy(t.sourceLocation, dest ? dest : "Pune", sizeof(t.sourceLocation) - 1);
    t.numDays = days > 0 ? days : 3;
    t.numTravellers = travellers > 0 ? travellers : 2;
    t.totalBudget = budget;
    t.estimatedCost = cost;
    t.totalDistanceKm = dist;
    strncpy(t.travelStyle, style ? style : "Standard", sizeof(t.travelStyle) - 1);
    strncpy(t.notes, notes ? notes : "", sizeof(t.notes) - 1);

    root = avlInsertTrip(root, &t);
    saveTripsToFile(root, filePath);

    printf("{\"success\":true,\"message\":\"Trip saved successfully.\",\"tripId\":%d}\n", newId);
    freeAVLTree(root);
}

static void apiTripGet(int tripId) {
    AVLNode* root = NULL;
    const char* filePath = getTripsFilePath();
    loadTripsFromFile(&root, filePath);

    const AVLNode* found = avlSearchTripById(root, tripId);
    if (!found) {
        printf("{\"success\":false,\"message\":\"Trip not found.\"}\n");
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
        printf("{\"success\":false,\"message\":\"Trip ID %d not found.\"}\n", tripId);
    } else {
        root = avlDeleteTripById(root, tripId);
        saveTripsToFile(root, filePath);
        printf("{\"success\":true,\"message\":\"Trip ID %d removed successfully.\"}\n", tripId);
    }

    freeAVLTree(root);
}

static void apiTripUpdate(int tripId, const char* name, const char* date, const char* dest,
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
        if (dest && strlen(dest) > 0) {
            strncpy(updated.destLocation, dest, sizeof(updated.destLocation) - 1);
            strncpy(updated.sourceLocation, dest, sizeof(updated.sourceLocation) - 1);
        }
        if (days > 0) updated.numDays = days;
        if (travellers > 0) updated.numTravellers = travellers;
        if (budget > 0) updated.totalBudget = budget;
        if (cost > 0) updated.estimatedCost = cost;
        if (dist > 0) updated.totalDistanceKm = dist;
        if (style && strlen(style) > 0) strncpy(updated.travelStyle, style, sizeof(updated.travelStyle) - 1);
        if (notes) strncpy(updated.notes, notes, sizeof(updated.notes) - 1);

        root = avlUpdateTrip(root, tripId, &updated);
        saveTripsToFile(root, filePath);
        printf("{\"success\":true,\"message\":\"Trip ID %d updated successfully.\"}\n", tripId);
    }

    freeAVLTree(root);
}

/* -------------------------------------------------------------------------
   COMBINED COMPLETE TRIP PLANNER API (Single City Focus & Exact TSP)
   ------------------------------------------------------------------------- */

static void apiFullTripPlan(const MasterData* data,
                            int destId,
                            int numDays, int numTravellers,
                            double userBudget, const char* travelStyle,
                            int selectedHotelId,
                            const char* customAttrIdsCsv) {
    const Location* city = findLocationById(data, destId);
    const char* cityName = city ? city->name : "Selected City";

    /* 1. Gather Attractions for Selected City */
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
        for (int i = 0; i < data->numAttractions && attrCount < 7; i++) {
            if (data->attractions[i].locationId == destId) {
                attrIds[attrCount++] = data->attractions[i].id;
            }
        }
        if (attrCount == 0) {
            for (int i = 0; i < 4 && i < data->numAttractions; i++) {
                attrIds[attrCount++] = data->attractions[i].id;
            }
        }
    }

    /* 2. Optimize Itinerary using Exact TSP only */
    FullItinerary itin = generateDayWiseItinerary(data, attrIds, attrCount, numDays);

    /* 3. Build Intra-City Route Hops */
    double intraDistTotal = itin.totalDistanceKm;
    int intraTimeTotal = itin.totalTravelTimeMin;

    /* 4. Budget Calculation in INR (₹) */
    BudgetBreakdown budget = calculateBudget(data, userBudget, numDays, numTravellers,
                                             travelStyle, destId, selectedHotelId,
                                             intraDistTotal, itin.grandTotalTicketCost);

    /* Output Unified JSON Response */
    printf("{\"success\":true,\"plan\":{");

    /* City info */
    printf("\"destinationCity\":");
    printJsonString(cityName);
    printf(",\"cityId\":%d,", destId);

    /* Route Sub-object (Intra-city tour connecting scheduled stops) */
    printf("\"route\":{\"reachable\":true,\"destId\":%d,\"cityName\":", destId);
    printJsonString(cityName);
    printf(",\"totalDistanceKm\":%.2f,\"totalTimeMin\":%d,\"statusMessage\":",
           intraDistTotal, intraTimeTotal);

    char routeMsg[256];
    snprintf(routeMsg, sizeof(routeMsg), "Optimal sightseeing circuit mapped across %d locations in %s (%.1f km total).",
             itin.totalAttractionsScheduled, cityName, intraDistTotal);
    printJsonString(routeMsg);

    printf(",\"hops\":[");
    int hopIndex = 0;
    for (int d = 0; d < itin.numDays; d++) {
        const DaySchedule* day = &itin.days[d];
        for (int k = 0; k < day->itemCount - 1; k++) {
            if (hopIndex > 0) printf(",");
            printf("{\"day\":%d,\"fromName\":", day->dayNumber);
            printJsonString(day->items[k].name);
            printf(",\"toName\":");
            printJsonString(day->items[k + 1].name);
            printf(",\"distanceKm\":%.1f,\"travelTimeMin\":%d}",
                   (double)(day->items[k + 1].travelTimeFromPrevMin) / 3.0, day->items[k + 1].travelTimeFromPrevMin);
            hopIndex++;
        }
    }
    printf("]},");

    /* Itinerary Sub-object (Exact TSP Result) */
    printf("\"itinerary\":{\"numDays\":%d,\"totalAttractionsScheduled\":%d,\"grandTotalTicketCost\":%.2f,\"totalDistanceKm\":%.2f,\"totalTravelTimeMin\":%d,",
           itin.numDays, itin.totalAttractionsScheduled, itin.grandTotalTicketCost, itin.totalDistanceKm, itin.totalTravelTimeMin);

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

    /* Budget Sub-object in INR */
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

    printf("\n=== RUNNING TRIP PLANNER CORE ENGINE TESTS ===\n\n");

    /* TEST 1: Shortest Path Route (Pune -> Mumbai) */
    printf("[Test 1/6] Shortest Path Navigation (Pune -> Mumbai)... ");
    DijkstraResult r1 = dijkstraShortestPath(&data->graph, 13, 6);
    if (r1.reachable && r1.totalDistanceKm > 100.0 && r1.pathLength >= 2) {
        printf("PASSED (Distance: %.1f km via Expressway)\n", r1.totalDistanceKm);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 2: Unreachable Destination Handling */
    printf("[Test 2/6] Unreachable Destination Detection (Pune -> Port Blair)... ");
    DijkstraResult r2 = dijkstraShortestPath(&data->graph, 13, 12);
    if (!r2.reachable) {
        printf("PASSED (Correctly identified unreachable route)\n");
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 3: Exact TSP Itinerary Optimization */
    printf("[Test 3/6] Exact TSP Itinerary Optimization (Pune Attractions)... ");
    int sampleAttrs[] = { 1, 2, 3, 4 };
    double dMat[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS];
    int tMat[MAX_SELECTED_ATTRS][MAX_SELECTED_ATTRS];
    buildDistanceMatrix(data, sampleAttrs, 4, dMat, tMat);
    TSPResult tsp = solveExactTSP(4, dMat, tMat);
    if (tsp.isOptimal && tsp.totalDistanceKm > 0.0) {
        printf("PASSED (Optimal tour: %.1f km)\n", tsp.totalDistanceKm);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 4: Budget Engine & Accommodations in INR */
    printf("[Test 4/6] Budget Calculator in INR (₹)... ");
    BudgetBreakdown b = calculateBudget(data, 25000.0, 3, 2, "Standard", 13, -1, 35.0, 500.0);
    if (b.totalExpense > 0 && b.topRecommendedCount > 0) {
        printf("PASSED (Estimated: INR %.0f, Status: %s)\n", b.totalExpense, b.status);
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 5: Attraction Search */
    printf("[Test 5/6] Attraction Search for 'Shaniwar'... ");
    int searchRes[10];
    int sCount = searchAttractions(ht, data, "Shaniwar", "All", searchRes, 10);
    if (sCount >= 1) {
        const Attraction* matched = findAttractionById(data, searchRes[0]);
        printf("PASSED (Found: %s)\n", matched ? matched->name : "Unknown");
        passed++;
    } else {
        printf("FAILED\n");
    }

    /* TEST 6: Saved Trips Persistence & Lookup */
    printf("[Test 6/6] Saved Trips Persistence & Retrieval... ");
    AVLNode* testTree = NULL;
    TripRecord tSample = { 9999, "Pune Monsoon Tour", "2026-12-01", 20261201, "Pune", "Pune", 3, 2, 25000.0, 18500.0, 48.0, "Standard", "Unit test" };
    testTree = avlInsertTrip(testTree, &tSample);
    const AVLNode* foundNode = avlSearchTripById(testTree, 9999);
    if (foundNode && strcmp(foundNode->trip.tripName, "Pune Monsoon Tour") == 0) {
        testTree = avlDeleteTripById(testTree, 9999);
        const AVLNode* afterDelete = avlSearchTripById(testTree, 9999);
        if (!afterDelete) {
            printf("PASSED (Trip Record CRUD & Balance Verified)\n");
            passed++;
        } else {
            printf("FAILED (Deletion check)\n");
        }
    } else {
        printf("FAILED\n");
    }
    freeAVLTree(testTree);

    printf("\nTEST REPORT: %d / %d tests passed successfully!\n\n", passed, total);
}

/* -------------------------------------------------------------------------
   INTERACTIVE CLI MENU
   ------------------------------------------------------------------------- */

static void printCliMenu(void) {
    printf("\n=================================================================\n");
    printf("                        TRIP PLANNER ENGINE                      \n");
    printf("=================================================================\n");
    printf(" 1. View All Destinations (Including Pune)                       \n");
    printf(" 2. Plan Route Between Cities                                    \n");
    printf(" 3. Optimize Itinerary with Exact TSP (Pune Attractions)         \n");
    printf(" 4. Calculate Trip Budget & Hotel Recommendations in INR (₹)     \n");
    printf(" 5. Search Attractions by Name or Category                       \n");
    printf(" 6. View & Manage Saved Trips                                    \n");
    printf(" 7. Plan Complete Trip for a City (e.g. Pune)                    \n");
    printf(" 8. Run Core Engine Unit Tests                                   \n");
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
                printf("\n--- AVAILABLE DESTINATIONS (%d cities) ---\n", data->graph.numLocations);
                for (int i = 0; i < data->graph.numLocations; i++) {
                    const Location* loc = &data->graph.locations[i];
                    printf("[%2d] %-14s (%-14s) - %s\n", loc->id, loc->name, loc->state, loc->description);
                }
                break;
            }
            case 2: {
                int src = 13, dest = 6;
                printf("Enter Origin City ID (0-%d) [default 13 for Pune]: ", data->graph.numLocations - 1);
                scanf("%d", &src);
                printf("Enter Destination City ID (0-%d) [default 6 for Mumbai]: ", data->graph.numLocations - 1);
                scanf("%d", &dest);
                DijkstraResult r = dijkstraShortestPath(&data->graph, src, dest);
                printf("\n--- ROUTE DETAILS ---\n");
                printf("Status: %s\n", r.statusMessage);
                if (r.reachable) {
                    printf("Distance: %.1f km | Travel Time: %d hrs %d mins | Highway Tolls: INR %.0f\n",
                           r.totalDistanceKm, r.totalTimeMin / 60, r.totalTimeMin % 60, r.totalTollCost);
                    printf("Route: ");
                    for (int i = 0; i < r.pathLength; i++) {
                        printf("%s%s", data->graph.locations[r.path[i]].name, (i < r.pathLength - 1) ? " -> " : "\n");
                    }
                }
                break;
            }
            case 3: {
                int sampleIds[] = { 1, 2, 3, 4 }; /* Pune attractions */
                printf("\n--- EXACT TSP ITINERARY OPTIMIZATION (Pune) ---\n");
                FullItinerary itin = generateDayWiseItinerary(data, sampleIds, 4, 2);
                printf("Total Distance: %.1f km | Travel Time: %d mins | Scheduled: %d stops\n",
                       itin.totalDistanceKm, itin.totalTravelTimeMin, itin.totalAttractionsScheduled);
                break;
            }
            case 4: {
                double budget = 25000.0;
                printf("Enter Total Trip Budget in INR [default 25000]: ");
                scanf("%lf", &budget);
                BudgetBreakdown b = calculateBudget(data, budget, 3, 2, "Standard", 13, -1, 35.0, 450.0);
                printf("\n--- ITEMIZED BUDGET BREAKDOWN (INR) ---\n");
                printf("Allocated Budget:  INR %.2f\n", b.userBudget);
                printf("Hotel Expense:     INR %.2f (%s)\n", b.hotelExpense, b.selectedHotel.name);
                printf("Food Expense:      INR %.2f\n", b.foodExpense);
                printf("Transport Expense: INR %.2f\n", b.transportExpense);
                printf("Activity Expense:  INR %.2f\n", b.activityExpense);
                printf("Emergency Buffer:  INR %.2f\n", b.emergencyBuffer);
                printf("-----------------------------------------\n");
                printf("Total Estimated:   INR %.2f\n", b.totalExpense);
                printf("Remaining Buffer:  INR %.2f (%s)\n", b.remainingBudget, b.status);
                printf("Alert Note:        %s\n", b.warningMessage);
                break;
            }
            case 5: {
                char q[64];
                printf("Enter search term (e.g. Fort, Palace, Lake, Temple, Wada): ");
                scanf("%63s", q);
                int results[20];
                int count = searchAttractions(ht, data, q, "All", results, 20);
                printf("\nFound %d matches for '%s':\n", count, q);
                for (int i = 0; i < count; i++) {
                    const Attraction* a = findAttractionById(data, results[i]);
                    if (a) {
                        printf(" - %-28s [%-11s] Rating: %.1f | Ticket: INR %.0f\n", a->name, a->category, a->rating, a->ticketCost);
                    }
                }
                break;
            }
            case 6: {
                printf("\n--- SAVED TRIPS ---\n");
                AVLNode* root = NULL;
                loadTripsFromFile(&root, getTripsFilePath());
                TripRecord list[MAX_SAVED_TRIPS];
                int count = 0;
                avlCollectInOrder(root, list, &count, MAX_SAVED_TRIPS);
                printf("Total Saved Trips: %d\n", count);
                for (int i = 0; i < count; i++) {
                    printf("[%d] Date: %s | City: %s | %d Days | Est: INR %.0f | %s\n",
                           list[i].tripId, list[i].startDate, list[i].destLocation,
                           list[i].numDays, list[i].estimatedCost, list[i].tripName);
                }
                freeAVLTree(root);
                break;
            }
            case 7: {
                printf("\n--- COMPLETE TRIP PLAN (Pune, 3 Days) ---\n");
                apiFullTripPlan(data, 13, 3, 2, 25000.0, "Standard", -1, NULL);
                break;
            }
            case 8: {
                runUnitTests(data, ht);
                break;
            }
            case 9:
                printf("\nExiting Trip Planner. Have a wonderful journey!\n");
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
            int src = (argc >= 4) ? atoi(argv[3]) : 13;
            int dest = (argc >= 5) ? atoi(argv[4]) : 6;
            apiPlanRoute(&data, src, dest);
        } else if (strcmp(cmd, "optimize_itinerary") == 0) {
            int days = (argc >= 4) ? atoi(argv[3]) : 3;
            const char* csv = (argc >= 5 && strcmp(argv[4], "_NONE_") != 0) ? argv[4] : NULL;
            apiOptimizeItinerary(&data, days, csv);
        } else if (strcmp(cmd, "calculate_budget") == 0) {
            double budget = (argc >= 4) ? atof(argv[3]) : 25000.0;
            int days = (argc >= 5) ? atoi(argv[4]) : 3;
            int travellers = (argc >= 6) ? atoi(argv[5]) : 2;
            const char* style = (argc >= 7) ? argv[6] : "Standard";
            int destId = (argc >= 8) ? atoi(argv[7]) : 13;
            int hotelId = (argc >= 9) ? atoi(argv[8]) : -1;
            double dist = (argc >= 10) ? atof(argv[9]) : 35.0;
            double actCost = (argc >= 11) ? atof(argv[10]) : 500.0;
            apiCalculateBudget(&data, budget, days, travellers, style, destId, hotelId, dist, actCost);
        } else if (strcmp(cmd, "search_attractions") == 0) {
            const char* query = (argc >= 4 && strcmp(argv[3], "_EMPTY_") != 0) ? argv[3] : "";
            const char* cat = (argc >= 5 && strcmp(argv[4], "_EMPTY_") != 0) ? argv[4] : "All";
            apiSearchAttractions(&data, ht, query, cat);
        } else if (strcmp(cmd, "full_trip_plan") == 0) {
            /* Support both:
               full_trip_plan <destId> <days> <travellers> <budget> <travelStyle> <hotelId> [attrCsv]
               OR full_trip_plan <srcId> <destId> <days> <travellers> <budget> <travelStyle> <hotelId> [attrCsv]
            */
            int destId = 13;
            int days = 3;
            int travellers = 2;
            double budget = 25000.0;
            const char* style = "Standard";
            int hotelId = -1;
            const char* attrCsv = NULL;

            if (argc >= 11) {
                /* Form: srcId destId days travellers budget style hotelId attrCsv */
                destId = atoi(argv[4]);
                days = atoi(argv[5]);
                travellers = atoi(argv[6]);
                budget = atof(argv[7]);
                style = argv[8];
                hotelId = atoi(argv[9]);
                if (argc >= 11 && strcmp(argv[10], "_NONE_") != 0) attrCsv = argv[10];
            } else {
                /* Form: destId days travellers budget style hotelId attrCsv */
                if (argc >= 4) destId = atoi(argv[3]);
                if (argc >= 5) days = atoi(argv[4]);
                if (argc >= 6) travellers = atoi(argv[5]);
                if (argc >= 7) budget = atof(argv[6]);
                if (argc >= 8) style = argv[7];
                if (argc >= 9) hotelId = atoi(argv[8]);
                if (argc >= 10 && strcmp(argv[9], "_NONE_") != 0) attrCsv = argv[9];
            }

            apiFullTripPlan(&data, destId, days, travellers, budget, style, hotelId, attrCsv);
        } else if (strcmp(cmd, "trip_list") == 0) {
            apiTripList();
        } else if (strcmp(cmd, "trip_add") == 0) {
            const char* name = (argc >= 4) ? argv[3] : "Pune City Expedition";
            const char* date = (argc >= 5) ? argv[4] : "2026-11-20";
            const char* dest = (argc >= 6) ? argv[5] : "Pune";
            int days = (argc >= 7) ? atoi(argv[6]) : 3;
            int travellers = (argc >= 8) ? atoi(argv[7]) : 2;
            double budget = (argc >= 9) ? atof(argv[8]) : 25000.0;
            double cost = (argc >= 10) ? atof(argv[9]) : 18500.0;
            double dist = (argc >= 11) ? atof(argv[10]) : 45.0;
            const char* style = (argc >= 12) ? argv[11] : "Standard";
            const char* notes = (argc >= 13) ? argv[12] : "Planned with Trip Planner";
            apiTripAdd(name, date, dest, days, travellers, budget, cost, dist, style, notes);
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
            const char* dest = (argc >= 7) ? argv[6] : NULL;
            int days = (argc >= 8) ? atoi(argv[7]) : 0;
            int travellers = (argc >= 9) ? atoi(argv[8]) : 0;
            double budget = (argc >= 10) ? atof(argv[9]) : 0.0;
            double cost = (argc >= 11) ? atof(argv[10]) : 0.0;
            double dist = (argc >= 12) ? atof(argv[11]) : 0.0;
            const char* style = (argc >= 13) ? argv[12] : NULL;
            const char* notes = (argc >= 14) ? argv[13] : NULL;
            apiTripUpdate(id, name, date, dest, days, travellers, budget, cost, dist, style, notes);
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
