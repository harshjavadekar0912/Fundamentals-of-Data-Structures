/**
 * search.c - Hash Table implementation with Separate Chaining for rapid attraction search.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 * 
 * ALGORITHM SPECIFICATION:
 * - Data Structure: Hash Table with Separate Chaining (Linked Lists).
 * - Hash Function: djb2 polynomial hash (low collision rate).
 * - Average Search Time Complexity: O(1) for keyword lookup + O(K) for category match.
 * - Worst-case Search Time Complexity: O(N) in case of extreme bucket collisions.
 * - Space Complexity: O(E) where E = total indexed tokens.
 */

#include "search.h"
#include <ctype.h>

/* Helper: string to lower case copy */
static void strToLower(const char* src, char* dest, size_t destSize) {
    size_t i = 0;
    while (src[i] && i < destSize - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\0';
}

/* -------------------------------------------------------------------------
   HASH FUNCTION (djb2 algorithm)
   ------------------------------------------------------------------------- */

unsigned int hashFunction(const char* str) {
    if (!str) return 0;
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + tolower(c); /* hash * 33 + c */
    }
    return (unsigned int)(hash % HASH_TABLE_SIZE);
}

/* -------------------------------------------------------------------------
   HASH TABLE INITIALIZATION & MEMORY MANAGEMENT
   ------------------------------------------------------------------------- */

HashTable* createHashTable(void) {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    ht->totalEntries = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    return ht;
}

void freeHashTable(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode* curr = ht->buckets[i];
        while (curr) {
            HashNode* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(ht);
}

void insertHashEntry(HashTable* ht, const char* key, int attractionId) {
    if (!ht || !key || strlen(key) == 0) return;

    char normKey[64];
    strToLower(key, normKey, sizeof(normKey));
    unsigned int bucket = hashFunction(normKey);

    /* Check for duplicate in bucket */
    HashNode* curr = ht->buckets[bucket];
    while (curr) {
        if (curr->attractionId == attractionId && strcmp(curr->key, normKey) == 0) {
            return; /* already present */
        }
        curr = curr->next;
    }

    /* Insert at head of separate chain: O(1) */
    HashNode* newNode = (HashNode*)malloc(sizeof(HashNode));
    strncpy(newNode->key, normKey, sizeof(newNode->key) - 1);
    newNode->key[sizeof(newNode->key) - 1] = '\0';
    newNode->attractionId = attractionId;
    newNode->next = ht->buckets[bucket];
    ht->buckets[bucket] = newNode;
    ht->totalEntries++;
}

/* -------------------------------------------------------------------------
   SEARCH INDEX BUILDER
   ------------------------------------------------------------------------- */

void buildAttractionSearchIndex(HashTable* ht, const MasterData* data) {
    if (!ht || !data) return;

    for (int i = 0; i < data->numAttractions; i++) {
        const Attraction* a = &data->attractions[i];

        /* 1. Index full attraction name */
        insertHashEntry(ht, a->name, a->id);

        /* 2. Index individual word tokens */
        char buffer[MAX_NAME_LEN];
        strncpy(buffer, a->name, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        char* token = strtok(buffer, " ,.-'()&");
        while (token != NULL) {
            if (strlen(token) >= 2) {
                insertHashEntry(ht, token, a->id);
            }
            token = strtok(NULL, " ,.-'()&");
        }

        /* 3. Index Category */
        insertHashEntry(ht, a->category, a->id);

        /* 4. Index City/Location Name */
        const Location* loc = findLocationById(data, a->locationId);
        if (loc) {
            insertHashEntry(ht, loc->name, a->id);
        }
    }
}

/* -------------------------------------------------------------------------
   SEARCH QUERY EXECUTION
   ------------------------------------------------------------------------- */

static int isAlreadyInResults(const int* arr, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (arr[i] == id) return 1;
    }
    return 0;
}

int searchAttractions(const HashTable* ht,
                      const MasterData* data,
                      const char* query,
                      const char* categoryFilter,
                      int* resultIds,
                      int maxResults) {
    if (!ht || !data || !resultIds || maxResults <= 0) return 0;

    int matchCount = 0;
    int hasQuery = (query && strlen(query) > 0);
    int hasCategory = (categoryFilter && strlen(categoryFilter) > 0 && strcasecmp(categoryFilter, "All") != 0);

    char normQuery[64];
    if (hasQuery) {
        strToLower(query, normQuery, sizeof(normQuery));
    }

    /* CASE 1: Query provided -> use Hash Table bucket lookup */
    if (hasQuery) {
        unsigned int bucket = hashFunction(normQuery);
        HashNode* curr = ht->buckets[bucket];

        /* Direct exact token match in Hash Table */
        while (curr && matchCount < maxResults) {
            if (strstr(curr->key, normQuery) != NULL) {
                const Attraction* a = findAttractionById(data, curr->attractionId);
                if (a) {
                    if (!hasCategory || strcasecmp(a->category, categoryFilter) == 0) {
                        if (!isAlreadyInResults(resultIds, matchCount, a->id)) {
                            resultIds[matchCount++] = a->id;
                        }
                    }
                }
            }
            curr = curr->next;
        }

        /* Also check word substring scan over all attractions if matches are few */
        for (int i = 0; i < data->numAttractions && matchCount < maxResults; i++) {
            const Attraction* a = &data->attractions[i];
            char lowerName[MAX_NAME_LEN];
            strToLower(a->name, lowerName, sizeof(lowerName));

            if (strstr(lowerName, normQuery) != NULL) {
                if (!hasCategory || strcasecmp(a->category, categoryFilter) == 0) {
                    if (!isAlreadyInResults(resultIds, matchCount, a->id)) {
                        resultIds[matchCount++] = a->id;
                    }
                }
            }
        }
    }
    /* CASE 2: No query, only category filter -> use Category Hash Bucket or scan */
    else if (hasCategory) {
        unsigned int catBucket = hashFunction(categoryFilter);
        HashNode* curr = ht->buckets[catBucket];

        while (curr && matchCount < maxResults) {
            if (strcasecmp(curr->key, categoryFilter) == 0) {
                const Attraction* a = findAttractionById(data, curr->attractionId);
                if (a && !isAlreadyInResults(resultIds, matchCount, a->id)) {
                    resultIds[matchCount++] = a->id;
                }
            }
            curr = curr->next;
        }
    }
    /* CASE 3: Neither query nor category -> return all attractions up to maxResults */
    else {
        for (int i = 0; i < data->numAttractions && matchCount < maxResults; i++) {
            resultIds[matchCount++] = data->attractions[i].id;
        }
    }

    return matchCount;
}

