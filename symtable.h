/* symtable.h */

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED

#include <stddef.h>

/* The struct of the symbol table */
typedef struct SymTable *SymTable_T;

/* Create a new symbol table */
SymTable_T SymTable_new(void);

/* Free the memory allocated by the symbol table */
void SymTable_free(SymTable_T oSymTable);

/* Return the size of the symbol table */
size_t SymTable_getLength(SymTable_T oSymTable);

/* Put a value into the table, no duplicates */
int SymTable_put(SymTable_T oSymTable,
 const char *pcKey, const void *pvValue);

/* Replace a value in the table */
void *SymTable_replace(SymTable_T oSymTable,
 const char *pcKey, const void *pvValue);

/* Check if a key exists in the table */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Get the value of a key in the table */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Remove a key from the table */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Map a function to each key-value pair in the table */
void SymTable_map(SymTable_T oSymTable,
 void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
 const void *pvExtra);

#endif
