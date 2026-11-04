/**
 * trip_management.c - AVL Tree (Self-Balancing Binary Search Tree) Implementation.
 * 
 * Part of Trip Planner Web Application
 * Core backend implemented in C (DSA)
 * 
 * ALGORITHM SPECIFICATION:
 * - Data Structure: AVL Tree with height-balance invariant (|balance factor| <= 1).
 * - Rotations: LL (Right Rotate), RR (Left Rotate), LR (Left-Right), RL (Right-Left).
 * - Insertion Time Complexity: O(log N).
 * - Search Time Complexity: O(log N).
 * - Deletion Time Complexity: O(log N).
 * - In-Order Traversal Time Complexity: O(N) producing chronological sorted order.
 * - Space Complexity: O(N) storage with O(log N) recursion depth.
 */

#include "trip_management.h"

/* Helper: max of two integers */
static int maxInt(int a, int b) {
    return (a > b) ? a : b;
}

int dateStringToInteger(const char* dateStr) {
    if (!dateStr || strlen(dateStr) < 8) return 20260101;
    int year = 0, month = 0, day = 0;
    if (sscanf(dateStr, "%d-%d-%d", &year, &month, &day) == 3) {
        return year * 10000 + month * 100 + day;
    }
    return 20260101;
}

int getAVLHeight(const AVLNode* node) {
    if (!node) return 0;
    return node->height;
}

int getAVLBalanceFactor(const AVLNode* node) {
    if (!node) return 0;
    return getAVLHeight(node->left) - getAVLHeight(node->right);
}

/* Compare two trip records: primary key = dateInt, secondary key = tripId */
static int compareTripKeys(int d1, int id1, int d2, int id2) {
    if (d1 < d2) return -1;
    if (d1 > d2) return 1;
    if (id1 < id2) return -1;
    if (id1 > id2) return 1;
    return 0;
}

static AVLNode* createAVLNode(const TripRecord* trip) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->trip = *trip;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/* -------------------------------------------------------------------------
   AVL ROTATIONS
   ------------------------------------------------------------------------- */

/* Right Rotation (LL Case) */
static AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    /* Perform rotation */
    x->right = y;
    y->left = T2;

    /* Update heights */
    y->height = maxInt(getAVLHeight(y->left), getAVLHeight(y->right)) + 1;
    x->height = maxInt(getAVLHeight(x->left), getAVLHeight(x->right)) + 1;

    return x; /* new root of subtree */
}

/* Left Rotation (RR Case) */
static AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    /* Perform rotation */
    y->left = x;
    x->right = T2;

    /* Update heights */
    x->height = maxInt(getAVLHeight(x->left), getAVLHeight(x->right)) + 1;
    y->height = maxInt(getAVLHeight(y->left), getAVLHeight(y->right)) + 1;

    return y; /* new root of subtree */
}

/* -------------------------------------------------------------------------
   AVL INSERTION: O(log N)
   ------------------------------------------------------------------------- */

AVLNode* avlInsertTrip(AVLNode* root, const TripRecord* trip) {
    if (!root) {
        return createAVLNode(trip);
    }

    int cmp = compareTripKeys(trip->dateInt, trip->tripId, root->trip.dateInt, root->trip.tripId);
    if (cmp < 0) {
        root->left = avlInsertTrip(root->left, trip);
    } else if (cmp > 0) {
        root->right = avlInsertTrip(root->right, trip);
    } else {
        /* Duplicate key: update record */
        root->trip = *trip;
        return root;
    }

    /* Update height */
    root->height = maxInt(getAVLHeight(root->left), getAVLHeight(root->right)) + 1;

    /* Check balance factor */
    int balance = getAVLBalanceFactor(root);

    /* LL Case */
    if (balance > 1 && compareTripKeys(trip->dateInt, trip->tripId, root->left->trip.dateInt, root->left->trip.tripId) < 0) {
        return rotateRight(root);
    }

    /* RR Case */
    if (balance < -1 && compareTripKeys(trip->dateInt, trip->tripId, root->right->trip.dateInt, root->right->trip.tripId) > 0) {
        return rotateLeft(root);
    }

    /* LR Case */
    if (balance > 1 && compareTripKeys(trip->dateInt, trip->tripId, root->left->trip.dateInt, root->left->trip.tripId) > 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    /* RL Case */
    if (balance < -1 && compareTripKeys(trip->dateInt, trip->tripId, root->right->trip.dateInt, root->right->trip.tripId) < 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

/* -------------------------------------------------------------------------
   AVL SEARCH OPERATIONS: O(log N)
   ------------------------------------------------------------------------- */

const AVLNode* avlSearchTripByDate(const AVLNode* root, int dateInt) {
    if (!root) return NULL;
    if (root->trip.dateInt == dateInt) {
        return root;
    }
    if (dateInt < root->trip.dateInt) {
        return avlSearchTripByDate(root->left, dateInt);
    }
    return avlSearchTripByDate(root->right, dateInt);
}

const AVLNode* avlSearchTripById(const AVLNode* root, int tripId) {
    if (!root) return NULL;
    if (root->trip.tripId == tripId) return root;

    const AVLNode* leftSearch = avlSearchTripById(root->left, tripId);
    if (leftSearch) return leftSearch;

    return avlSearchTripById(root->right, tripId);
}

/* Helper: find node with minimum value (in-order successor) */
static AVLNode* findMinValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current->left != NULL) {
        current = current->left;
    }
    return current;
}

/* -------------------------------------------------------------------------
   AVL DELETION: O(log N)
   ------------------------------------------------------------------------- */

static AVLNode* avlDeleteNodeRecursive(AVLNode* root, int dateInt, int tripId) {
    if (!root) return NULL;

    int cmp = compareTripKeys(dateInt, tripId, root->trip.dateInt, root->trip.tripId);
    if (cmp < 0) {
        root->left = avlDeleteNodeRecursive(root->left, dateInt, tripId);
    } else if (cmp > 0) {
        root->right = avlDeleteNodeRecursive(root->right, dateInt, tripId);
    } else {
        /* Node with only one child or no child */
        if ((root->left == NULL) || (root->right == NULL)) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) {
                /* No child */
                temp = root;
                root = NULL;
            } else {
                /* One child */
                *root = *temp;
            }
            free(temp);
        } else {
            /* Node with two children: get in-order successor */
            AVLNode* temp = findMinValueNode(root->right);
            root->trip = temp->trip;
            root->right = avlDeleteNodeRecursive(root->right, temp->trip.dateInt, temp->trip.tripId);
        }
    }

    if (!root) return NULL;

    /* Update height */
    root->height = maxInt(getAVLHeight(root->left), getAVLHeight(root->right)) + 1;

    /* Rebalance */
    int balance = getAVLBalanceFactor(root);

    /* LL Case */
    if (balance > 1 && getAVLBalanceFactor(root->left) >= 0) {
        return rotateRight(root);
    }

    /* LR Case */
    if (balance > 1 && getAVLBalanceFactor(root->left) < 0) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }

    /* RR Case */
    if (balance < -1 && getAVLBalanceFactor(root->right) <= 0) {
        return rotateLeft(root);
    }

    /* RL Case */
    if (balance < -1 && getAVLBalanceFactor(root->right) > 0) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }

    return root;
}

AVLNode* avlDeleteTripById(AVLNode* root, int tripId) {
    const AVLNode* target = avlSearchTripById(root, tripId);
    if (!target) return root; /* not found */
    int dInt = target->trip.dateInt;
    return avlDeleteNodeRecursive(root, dInt, tripId);
}

AVLNode* avlUpdateTrip(AVLNode* root, int tripId, const TripRecord* updatedTrip) {
    const AVLNode* target = avlSearchTripById(root, tripId);
    if (!target) return root;

    /* If date changed, delete and reinsert to maintain BST ordering */
    if (target->trip.dateInt != updatedTrip->dateInt) {
        root = avlDeleteTripById(root, tripId);
        root = avlInsertTrip(root, updatedTrip);
    } else {
        /* Update in place */
        AVLNode* mutableTarget = (AVLNode*)target;
        mutableTarget->trip = *updatedTrip;
    }
    return root;
}

/* -------------------------------------------------------------------------
   IN-ORDER TRAVERSAL: O(N) Chronological Sorted Output
   ------------------------------------------------------------------------- */

void avlCollectInOrder(const AVLNode* root, TripRecord* list, int* count, int maxCount) {
    if (!root || *count >= maxCount) return;

    avlCollectInOrder(root->left, list, count, maxCount);
    if (*count < maxCount) {
        list[(*count)++] = root->trip;
    }
    avlCollectInOrder(root->right, list, count, maxCount);
}

void freeAVLTree(AVLNode* root) {
    if (!root) return;
    freeAVLTree(root->left);
    freeAVLTree(root->right);
    free(root);
}

/* -------------------------------------------------------------------------
   FILE PERSISTENCE (PIPE-DELIMITED FORMAT)
   ------------------------------------------------------------------------- */

static void serializeNodeToFile(const AVLNode* node, FILE* fp) {
    if (!node || !fp) return;
    serializeNodeToFile(node->left, fp);

    const TripRecord* t = &node->trip;
    fprintf(fp, "%d|%s|%s|%d|%s|%s|%d|%d|%.2f|%.2f|%.2f|%s|%s\n",
            t->tripId,
            t->tripName,
            t->startDate,
            t->dateInt,
            t->sourceLocation,
            t->destLocation,
            t->numDays,
            t->numTravellers,
            t->totalBudget,
            t->estimatedCost,
            t->totalDistanceKm,
            t->travelStyle,
            t->notes);

    serializeNodeToFile(node->right, fp);
}

int saveTripsToFile(const AVLNode* root, const char* filename) {
    if (!filename) return 0;
    FILE* fp = fopen(filename, "w");
    if (!fp) return 0;

    serializeNodeToFile(root, fp);
    fclose(fp);
    return 1;
}

int loadTripsFromFile(AVLNode** root, const char* filename) {
    if (!root || !filename) return 0;
    FILE* fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[1024];
    int loaded = 0;
    while (fgets(line, sizeof(line), fp)) {
        /* Strip newline */
        size_t len = strlen(line);
        if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
        }
        if (strlen(line) == 0) continue;

        TripRecord t;
        memset(&t, 0, sizeof(TripRecord));

        char* token = strtok(line, "|");
        if (token) t.tripId = atoi(token);
        token = strtok(NULL, "|");
        if (token) strncpy(t.tripName, token, sizeof(t.tripName) - 1);
        token = strtok(NULL, "|");
        if (token) strncpy(t.startDate, token, sizeof(t.startDate) - 1);
        token = strtok(NULL, "|");
        if (token) t.dateInt = atoi(token);
        token = strtok(NULL, "|");
        if (token) strncpy(t.sourceLocation, token, sizeof(t.sourceLocation) - 1);
        token = strtok(NULL, "|");
        if (token) strncpy(t.destLocation, token, sizeof(t.destLocation) - 1);
        token = strtok(NULL, "|");
        if (token) t.numDays = atoi(token);
        token = strtok(NULL, "|");
        if (token) t.numTravellers = atoi(token);
        token = strtok(NULL, "|");
        if (token) t.totalBudget = atof(token);
        token = strtok(NULL, "|");
        if (token) t.estimatedCost = atof(token);
        token = strtok(NULL, "|");
        if (token) t.totalDistanceKm = atof(token);
        token = strtok(NULL, "|");
        if (token) strncpy(t.travelStyle, token, sizeof(t.travelStyle) - 1);
        token = strtok(NULL, "|");
        if (token) strncpy(t.notes, token, sizeof(t.notes) - 1);

        if (t.tripId > 0) {
            *root = avlInsertTrip(*root, &t);
            loaded++;
        }
    }

    fclose(fp);
    return loaded;
}

