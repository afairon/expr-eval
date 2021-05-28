#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <AST.h>
#include <grammar/calc.tab.h>

Node *create_const_int(int value)
{
	Node *n = (Node *)malloc(sizeof(Node));
	if (n == NULL) {
		return NULL;
	}

	n->next = NULL;
	n->type = CONST;
	n->num_registers = 1;
	n->num_ops = 0;
	n->N.constant.type = TYPE_INTEGER;
	n->N.constant.value.ival = value;

	return n;
}

Node *create_const_float(double value)
{
	Node *n = (Node *)malloc(sizeof(Node));
	if (n == NULL) {
		return NULL;
	}

	n->next = NULL;
	n->type = CONST;
	n->num_registers = 1;
	n->num_ops = 0;
	n->N.constant.type = TYPE_FLOAT;
	n->N.constant.value.fval = value;

	return n;
}

Node *create_id(char *id, Node *other)
{
	Node *n = (Node *)malloc(sizeof(Node));
	if (n == NULL) {
		return NULL;
	}

	n->N.identifier.id = (char *)malloc((strlen(id) + 1) * sizeof(char));
	if (n->N.identifier.id == NULL) {
		free(n);
		return NULL;
	}

	n->next = NULL;
	n->type = ID;
	n->num_registers = 0;
	if (other != NULL) {
		n->num_registers = other->num_registers;
	}
	n->num_ops = 0;
	strcpy(n->N.identifier.id, id);
	n->N.identifier.node = other;

	return n;
}

Node *create_op(int op_type, int nops, ...)
{
	int i;
	va_list ap;
	Node *n = (Node *)malloc(sizeof(Node));
	if (n == NULL) {
		return NULL;
	}

	n->N.operation.nodes = (struct Node **)malloc(nops * sizeof(struct Node *));
	if (n->N.operation.nodes == NULL) {
		free(n);
		return NULL;
	}

	n->next = NULL;
	n->num_ops = 1;
	n->N.operation.op = op_type;
	n->N.operation.nops = nops;
	n->N.operation.is_term = 0;
	n->N.operation.to_save = NULL;

	switch (op_type)
	{
		case GT:
		case GTEQ:
		case LT:
		case LTEQ:
		case EQ:
		case NTEQ:
		{
			n->type = LOGICAL_OP;
			break;
		}
		
		default:
			n->type = ARITHMETIC_OP;
			break;
	}

	bool same = false;
	va_start(ap, nops);
	for (i = 0; i < nops; i++) {
		n->N.operation.nodes[i] = va_arg(ap, struct Node *);
		if (i > 0 && n->num_registers == n->N.operation.nodes[i]->num_registers) {
			same = true;
		}
		if (i == 0 || (i > 0 && n->num_registers < n->N.operation.nodes[i]->num_registers)) {
			n->num_registers = n->N.operation.nodes[i]->num_registers;
			same = false;
		}
		n->num_ops = n->num_ops + n->N.operation.nodes[i]->num_ops;

		if (n->type != LOGICAL_OP) {
			continue;
		}

		if (n->N.operation.nodes[i]->type == LOGICAL_OP && !n->N.operation.nodes[i]->N.operation.is_term) {
			if (!(i % 2)) {
				n->N.operation.nodes[i]->N.operation.to_save = n->N.operation.nodes[i]->N.operation.nodes[1];
			} else {
				n->N.operation.nodes[i]->N.operation.to_save = n->N.operation.nodes[i]->N.operation.nodes[0];
			}
		}
	}

	if (same == true || nops == 1) {
		n->num_registers++;
	}

	return n;
}

void push_back(Node *n1, Node *n2)
{
	n1->next = n2;
}

static void free_next_node(Node *n)
{
	if (n == NULL) {
		return;
	}

	if (n->type == ID) {
		free_id(&n->N.identifier);
	}

	if (n->type == ARITHMETIC_OP || n->type == LOGICAL_OP) {
		free_op(&n->N.operation);
	}
	
	free(n);
}

void free_id(NodeID *n)
{
	free_next_node(n->node);
	free(n->id);
}

void free_op(NodeOP *n)
{
	int nops = n->nops;

	for (int i = 0; i < nops; i++) {
		free_next_node(n->nodes[i]);
	}

	free(n->nodes);
}

void free_node(Node **n)
{
	Node *next = *n, *temp;

	while (next != NULL) {
		temp = next;
		next = next->next;
		free_next_node(temp);
	}

	*n = NULL;
}
