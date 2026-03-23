/* symtablehash.c */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

/* Resized bucket sizes */
static const size_t AU_BUCKET_COUNTS[] = 
   {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};

/* SymTableList struct used in linked lists */
typedef struct SymTableList *SymTableList_T;

/* Hash function, given by assignment */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

/* Static functions for Nodes */

/* Hash SymTable Stuct */
struct SymTable {
   /* An array of pointers to SymTableNodes */
   struct SymTableList_T *apsBuckets; 
   size_t uBucketCount;
   size_t uLength;
};

/* Each node in the linked list (same as symtablelist.c) */
struct SymTableNode {
    char *pcKey;          /* Must be a defensive copy */
    void *pvValue;        /* The value associated with the key */
    struct SymTableNode *psNextNode;
};

/* The "Head" structure that represents the table itself, used in linked list */
struct SymTableList {
    struct SymTableNode *psFirstNode;
    size_t uLength;
};

/* Static method for searching in the SymTableList
 * Same as in symtablelist.c */
static struct SymTableNode *SymTable_findNode(SymTableList_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrent;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrent = oSymTable->psFirstNode;
         psCurrent != NULL;
         psCurrent = psCurrent->psNextNode) {

        if (strcmp(psCurrent->pcKey, pcKey) == 0) {
            return psCurrent;
        }
    }

    return NULL;
}

/* Static version of linked list functions: 
 * All same implementation as simtablelist.c */

/* New linked list */
static SymTableList_T SymTableList_new(void) {
    SymTableList_T oSymTable;

    oSymTable = (SymTableList_T)malloc(sizeof(struct SymTableList));

    if (oSymTable == NULL) {
        return NULL;
    }

    oSymTable->psFirstNode = NULL;
    oSymTable->uLength = 0;

    return oSymTable;
}

/* Free linked list */
static void SymTableList_free(SymTableList_T oSymTable) {
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(oSymTable != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psNextNode)
    {
        psNextNode = psCurrentNode->psNextNode;

        free((void*)psCurrentNode->pcKey);
        free(psCurrentNode);
    }

    free(oSymTable);
}

/* Put in a linked list */
static int SymTableList_put(SymTableList_T oSymTable, const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNewNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Loop to check for dups */
    if (SymTable_findNode(oSymTable, pcKey) != NULL) {
        return 0;
    }

    /* Assuming no dups */
    psNewNode = malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL) {
        return 0;
    }

    psNewNode->pcKey = malloc(strlen(pcKey) + 1);
    if (psNewNode->pcKey == NULL) {
        free(psNewNode);
        return 0;
    }

    strcpy((char*)psNewNode->pcKey, pcKey);

    psNewNode->pvValue = (void *)pvValue;
    psNewNode->psNextNode = oSymTable->psFirstNode;
    oSymTable->psFirstNode = psNewNode;

    oSymTable->uLength++;

    return 1;
}

/* Replace the value of a node in a linked list */
static void *SymTableList_replace(SymTableList_T oSymTable, const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNode;
    void *pvOldValue;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    psNode = SymTable_findNode(oSymTable, pcKey);
    if (psNode == NULL) {
        return NULL;
    }

    pvOldValue = (void*)psNode->pvValue;

    psNode->pvValue = (void *)pvValue;

    return pvOldValue;
}

/* Check if a linked list contains a key */
static int SymTableList_contains(SymTableList_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    return (SymTable_findNode(oSymTable, pcKey) != NULL);
}

/* Get the value of a node in a linked list */
static void *SymTableList_get(SymTableList_T oSymTable, const char *pcKey) {
    struct SymTableNode *psNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    psNode = SymTable_findNode(oSymTable, pcKey);
    if (psNode == NULL) {
        return NULL;
    }

    return (void *)psNode->pvValue;
}

/* Remove a node from a linked list */
static void *SymTableList_remove(SymTableList_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrent;
    struct SymTableNode *psPrev = NULL;
    void *pvValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (psCurrent = oSymTable->psFirstNode;
            psCurrent != NULL;
            psPrev = psCurrent, psCurrent = psCurrent->psNextNode) {

        if (strcmp(psCurrent->pcKey, pcKey) == 0) {
            pvValue = (void*)psCurrent->pvValue;

            if (psPrev == NULL) {
                oSymTable->psFirstNode = psCurrent->psNextNode;
            }
            else {
                psPrev->psNextNode = psCurrent->psNextNode;
            }

            free((void*)psCurrent->pcKey);
            free(psCurrent);

            oSymTable->uLength--;
            return pvValue;
        }
    }

    return NULL;
}

/* Map an entire linked list */
static void SymTableList_map(SymTableList_T oSymTable,
        void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
        const void *pvExtra) {
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode) {
        (*pfApply)((char *)psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
    }
}

/* Helper function: Resize a hashed Symbol Table */
static void SymTable_resize(SymTable_T oSymTable) {
   size_t uNewBucketCount;
   size_t i;
   size_t uCurrentCountIndex = 0;
   SymTableList_T *apsNewBuckets;

   assert(oSymTable != NULL);

   /* Find the next size */
    for (i = 0; i < 8; i++) {
        if (AU_BUCKET_COUNTS[i] == oSymTable->uBucketCount) {
            uCurrentCountIndex = i;
            break;
        }
    }

   if (uCurrentCountIndex >= 7) return;
   uNewBucketCount = AU_BUCKET_COUNTS[uCurrentCountIndex + 1];

   /* Allocate the new array of buckets */
   apsNewBuckets = (SymTableList_T*)malloc(uNewBucketCount * sizeof(SymTableList_T));
   if (apsNewBuckets == NULL) return;

   for (i = 0; i < uNewBucketCount; i++) {
      apsNewBuckets[i] = SymTableList_new();
      if (apsNewBuckets[i] == NULL) {
         /* Simple cleanup */
         return; 
      }
   }

   /* Rehash existing nodes into the new bucket array */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        SymTableList_T oOldList = oSymTable->apsBuckets[i];
        struct SymTableNode *psCurrent = oOldList->psFirstNode;

        while (psCurrent != NULL) {
            size_t uNewHash = SymTable_hash(psCurrent->pcKey, uNewBucketCount);
            /* Direct insertion into the new list to avoid put/resize recursion */
            SymTableList_put(apsNewBuckets[uNewHash], psCurrent->pcKey, psCurrent->pvValue);
            psCurrent = psCurrent->psNextNode;
        }
        
        /* Free the old list object and its nodes */
        SymTableList_free(oOldList);
    }

   /* Update the original SymTable struct */
   free(oSymTable->apsBuckets);
   oSymTable->apsBuckets = apsNewBuckets;
   oSymTable->uBucketCount = uNewBucketCount;
}

/* Initialize a hash table */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    size_t i = 0;

    /* Allocate memeory for the table */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    if (oSymTable == NULL) {
        return NULL;
    }

    /* Initialize length and bucket number (as given) */
    oSymTable->uLength = 0;
    oSymTable->uBucketCount = AU_BUCKET_COUNTS[0]; 

    /* Allocate memeory for the buckets */
    oSymTable->apsBuckets = (SymTableList_T*)malloc(oSymTable->uBucketCount * sizeof(SymTableList_T));

    /* Failure returns */
    if (oSymTable->apsBuckets == NULL) {
        free(oSymTable);
        return NULL;
    }

    /* Initialize an empty linked list for each bucket */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        oSymTable->apsBuckets[i] = SymTableList_new();

        /* If a list fails to allocate, clean up everything before returning */
        if (oSymTable->apsBuckets[i] == NULL) {
            size_t j;
            for (j = 0; j < i; j++) {
                SymTableList_free(oSymTable->apsBuckets[j]);
            }
            free(oSymTable->apsBuckets);
            free(oSymTable);
            return NULL;
        }
    }

    return oSymTable;
}

/* Free a hash table */
void SymTable_free(SymTable_T oSymTable) {
    size_t i = 0;

    assert(oSymTable != NULL);

    /* Loop through buckets and free each linked list */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        SymTableList_free(oSymTable->apsBuckets[i]);
    }

    /* Free the buckets and the table */
    free(oSymTable->apsBuckets);
    free(oSymTable);
}

/* Return the length of a hash table */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);

    return(oSymTable->uLength);
}

/* Put a key + value into a hash table, return 1 if success, 0 for failure */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t hash;
    int iResult;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Resize the table if necessary using the helper */
    if (oSymTable->uLength >= oSymTable->uBucketCount) {
        SymTable_resize(oSymTable);
    }

    /* Calculate the hash value */
    hash = SymTable_hash(pcKey, oSymTable->uBucketCount);

    /* Put as a node in the associate linked list */
    iResult = SymTableList_put(oSymTable->apsBuckets[hash], pcKey, pvValue);

    if (iResult == 1) {
        oSymTable->uLength++;
    }

    return(iResult);
}

/* Replace the value of a key, return the old value or null if no key */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t hash;
    assert(oSymTable != 0);
    assert(pcKey != 0);

    /* Calculate hash */
    hash = SymTable_hash(pcKey, oSymTable->uBucketCount);

    /* Use the associated linked list */
    return(SymTableList_replace(oSymTable->apsBuckets[hash], pcKey, pvValue));
}

/* Check if a table contains a key, 0 if it doesn't, 1 if it does */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t hash;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Calulate hash */
    hash = SymTable_hash(pcKey, oSymTable->uBucketCount);

    /* Use linked list contains for the associated bucket */
    return(SymTableList_contains(oSymTable->apsBuckets[hash], pcKey));
}

/* Get the value of a key */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t hash;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    hash = SymTable_hash(pcKey, oSymTable->uBucketCount);

    return SymTableList_get(oSymTable->apsBuckets[hash], pcKey);
}

/* Remove a node from the table, return NULL if failed, return the old value if success */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t hash;
    void *vpResult;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Calculate hash */
    hash = SymTable_hash(pcKey, oSymTable->uBucketCount);

    /* Get the result using linked list and hash */
    vpResult = (void *)SymTableList_remove(oSymTable->apsBuckets[hash], pcKey);

    if (vpResult != NULL) {
        oSymTable->uLength--; /* Decrement if something removed */
    }

    return vpResult;
}

/* Apply a map to all values in the table */
void SymTable_map(SymTable_T oSymTable,
        void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
        const void *pvExtra) {
    size_t i = 0;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Loop through each bucket and apply list mapping to each */
    for(i = 0; i < oSymTable->uBucketCount; i++) {
        SymTableList_map(oSymTable->apsBuckets[i], pfApply, pvExtra);
    }
}
