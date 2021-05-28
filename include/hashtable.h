#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>

#define DEFAULT_HASHTABLE_SIZE 5

typedef void (*free_func)(void *ptr);

typedef struct entry_t
{
	char *key;
	void (*addr);
	struct entry_t *next;
} entry_t;

typedef struct hashtable_t
{
	entry_t **entries;
	size_t size;
} hashtable_t;

hashtable_t *create_hashtable(size_t size);
entry_t *create_pair(const char *key, void (*addr));

void hashtable_set(hashtable_t *hashtable, const char *key, void (*addr));
void *hashtable_get(hashtable_t *hashtable, const char *key);
void hashtable_delete(hashtable_t *hashtable, const char *key, free_func func);

void free_hashtable(hashtable_t *hashtable, free_func func);
void free_entry(entry_t *entry, free_func func);

#endif
