/* symtablelist.c */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

/* Each node in the linked list */
struct SymTableNode {
    /* The key for this node */
    char *pcKey;          /* Must be a defensive copy */
    void *pvValue;        /* The value associated with the key */
    struct SymTableNode *psNextNode; /* Pointer to the next node in the list */
};

/* The Head structure that represents the table itself */
struct SymTable {
    /* The first node in the linked list */
    struct SymTableNode *psFirstNode;
    /* The number of nodes in the list */
    size_t uLength;      
};

/* Static method for searching in the SymTable
* Takes in both the symbol table (oSymTable) and the key to search for (pcKey)
* Returns a pointer to the node if found, NULL otherwise */
static struct SymTableNode *SymTable_findNode(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrent;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Parse through list till you find a matching key */
    for (psCurrent = oSymTable->psFirstNode;
         psCurrent != NULL;
         psCurrent = psCurrent->psNextNode) {
        
        if (strcmp(psCurrent->pcKey, pcKey) == 0) {
            return psCurrent;
        }
    }

    return NULL;
}

/* Initialize new linked list */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    /* Allocate memory for the list as a defensive copy */
    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    if (oSymTable == NULL) {
        return NULL;
    }

    /* Initialize the first node to null and size to 0 */
    oSymTable->psFirstNode = NULL;
    oSymTable->uLength = 0;

    return oSymTable;
}

/* Free the memory allocated by the list and then the list itself */
void SymTable_free(SymTable_T oSymTable) {
    /* Nodes to be able to loop through defensivly */
    struct SymTableNode *psCurrentNode;
    struct SymTableNode *psNextNode;

    assert(oSymTable != NULL);

    /* Loop through list freeing each node */
    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psNextNode)
    {
        psNextNode = psCurrentNode->psNextNode;
        
        free((void*)psCurrentNode->pcKey);
        free(psCurrentNode);
    }

    /* Free the list itself */
    free(oSymTable);
}

/* Return the size of the table */
size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);

    return oSymTable->uLength;
}

/* Put a value into the table, no duplicates */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNewNode;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Loop to check for dups */
    if (SymTable_findNode(oSymTable, pcKey) != NULL) {
        return 0;
    }

    /* Assuming no dups, assign memory for new node */
    psNewNode = malloc(sizeof(struct SymTableNode));
    if (psNewNode == NULL) {
        return 0;
    }

    psNewNode->pcKey = malloc(strlen(pcKey) + 1);
    if (psNewNode->pcKey == NULL) {
        free(psNewNode);
        return 0;
    }

    /* Copy the key value into the node */
    strcpy((char*)psNewNode->pcKey, pcKey);

    /* Set the value, set the new node to start of list */
    psNewNode->pvValue = (void *)pvValue; 
    psNewNode->psNextNode = oSymTable->psFirstNode;
    oSymTable->psFirstNode = psNewNode;

    oSymTable->uLength++;

    return 1; /* Success */
}

/* Replace a value in a table, return old value if success, null if failed */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    struct SymTableNode *psNode;    
    void *pvOldValue;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Use static method to find if the key exists in the list */
    psNode = SymTable_findNode(oSymTable, pcKey);
    if (psNode == NULL) {
        return NULL; /* Return null if it doesn't exist */
    }

    /* Replace the old value with the new one and then return the old value */
    pvOldValue = (void*)psNode->pvValue;

    psNode->pvValue = (void *)pvValue;

    return pvOldValue;
}

/* Check if a key exists in the table, return 1 if it does, 0 if not */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Use the find node static method which does exactly this */
    return (SymTable_findNode(oSymTable, pcKey) != NULL); /* Compare to != NULL to get 0 and 1 */
}

/* Get the value of a key in the list, return the pointer to the value or null if the key doesn't exist */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psNode;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* find the node */
    psNode = SymTable_findNode(oSymTable, pcKey);
    if (psNode == NULL) {
        return NULL;
    }

    return (void *)psNode->pvValue; /* return the value of the node */
}

/* Remove a node from the list, return the removed value */ 
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psCurrent;
    struct SymTableNode *psPrev = NULL;
    void *pvValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Loop thrrough the list while keeping track of the previous node */
    for (psCurrent = oSymTable->psFirstNode;
            psCurrent != NULL;
            psPrev = psCurrent, psCurrent = psCurrent->psNextNode) {

        /* If you find the value, remove it, 
         * set the previous nodes next node to the removed nodes next node */
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

    return NULL; /* Failed to find the node */
}

/* Apply a mapping function to all values in a list */
void SymTable_map(SymTable_T oSymTable, 
        void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
        const void *pvExtra) {
    struct SymTableNode *psCurrentNode;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Apply the mapping to the value of each node */
    for (psCurrentNode = oSymTable->psFirstNode;
            psCurrentNode != NULL;
            psCurrentNode = psCurrentNode->psNextNode) {
        (*pfApply)((char *)psCurrentNode->pcKey, (void*)psCurrentNode->pvValue, (void*)pvExtra);
    }
}

