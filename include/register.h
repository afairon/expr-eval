#ifndef REGISTER_H
#define REGISTER_H

#include <stddef.h>

#include <AST.h>

typedef struct
{
	NodeConst *reg;
	size_t size;
} Register;

Register *create_set_registers(size_t n);
void resize_registers(Register *src, size_t n);

void register_load_constant(NodeConst *dst, NodeConst *n);
void register_load_identifier(NodeConst *dst, NodeID *n);
void register_convert_int_to_float(NodeConst *dst, NodeConst *src);
void register_convert_float_to_int(NodeConst *dst, NodeConst *src);

void register_gt(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_gteq(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_lt(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_lteq(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_eq(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_nteq(NodeConst *dst, NodeConst *src1, NodeConst *src2);

void register_addition(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_subtraction(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_multiplication(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_division(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_intdivision(NodeConst *dst, NodeConst *src1, NodeConst *src2);
void register_exponent(NodeConst *dst, NodeConst *src1, NodeConst *src2);

void free_register(Register *r);

#endif
