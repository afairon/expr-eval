#include <stdlib.h>

#include <hashtable.h>
#include <symbol.h>

hashtable_t *create_symboltable()
{
	return create_hashtable(DEFAULT_HASHTABLE_SIZE);
}

void symboltable_set(hashtable_t *hashtable, const char *id, NodeConst *n)
{
	Node *value;

	if (n->type == TYPE_INTEGER) {
		value = create_const_int(n->value.ival);
	} else {
		value = create_const_float(n->value.fval);
	}

	hashtable_delete(hashtable, id, free);
	hashtable_set(hashtable, id, (void *)value);
}

Node *symboltable_get(hashtable_t *hashtable, const char *id)
{
	Node *value = NULL, *retrieved;

	retrieved = (Node *)hashtable_get(hashtable, id);
	if (retrieved == NULL) {
		return NULL;
	}

	value = (Node *)malloc(sizeof(Node));
	if (value == NULL) {
		return NULL;
	}

	value->type = retrieved->type;
	value->N.constant.type = retrieved->N.constant.type;
	if (value->N.constant.type == TYPE_INTEGER) {
		value->N.constant.value.ival = retrieved->N.constant.value.ival;
	} else {
		value->N.constant.value.fval = retrieved->N.constant.value.fval;
	}

	return value;
}

void free_symboltable(hashtable_t *hashtable)
{
	free_hashtable(hashtable, free);
}
