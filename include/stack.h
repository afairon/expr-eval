#ifndef STACK_H
#define STACK_H

#include <stddef.h>

#define STACK_BLOCK_SIZE 8

typedef struct
{
	int *data;
	int top;
	size_t size;
	size_t cap;
} stack;

stack *create_stack(size_t size);

void push(stack *s, int value);
int pop(stack *s);

int top(stack *s);
int empty(stack *s);

void free_stack(stack *s);

#endif
