#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdio.h>

#include <AST.h>

void emit(FILE *out, Node *n);
int evaluate(FILE *out, Node *n);
void handle_operation(FILE *out, int dst, int src1, int src2, NodeOP *n);

void emit_load_constant(FILE *out, int dst, NodeConst *n);
void emit_load_identifier(FILE *out, int dst, NodeID *n);
void emit_store_register_to_identifier(FILE *out, const char *dst, int src);
void emit_convert_int_to_float(FILE *out, int dst, int src);
void emit_convert_float_to_int(FILE *out, int dst, int src);

void emit_gt(FILE *out, int dst, int src1, int src2);
void emit_gteq(FILE *out, int dst, int src1, int src2);
void emit_lt(FILE *out, int dst, int src1, int src2);
void emit_lteq(FILE *out, int dst, int src1, int src2);
void emit_eq(FILE *out, int dst, int src1, int src2);
void emit_nteq(FILE *out, int dst, int src1, int src2);

void emit_addition(FILE *out, int dst, int src1, int src2);
void emit_subtraction(FILE *out, int dst, int src1, int src2);
void emit_multiplication(FILE *out, int dst, int src1, int src2);
void emit_division(FILE *out, int dst, int src1, int src2);
void emit_intdivision(FILE *out, int dst, int src1, int src2);
void emit_exponent(FILE *out, int dst, int src1, int src2);

void free_translation();

#endif
