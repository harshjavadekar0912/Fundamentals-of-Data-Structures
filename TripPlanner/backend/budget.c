/**
 * budget.c - Expense breakdown, Hotel Min-Heap priority queue,
 * QuickSort implementations, and budget alert system in Indian Rupees (INR ₹).
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#include "budget.h"

/* -------------------------------------------------------------------------
   HOTEL MIN-HEAP IMPLEMENTATION
   ------------------------------------------------------------------------- */

HotelMinHeap* createHotelMinHeap(int capacity) {
    HotelMinHeap* heap = (HotelMinHeap*)malloc(sizeof(HotelMinHeap));
    heap->size = 0;
    heap->capacity = capacity;
    heap->array = (HotelHeapNode*)malloc(capacity * sizeof(HotelHeapNode));
    return heap;
}

void freeHotelMinHeap(HotelMinHeap* heap) {
    if (!heap) return;
    if (heap->array) free(heap->array);
    free(heap);
}

void insertHotelMinHeap(HotelMinHeap* heap, int hotelId, double price) {
    if (!heap || heap->size >= heap->capacity) return;

    int i = heap->size++;
    heap->array[i].hotelId = hotelId;
    heap->array[i].price = price;

    /* Up-heapify */
    while (i > 0 && heap->array[i].price < heap->array[(i - 1) / 2].price) {
        HotelHeapNode temp = heap->array[i];
        heap->array[i] = heap->array[(i - 1) / 2];
        heap->array[(i - 1) / 2] = temp;
        i = (i - 1) / 2;
    }
}

HotelHeapNode extractMinHotel(HotelMinHeap* heap) {
    HotelHeapNode minNode = { -1, 0.0 };
    if (!heap || heap->size <= 0) return minNode;

    minNode = heap->array[0];
    heap->array[0] = heap->array[--heap->size];

    /* Down-heapify */
    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < heap->size && heap->array[left].price < heap->array[smallest].price) {
            smallest = left;
        }
        if (right < heap->size && heap->array[right].price < heap->array[smallest].price) {
            smallest = right;
        }

        if (smallest != i) {
            HotelHeapNode temp = heap->array[i];
            heap->array[i] = heap->array[smallest];
            heap->array[smallest] = temp;
            i = smallest;
        } else {
            break;
        }
    }

    return minNode;
}

/* -------------------------------------------------------------------------
   HOTEL SORTING ALGORITHMS (QuickSort implementations in C)
   ------------------------------------------------------------------------- */

static void swapHotels(Hotel* a, Hotel* b) {
    Hotel temp = *a;
    *a = *b;
    *b = temp;
}

/* QuickSort by Price */
static int partitionByPrice(Hotel* arr, int low, int high, int ascending) {
    double pivot = arr[high].pricePerNight;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        int cond = ascending ? (arr[j].pricePerNight <= pivot) : (arr[j].pricePerNight >= pivot);
        if (cond) {
            i++;
            swapHotels(&arr[i], &arr[j]);
        }
    }
    swapHotels(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void quickSortPrice(Hotel* arr, int low, int high, int ascending) {
    if (low < high) {
        int pi = partitionByPrice(arr, low, high, ascending);
        quickSortPrice(arr, low, pi - 1, ascending);
        quickSortPrice(arr, pi + 1, high, ascending);
    }
}

void sortHotelsByPrice(Hotel* arr, int n, int ascending) {
    if (!arr || n <= 1) return;
    quickSortPrice(arr, 0, n - 1, ascending);
}

/* QuickSort by Rating */
static int partitionByRating(Hotel* arr, int low, int high, int ascending) {
    double pivot = arr[high].rating;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        int cond = ascending ? (arr[j].rating <= pivot) : (arr[j].rating >= pivot);
        if (cond) {
            i++;
            swapHotels(&arr[i], &arr[j]);
        }
    }
    swapHotels(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void quickSortRating(Hotel* arr, int low, int high, int ascending) {
    if (low < high) {
        int pi = partitionByRating(arr, low, high, ascending);
        quickSortRating(arr, low, pi - 1, ascending);
        quickSortRating(arr, pi + 1, high, ascending);
    }
}

void sortHotelsByRating(Hotel* arr, int n, int ascending) {
    if (!arr || n <= 1) return;
    quickSortRating(arr, 0, n - 1, ascending);
}

/* QuickSort by Distance */
static int partitionByDistance(Hotel* arr, int low, int high, int ascending) {
    double pivot = arr[high].distFromCenterKm;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        int cond = ascending ? (arr[j].distFromCenterKm <= pivot) : (arr[j].distFromCenterKm >= pivot);
        if (cond) {
            i++;
            swapHotels(&arr[i], &arr[j]);
        }
    }
    swapHotels(&arr[i + 1], &arr[high]);
    return i + 1;
}

static void quickSortDistance(Hotel* arr, int low, int high, int ascending) {
    if (low < high) {
        int pi = partitionByDistance(arr, low, high, ascending);
        quickSortDistance(arr, low, pi - 1, ascending);
        quickSortDistance(arr, pi + 1, high, ascending);
    }
}

void sortHotelsByDistance(Hotel* arr, int n, int ascending) {
    if (!arr || n <= 1) return;
    quickSortDistance(arr, 0, n - 1, ascending);
}

/* -------------------------------------------------------------------------
   BUDGET & RECOMMENDATION CALCULATION (in Indian Rupees INR ₹)
   ------------------------------------------------------------------------- */

BudgetBreakdown calculateBudget(const MasterData* data,
                                double userBudget,
                                int numDays,
                                int numTravellers,
                                const char* travelStyle,
                                int destLocationId,
                                int selectedHotelId,
                                double routeDistanceKm,
                                double activityTicketTotal) {
    BudgetBreakdown b;
    memset(&b, 0, sizeof(BudgetBreakdown));

    b.userBudget = userBudget;
    b.numDays = (numDays > 0) ? numDays : 1;
    b.numTravellers = (numTravellers > 0) ? numTravellers : 1;
    strncpy(b.travelStyle, (travelStyle && strlen(travelStyle) > 0) ? travelStyle : "Standard", 15);

    int nights = (b.numDays > 1) ? (b.numDays - 1) : 1;
    int rooms = (int)ceil((double)b.numTravellers / 2.0);
    if (rooms < 1) rooms = 1;

    /* Gather candidate hotels for destination */
    Hotel candidates[MAX_HOTELS];
    int candidateCount = 0;

    for (int i = 0; i < data->numHotels; i++) {
        if (destLocationId < 0 || data->hotels[i].locationId == destLocationId) {
            candidates[candidateCount++] = data->hotels[i];
        }
    }
    if (candidateCount == 0) {
        for (int i = 0; i < data->numHotels && i < 6; i++) {
            candidates[candidateCount++] = data->hotels[i];
        }
    }

    /* 1. Min-Heap for cheapest hotels */
    HotelMinHeap* minHeap = createHotelMinHeap(candidateCount + 2);
    for (int i = 0; i < candidateCount; i++) {
        insertHotelMinHeap(minHeap, candidates[i].id, candidates[i].pricePerNight);
    }

    Hotel cheapestHotels[MAX_HOTELS];
    int cheapestCount = 0;
    while (minHeap->size > 0 && cheapestCount < 6) {
        HotelHeapNode node = extractMinHotel(minHeap);
        const Hotel* h = findHotelById(data, node.hotelId);
        if (h) {
            cheapestHotels[cheapestCount++] = *h;
        }
    }
    freeHotelMinHeap(minHeap);

    /* 2. Selected hotel */
    const Hotel* chosenHotel = NULL;
    if (selectedHotelId > 0) {
        chosenHotel = findHotelById(data, selectedHotelId);
    }

    if (!chosenHotel) {
        for (int i = 0; i < candidateCount; i++) {
            if (strcasecmp(candidates[i].tier, b.travelStyle) == 0) {
                chosenHotel = &candidates[i];
                break;
            }
        }
        if (!chosenHotel && candidateCount > 0) {
            chosenHotel = &candidates[0];
        }
    }

    if (chosenHotel) {
        b.selectedHotel = *chosenHotel;
        b.selectedHotelId = chosenHotel->id;
        b.hotelExpense = chosenHotel->pricePerNight * nights * rooms;
    } else {
        b.hotelExpense = 3500.0 * nights * rooms;
    }

    /* 3. Food Expense in INR */
    double dailyFoodRate = 1200.0; /* Standard: ₹1,200/day */
    if (strcasecmp(b.travelStyle, "Budget") == 0) {
        dailyFoodRate = 450.0;     /* Budget: ₹450/day */
    } else if (strcasecmp(b.travelStyle, "Luxury") == 0) {
        dailyFoodRate = 3000.0;    /* Luxury: ₹3,000/day */
    }
    b.foodExpense = dailyFoodRate * b.numTravellers * b.numDays;

    /* 4. Transport Expense in INR */
    double ratePerKm = 22.0;       /* Standard cab: ₹22/km */
    double localDailyTransit = 900.0;
    if (strcasecmp(b.travelStyle, "Budget") == 0) {
        ratePerKm = 12.0;          /* Budget transit: ₹12/km */
        localDailyTransit = 350.0;
    } else if (strcasecmp(b.travelStyle, "Luxury") == 0) {
        ratePerKm = 45.0;          /* Luxury chauffeur: ₹45/km */
        localDailyTransit = 2500.0;
    }
    b.transportExpense = (ratePerKm * routeDistanceKm) + (localDailyTransit * b.numDays);

    /* 5. Activities & Tickets in INR */
    b.activityExpense = activityTicketTotal * b.numTravellers;

    /* 6. Emergency Buffer (7%) */
    double baseSum = b.hotelExpense + b.foodExpense + b.transportExpense + b.activityExpense;
    b.emergencyBuffer = baseSum * 0.07;

    /* 7. Total & Remaining in INR */
    b.totalExpense = baseSum + b.emergencyBuffer;
    b.remainingBudget = b.userBudget - b.totalExpense;

    /* 8. Budget Status & Messages */
    if (b.remainingBudget < 0.0) {
        strncpy(b.status, "Over Budget", 31);
        snprintf(b.warningMessage, sizeof(b.warningMessage),
                 "Notice: Estimated expenditure (₹%.0f) exceeds your budget (₹%.0f) by ₹%.0f. Consider choosing an economical hotel option or adjusting travel style.",
                 b.totalExpense, b.userBudget, fabs(b.remainingBudget));
    } else if (b.remainingBudget < (b.userBudget * 0.15)) {
        strncpy(b.status, "Near Limit", 31);
        snprintf(b.warningMessage, sizeof(b.warningMessage),
                 "Caution: You are close to your budget limit (₹%.0f remaining, %.1f%% buffer). Reserve funds for unexpected personal expenses.",
                 b.remainingBudget, (b.remainingBudget / b.userBudget) * 100.0);
    } else {
        strncpy(b.status, "Within Budget", 31);
        snprintf(b.warningMessage, sizeof(b.warningMessage),
                 "Healthy Budget: ₹%.0f remaining (%.1f%% buffer). Your planned trip is comfortably within budget.",
                 b.remainingBudget, (b.remainingBudget / b.userBudget) * 100.0);
    }

    /* 9. Top 3 Hotel Recommendations */
    Hotel ratingSorted[MAX_HOTELS];
    int sortCount = candidateCount > MAX_HOTELS ? MAX_HOTELS : candidateCount;
    for (int i = 0; i < sortCount; i++) ratingSorted[i] = candidates[i];
    sortHotelsByRating(ratingSorted, sortCount, 0);

    b.topRecommendedCount = 0;
    for (int i = 0; i < sortCount && b.topRecommendedCount < 3; i++) {
        b.topRecommendations[b.topRecommendedCount++] = ratingSorted[i];
    }

    /* 10. Cheaper Alternatives */
    b.cheaperAltCount = 0;
    b.potentialSavings = 0.0;
    if (chosenHotel) {
        for (int i = 0; i < cheapestCount && b.cheaperAltCount < 3; i++) {
            if (cheapestHotels[i].id != chosenHotel->id && cheapestHotels[i].pricePerNight < chosenHotel->pricePerNight) {
                b.cheaperAlternatives[b.cheaperAltCount++] = cheapestHotels[i];
                double diff = (chosenHotel->pricePerNight - cheapestHotels[i].pricePerNight) * nights * rooms;
                if (diff > b.potentialSavings) {
                    b.potentialSavings = diff;
                }
            }
        }
    }

    return b;
}
