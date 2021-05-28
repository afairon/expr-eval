#ifndef SYMBOL_H
#define SYMBOL_H

#include <AST.h>
#include <hashtable.h>

hashtable_t *create_symboltable();

void symboltable_set(hashtable_t *hashtable, const char *id, NodeConst *n);
Node *symboltable_get(hashtable_t *hashtable, const char *id);

void free_symboltable(hashtable_t *hashtable);

#endif
