#include <stddef.h>
#include <stdlib.h>

#include <stack.h>

stack *create_stack(size_t size)
{
	size_t cap;
	
	stack *s = (stack *)malloc(sizeof(stack));
	if (s == NULL) {
		return NULL;
	}

	if (size > STACK_BLOCK_SIZE) {
		cap = STACK_BLOCK_SIZE * (size / STACK_BLOCK_SIZE + 1);
	} else {
		cap = STACK_BLOCK_SIZE;
	}

	s->data = (int *)calloc(cap, sizeof(int));
	if (s->data == NULL) {
		free(s);
		return NULL;
	}

	s->top = -1;
	s->size = 0;
	s->cap = cap;

	return s;
}

void push(stack *s, int value)
{
	if (s->size == s->cap) {
		int cap = STACK_BLOCK_SIZE * (s->size / STACK_BLOCK_SIZE + 1);
		s->data = (int *)realloc(s->data, cap * sizeof(int));
		s->cap = cap;
	}

	s->top++;
	s->data[s->top] = value;
	s->size++;
}

int pop(stack *s)
{
	if (s->size == 0) {
		return 0;
	}

	s->size--;
	return s->data[s->top--];
}

int empty(stack *s)
{
	if (s->top <= -1) {
		return 1;
	}

	return 0;
}

void free_stack(stack *s)
{
	if (s == NULL) {
		return;
	}

	if (s->data != NULL) {
		free(s->data);
	}

	free(s);
}
