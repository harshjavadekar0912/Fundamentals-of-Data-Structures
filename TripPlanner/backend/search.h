/**
 * search.h - Hash Table data structure for searching attractions by name and category.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "data.h"

#define HASH_TABLE_SIZE 389
#define MAX_SEARCH_RESULTS 80

/* --- Hash Table Node (Separate Chaining for Collision Resolution) --- */
typedef struct HashNode {
    char key[64];              /* Indexed keyword, name token, or category */
    int attractionId;
    struct HashNode* next;
} HashNode;

/* --- Hash Table Structure --- */
typedef struct {
    HashNode* buckets[HASH_TABLE_SIZE];
    int totalEntries;
} HashTable;

/* Function Prototypes */
HashTable* createHashTable(void);
void freeHashTable(HashTable* ht);
unsigned int hashFunction(const char* str);
void insertHashEntry(HashTable* ht, const char* key, int attractionId);
void buildAttractionSearchIndex(HashTable* ht, const MasterData* data);
int searchAttractions(const HashTable* ht,
                       const MasterData* data,
                       const char* query,
                       const char* categoryFilter,
                       int* resultIds,
                       int maxResults);

#endif /* SEARCH_H */

