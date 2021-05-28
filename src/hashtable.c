#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <hashtable.h>

static unsigned long hash(const char *key, size_t size)
{
	unsigned long hash = 5381;
	int c;

	while (c = *key++) {
		hash = ((hash << 5) + hash) + c;
	}

	hash = hash % size;

	return hash;
}

hashtable_t *create_hashtable(size_t size)
{
	hashtable_t *hashtable = (hashtable_t *)malloc(sizeof(hashtable_t));

	hashtable->entries = (entry_t **)calloc(size, sizeof(entry_t *));
	hashtable->size = size;

	return hashtable;
}

entry_t *create_pair(const char *key, void (*addr))
{
	entry_t *entry = (entry_t *)malloc(sizeof(entry_t));
	entry->key = (char *)malloc(strlen(key) + 1);

	strcpy(entry->key, key);
	entry->addr = addr;

	entry->next = NULL;

	return entry;
}

void hashtable_set(hashtable_t *hashtable, const char *key, void (*addr))
{
	entry_t *entry, *prev;
	unsigned long slot = hash(key, hashtable->size);

	entry = hashtable->entries[slot];
	if (entry == NULL) {
		hashtable->entries[slot] = create_pair(key, addr);
		return;
	}

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			entry->addr = addr;
			return;
		}
		prev = entry;
		entry = entry->next;
	}

	prev->next = create_pair(key, addr);
}

void *hashtable_get(hashtable_t *hashtable, const char *key)
{
	entry_t *entry;
	unsigned long slot = hash(key, hashtable->size);

	entry = hashtable->entries[slot];
	if (entry == NULL) {
		return NULL;
	}

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->addr;
		}
		entry = entry->next;
	}

	return NULL;
}

void hashtable_delete(hashtable_t *hashtable, const char *key, free_func func)
{
	entry_t *entry, *prev = NULL;
	unsigned long slot = hash(key, hashtable->size);

	entry = hashtable->entries[slot];
	if (entry == NULL) {
		return;
	}

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			if (prev == NULL) {
				hashtable->entries[slot] = entry->next;
			} else {
				prev->next = entry->next;
			}
			free_entry(entry, func);
			return;
		}
		prev = entry;
		entry = entry->next;
	}
}

void free_hashtable(hashtable_t *hashtable, free_func func)
{
	size_t size = hashtable->size;
	entry_t *entry, *prev;
	for (int i = 0; i < size; i++) {
		entry = hashtable->entries[i];

		if (entry == NULL) {
			continue;
		}

		while (entry != NULL) {
			prev = entry;
			entry = entry->next;
			free_entry(prev, func);
		}
	}

	free(hashtable->entries);
	free(hashtable);
}

void free_entry(entry_t *entry, free_func func)
{
	free(entry->key);
	if (entry->addr != NULL) {
		func(entry->addr);
	}
	free(entry);
}
