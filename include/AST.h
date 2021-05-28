#ifndef AST_H
#define AST_H

#include <stddef.h>

typedef enum
{
	CONST,
	ID,
	ARITHMETIC_OP,
	LOGICAL_OP,
} NodeType;

typedef enum
{
	TYPE_INTEGER,
	TYPE_FLOAT,
} NUMBERTYPE;

typedef struct NodeConst
{
	union
	{
		double fval;
		int ival;
	} value;
	NUMBERTYPE type;
} NodeConst;

typedef struct NodeID
{
	struct Node *node;
	char *id;
} NodeID;

typedef struct NodeOP
{
	struct Node **nodes;
	struct Node *to_save;
	int nops;
	int op;
	int is_term;
} NodeOP;

typedef struct Node
{
	union
	{
		NodeConst constant;
		NodeID identifier;
		NodeOP operation;
	} N;
	struct Node *next;
	size_t num_registers;
	size_t num_ops;
	NodeType type;
} Node;

Node *create_const_int(int value);
Node *create_const_float(double value);
Node *create_id(char *id, Node *other);
Node *create_op(int op_type, int nops, ...);

void push_back(Node *n1, Node *n2);

void expr(Node *n);

void free_id(NodeID *n);
void free_op(NodeOP *n);
void free_node(Node **n);

#endif
