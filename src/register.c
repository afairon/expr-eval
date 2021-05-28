#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <AST.h>
#include <register.h>
#include <grammar/calc.tab.h>

Register *create_set_registers(size_t n)
{
	Register *registers = (Register *)malloc(sizeof(Register));
	if (registers == NULL) {
		return NULL;
	}

	registers->reg = (NodeConst *)calloc(n, sizeof(NodeConst));
	registers->size = n;

	return registers;
}

void resize_registers(Register *src, size_t n)
{
	src->reg = (NodeConst *)realloc(src->reg, n * sizeof(NodeConst));
	if (src->reg == NULL) {
		src->size = 0;
	} else {
		src->size = n;
	}
}

void register_load_constant(NodeConst *dst, NodeConst *n)
{
	if (n->type == TYPE_INTEGER) {
		dst->type = TYPE_INTEGER;
		dst->value.ival = n->value.ival;
	} else {
		dst->type = TYPE_FLOAT;
		dst->value.fval = n->value.fval;
	}
}

void register_load_identifier(NodeConst *dst, NodeID *n)
{
	if (n->node == NULL) {
		return;
	}

	dst->type = n->node->N.constant.type;
	if (dst->type == TYPE_INTEGER) {
		dst->value.ival = n->node->N.constant.value.ival;
	} else {
		dst->value.fval = n->node->N.constant.value.fval;
	}
}

void register_convert_int_to_float(NodeConst *dst, NodeConst *src)
{
	dst->type = TYPE_FLOAT;
	dst->value.fval = (double)src->value.ival;
}

void register_convert_float_to_int(NodeConst *dst, NodeConst *src)
{
	dst->type = TYPE_INTEGER;
	dst->value.ival = (int)src->value.fval;
}

void register_gt(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval > src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_gteq(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval >= src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_lt(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval < src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_lteq(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval <= src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_eq(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval == src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_nteq(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (src1->value.fval != src2->value.fval) {
		dst->value.ival = 1;
	} else {
		dst->value.ival = 0;
	}
}

void register_addition(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (dst->type == TYPE_INTEGER) {
		dst->value.ival = src1->value.ival + src2->value.ival;
	} else {
		dst->value.fval = src1->value.fval + src2->value.fval;
	}
}

void register_subtraction(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (dst->type == TYPE_INTEGER) {
		dst->value.ival = src1->value.ival - src2->value.ival;
	} else {
		dst->value.fval = src1->value.fval - src2->value.fval;
	}
}

void register_multiplication(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (dst->type == TYPE_INTEGER) {
		dst->value.ival = src1->value.ival * src2->value.ival;
	} else {
		dst->value.fval = src1->value.fval * src2->value.fval;
	}
}

void register_division(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	dst->value.fval = src1->value.fval / src2->value.fval;
}

void register_intdivision(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	dst->value.ival = src1->value.ival / src2->value.ival;
}

void register_exponent(NodeConst *dst, NodeConst *src1, NodeConst *src2)
{
	if (dst->type == TYPE_INTEGER) {
		dst->value.ival = pow((double)src1->value.ival, (double)src2->value.ival);
	} else {
		dst->value.fval = pow(src1->value.fval, src2->value.fval);
	}
}

void free_register(Register *r)
{
	if (r == NULL) {
		return;
	}

	if (r->reg != NULL) {
		free(r->reg);
	}

	free(r);
}
