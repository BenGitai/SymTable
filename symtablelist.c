/* symtablelist.c */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtable.h"

/* Each node in the linked list */
struct SymTableNode {
    char *pcKey;          /* Must be a defensive copy */
    void *pvValue;        /* The value associated with the key */
    struct SymTableNode *psNextNode;
};

/* The "Head" structure that represents the table itself */
struct SymTable {
    struct SymTableNode *psFirstNode;
    size_t uLength;      
};

/* Static method for searching in the SymTable */
static struct SymTableNode *SymTable_findNode(SymTable_T oSymTable, const char *pcKey) {
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

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;

    oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

    if (oSymTable == NULL) {
        return NULL;
    }

    oSymTable->psFirstNode = NULL;
    oSymTable->uLength = 0;

    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
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

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);

    return oSymTable->uLength;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
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

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
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

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    return (SymTable_findNode(oSymTable, pcKey) != NULL);
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    struct SymTableNode *psNode;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    psNode = SymTable_findNode(oSymTable, pcKey);
    if (psNode == NULL) {
        return NULL;
    }

    return (void *)psNode->pvValue;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
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

void SymTable_map(SymTable_T oSymTable, 
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

