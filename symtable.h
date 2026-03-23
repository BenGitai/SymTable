/* symtable.h */

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED

#include <stddef.h>

/* The struct of the symbol table */
typedef struct SymTable *SymTable_T;

/* Create a new symbol table
* Returns a pointer to the new symbol table, or NULL if allocation fails */
SymTable_T SymTable_new(void);

/* Free the memory allocated by the symbol table
* Takes in a pointer to the symbol table to be freed (oSymTable) */
void SymTable_free(SymTable_T oSymTable);

/* Return the size of the symbol table
* Takes in a pointer to the symbol table (oSymTable)
* Returns the number of nodes in the table */
size_t SymTable_getLength(SymTable_T oSymTable);

/* Put a value into the table, no duplicates
* Takes in a pointer to the symbol table (oSymTable), the key (pcKey), and the value (pvValue)
* Returns 1 if successful, 0 if failed */
int SymTable_put(SymTable_T oSymTable,
 const char *pcKey, const void *pvValue);

/* Replace a value in the table
* Takes in a pointer to the symbol table (oSymTable), the key (pcKey), and the new value (pvValue)
* Returns a pointer to the old value if successful, NULL if failed */
void *SymTable_replace(SymTable_T oSymTable,
 const char *pcKey, const void *pvValue);

/* Check if a key exists in the table
* Takes in a pointer to the symbol table (oSymTable) and the key (pcKey)
* Returns 1 if the key exists, 0 if it doesn't */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Get the value of a key in the table
* Takes in a pointer to the symbol table (oSymTable) and the key (pcKey)
* Returns a pointer to the value if successful, NULL if failed */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Remove a key from the table
* Takes in a pointer to the symbol table (oSymTable) and the key (pcKey)
* Returns a pointer to the removed value if successful, NULL if failed */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Map a function to each key-value pair in the table
* Takes in a pointer to the symbol table (oSymTable), a function pointer (pfApply), and a pointer to extra data (pvExtra)
* The function pointer should take a key, a value, and the extra data as arguments */
void SymTable_map(SymTable_T oSymTable,
 void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
 const void *pvExtra);

#endif
