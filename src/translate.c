#include <stdio.h>
#include <stddef.h>

#include <AST.h>
#include <hashtable.h>
#include <symbol.h>
#include <register.h>
#include <stack.h>
#include <translate.h>
#include <grammar/calc.tab.h>

extern hashtable_t *symbols;

Register *registers = NULL;
stack *stack_registers = NULL;
stack *stack_available_temporaries = NULL;
stack *stack_used_temporaries = NULL;

char addr[64];
char addr_print[] = "print";

#ifdef DEBUG
static void print_registers(FILE *out)
{
	for (int i = 0; i < registers->size; i++) {
		if (registers->reg[i].type == TYPE_INTEGER) {
			fprintf(out, "[%d] ", registers->reg[i].value.ival);
		} else {
			fprintf(out, "[%lf] ", registers->reg[i].value.fval);
		}
	}
	printf("\n");
}
#endif

static void prepare(Node *n)
{
	size_t num_registers = n->num_registers;
	size_t num_ops = n->num_ops;

	if (registers == NULL) {
		registers = create_set_registers(num_registers);
	}

	size_t size_registers = registers->size;

	if (size_registers < num_registers) {
		resize_registers(registers, num_registers);
		size_registers = registers->size;
	}

	if (stack_registers == NULL) {
		stack_registers = create_stack(num_registers);
	} else {
		while (!empty(stack_registers)) {
			pop(stack_registers);
		}
	}

	for (int i = size_registers; i > 0; i--) {
		push(stack_registers, i-1);
	}

	if (stack_available_temporaries == NULL) {
		stack_available_temporaries = create_stack(num_ops);
	} else {
		while (!empty(stack_available_temporaries)) {
			pop(stack_available_temporaries);
		}
	}

	for (int i = 2 * num_ops; i > 0; i--) {
		push(stack_available_temporaries, i-1);
	}

	if (stack_used_temporaries == NULL) {
		stack_used_temporaries = create_stack(num_ops);
	} else {
		while (!empty(stack_used_temporaries)) {
			pop(stack_used_temporaries);
		}
	}
}

void emit(FILE *out, Node *n)
{
	int ret;
	
	prepare(n);
	
	switch (n->type)
	{
		case CONST:
		case ARITHMETIC_OP:
		case LOGICAL_OP:
		{
			ret = evaluate(out, n);
			emit_store_register_to_identifier(out, addr_print, ret);
			break;
		}

		case ID:
		{
			ret = evaluate(out, n->N.identifier.node);
			symboltable_set(symbols, n->N.identifier.id, &registers->reg[ret]);
			emit_store_register_to_identifier(out, n->N.identifier.id, ret);
			break;
		}

		default:
			break;
	}
}

int evaluate(FILE *out, Node *n)
{
	int ret = 0;

	if (n == NULL) {
		return ret;
	}

	switch (n->type)
	{
		case CONST:
		{
			if (!empty(stack_registers)) {
				ret = pop(stack_registers);
				register_load_constant(&registers->reg[ret], &n->N.constant);
				emit_load_constant(out, ret, &n->N.constant);
#ifdef DEBUG
				print_registers(out);
#endif
			}
			break;
		}

		case ID:
		{
			if (!empty(stack_registers)) {
				ret = pop(stack_registers);
				register_load_identifier(&registers->reg[ret], &n->N.identifier);
				emit_load_identifier(out, ret, &n->N.identifier);
#ifdef DEBUG
				print_registers(out);
#endif
			}
			break;
		}

		case ARITHMETIC_OP:
		{
			int l, r;
			if (n->N.operation.nops == 1) {
				l = evaluate(out, n->N.operation.nodes[0]);
			} else if (n->N.operation.nodes[0]->num_registers >= n->N.operation.nodes[1]->num_registers) {
				l = evaluate(out, n->N.operation.nodes[0]);
				r = evaluate(out, n->N.operation.nodes[1]);
			} else {
				r = evaluate(out, n->N.operation.nodes[1]);
				l = evaluate(out, n->N.operation.nodes[0]);
			}

			ret = l;
			if (n->N.operation.nops > 1) {
				push(stack_registers, r);	
			}

			handle_operation(out, ret, l, r, &n->N.operation);

#ifdef DEBUG
			print_registers(out);
#endif
			break;
		}

		case LOGICAL_OP:
		{
			int l, r;
			if (n->N.operation.nodes[0]->num_registers >= n->N.operation.nodes[1]->num_registers) {
				l = evaluate(out, n->N.operation.nodes[0]);
				r = evaluate(out, n->N.operation.nodes[1]);
				if (n->N.operation.nodes[0]->type == LOGICAL_OP && n->N.operation.nodes[0]->N.operation.to_save != NULL) {
					int truth, last_result;
					truth = pop(stack_used_temporaries);
					last_result = pop(stack_used_temporaries);
					sprintf(addr, "T_%d", last_result);
					NodeID identifier;
					identifier.id = addr;
					identifier.node = symboltable_get(symbols, addr);
					register_load_identifier(&registers->reg[l], &identifier);
					emit_load_identifier(out, l, &identifier);
					free_node(&identifier.node);
					push(stack_available_temporaries, last_result);
					push(stack_used_temporaries, truth);
#ifdef DEBUG
					print_registers(out);
#endif
				}
			} else {
				r = evaluate(out, n->N.operation.nodes[1]);
				l = evaluate(out, n->N.operation.nodes[0]);
				if (n->N.operation.nodes[1]->type == LOGICAL_OP && n->N.operation.nodes[1]->N.operation.to_save != NULL) {
					int truth, last_result;
					truth = pop(stack_used_temporaries);
					last_result = pop(stack_used_temporaries);
					sprintf(addr, "T_%d", last_result);
					NodeID identifier;
					identifier.id = addr;
					identifier.node = symboltable_get(symbols, addr);
					register_load_identifier(&registers->reg[r], &identifier);
					emit_load_identifier(out, r, &identifier);
					free_node(&identifier.node);
					push(stack_available_temporaries, last_result);
					push(stack_used_temporaries, truth);
#ifdef DEBUG
					print_registers(out);
#endif
				}
			}

			ret = l;
			if (n->N.operation.nops > 1) {
				push(stack_registers, r);	
			}

			// Save expression before doing any operation
			if (n->N.operation.to_save != NULL) {
#ifdef DEBUG
				printf("0=%p 1=%p to_save=%p\n", n->N.operation.nodes[0], n->N.operation.nodes[1], n->N.operation.to_save);
#endif
				int temp = pop(stack_available_temporaries);
				push(stack_used_temporaries, temp);
				sprintf(addr, "T_%d", temp);
				if (n->N.operation.to_save == n->N.operation.nodes[0]) {
					symboltable_set(symbols, addr, &registers->reg[l]);
					emit_store_register_to_identifier(out, addr, l);
				} else {
					symboltable_set(symbols, addr, &registers->reg[r]);
					emit_store_register_to_identifier(out, addr, r);
				}
			}

			handle_operation(out, ret, l, r, &n->N.operation);

			// Save truth value after operation
			if (n->N.operation.to_save != NULL) {
#ifdef DEBUG
				printf("0=%p 1=%p to_save=%p\n", n->N.operation.nodes[0], n->N.operation.nodes[1], n->N.operation.to_save);
#endif
				int temp = pop(stack_available_temporaries);
				push(stack_used_temporaries, temp);
				sprintf(addr, "T_%d", temp);
				symboltable_set(symbols, addr, &registers->reg[l]);
				emit_store_register_to_identifier(out, addr, l);
			}

			// AND operation with all truth value
			if (n->N.operation.to_save == NULL && !n->N.operation.is_term) {
				while (!empty(stack_used_temporaries)) {
					int truth;
					truth = pop(stack_used_temporaries);
					sprintf(addr, "T_%d", truth);
					NodeID identifier;
					identifier.id = addr;
					identifier.node = symboltable_get(symbols, addr);

					register_load_identifier(&registers->reg[r], &identifier);
					emit_load_identifier(out, r, &identifier);
					free_node(&identifier.node);
#ifdef DEBUG
					print_registers(out);
#endif
					NodeOP op;
					op.op = ADDITION;
					handle_operation(out, l, l, r, &op);

					NodeConst num;
					num.type = TYPE_FLOAT;
					num.value.fval = 2.0;
					register_load_constant(&registers->reg[r], &num);
					emit_load_constant(out, r, &num);
					
					op.op = EQ;
					handle_operation(out, l, l, r, &op);
				}
			}
#ifdef DEBUG
			print_registers(out);
#endif
			break;
		}

		default:
			break;
	}

	return ret;
}

void handle_operation(FILE *out, int dst, int src1, int src2, NodeOP *n)
{
	switch (n->op)
	{
		case GT:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_gt(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_gt(out, dst, src1, src2);
			break;
		}

		case GTEQ:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_gteq(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_gteq(out, dst, src1, src2);
			break;
		}

		case LT:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_lt(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_lt(out, dst, src1, src2);
			break;
		}

		case LTEQ:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_lteq(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_lteq(out, dst, src1, src2);
			break;
		}

		case EQ:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_eq(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_eq(out, dst, src1, src2);
			break;
		}

		case NTEQ:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src2, src2);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_nteq(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_nteq(out, dst, src1, src2);
			break;
		}

		case ADDITION:
		{
			if (n->nops == 1) {
				break;
			}
			if (registers->reg[src1].type == TYPE_FLOAT || registers->reg[src2].type == TYPE_FLOAT) {
				if (registers->reg[src1].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
					emit_convert_int_to_float(out, src1, src1);
				}
				if (registers->reg[src2].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
					emit_convert_int_to_float(out, src2, src2);
				}
				registers->reg[dst].type = TYPE_FLOAT;
			} else {
				registers->reg[dst].type = TYPE_INTEGER;
			}
			register_addition(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_addition(out, dst, src1, src2);
			break;
		}

		case SUBTRACTION:
		{
			if (n->nops == 1) {
				NodeConst tmp;
				int tmpreg = pop(stack_registers);
				if (registers->reg[dst].type == TYPE_INTEGER) {
					tmp.type = TYPE_INTEGER;
					tmp.value.ival = -1;
				} else {
					tmp.type = TYPE_FLOAT;
					tmp.value.fval = -1.0;
				}
				register_load_constant(&registers->reg[tmpreg], &tmp);
				emit_load_constant(out, tmpreg, &tmp);
				register_multiplication(&registers->reg[dst], &registers->reg[dst], &registers->reg[tmpreg]);
				emit_multiplication(out, dst, dst, tmpreg);
				push(stack_registers, tmpreg);
				break;
			}
			if (registers->reg[src1].type == TYPE_FLOAT || registers->reg[src2].type == TYPE_FLOAT) {
				if (registers->reg[src1].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
					emit_convert_int_to_float(out, src1, src1);
				}
				if (registers->reg[src2].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
					emit_convert_int_to_float(out, src2, src2);
				}
				registers->reg[dst].type = TYPE_FLOAT;
			} else {
				registers->reg[dst].type = TYPE_INTEGER;
			}
			register_subtraction(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_subtraction(out, dst, src1, src2);
			break;
		}

		case MULTIPLICATION:
		{
			if (registers->reg[src1].type == TYPE_FLOAT || registers->reg[src2].type == TYPE_FLOAT) {
				if (registers->reg[src1].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
					emit_convert_int_to_float(out, src1, src1);
				}
				if (registers->reg[src2].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
					emit_convert_int_to_float(out, src2, src2);
				}
				registers->reg[dst].type = TYPE_FLOAT;
			} else {
				registers->reg[dst].type = TYPE_INTEGER;
			}
			register_multiplication(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_multiplication(out, dst, src1, src2);
			break;
		}

		case DIVISION:
		{
			if (registers->reg[src1].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_int_to_float(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_INTEGER) {
				register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_int_to_float(out, src1, src1);
			}
			registers->reg[dst].type = TYPE_FLOAT;
			register_division(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_division(out, dst, src1, src2);
			break;
		}

		case INTDIVISION:
		{
			if (registers->reg[src1].type == TYPE_FLOAT) {
				register_convert_float_to_int(&registers->reg[src1], &registers->reg[src1]);
				emit_convert_float_to_int(out, src1, src1);
			}
			if (registers->reg[src2].type == TYPE_FLOAT) {
				register_convert_float_to_int(&registers->reg[src2], &registers->reg[src2]);
				emit_convert_float_to_int(out, src1, src1);
			}
			registers->reg[dst].type = TYPE_INTEGER;
			register_intdivision(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_intdivision(out, dst, src1, src2);
			break;
		}

		case EXPONENT:
		{
			if (registers->reg[src1].type == TYPE_FLOAT || registers->reg[src2].type == TYPE_FLOAT) {	
				if (registers->reg[src1].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src1], &registers->reg[src1]);
					emit_convert_int_to_float(out, src1, src1);
				}
				if (registers->reg[src2].type == TYPE_INTEGER) {
					register_convert_int_to_float(&registers->reg[src2], &registers->reg[src2]);
					emit_convert_int_to_float(out, src2, src2);
				}
				registers->reg[dst].type = TYPE_FLOAT;
			} else {
				registers->reg[dst].type = TYPE_INTEGER;
			}
			register_exponent(&registers->reg[dst], &registers->reg[src1], &registers->reg[src2]);
			emit_exponent(out, dst, src1, src2);
			break;
		}

		default:
			break;
	}
}

void emit_load_constant(FILE *out, int dst, NodeConst *n)
{
	if (n->type == TYPE_INTEGER) {
		fprintf(out, "LD R%d #%d\n", dst, n->value.ival);
	} else {
		fprintf(out, "LD R%d #%lf\n", dst, n->value.fval);
	}
}

void emit_load_identifier(FILE *out, int dst, NodeID *n)
{
	fprintf(out, "LD R%d @%s\n", dst, n->id);
}

void emit_store_register_to_identifier(FILE *out, const char *dst, int src)
{
	fprintf(out, "ST @%s R%d\n", dst, src);
}

void emit_convert_int_to_float(FILE *out, int dst, int src)
{
	fprintf(out, "FL.i R%d R%d\n", dst, src);
}

void emit_convert_float_to_int(FILE *out, int dst, int src)
{
	fprintf(out, "INT.f R%d R%d\n", dst, src);
}

void emit_gt(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "GT.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_gteq(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "GE.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_lt(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "LT.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_lteq(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "LE.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_eq(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "EQ.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_nteq(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "NE.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_addition(FILE *out, int dst, int src1, int src2)
{
	if (registers->reg[dst].type == TYPE_INTEGER) {
		fprintf(out, "ADD.i R%d R%d R%d\n", dst, src1, src2);
	} else {
		fprintf(out, "ADD.f R%d R%d R%d\n", dst, src1, src2);
	}
}

void emit_subtraction(FILE *out, int dst, int src1, int src2)
{
	if (registers->reg[dst].type == TYPE_INTEGER) {
		fprintf(out, "SUB.i R%d R%d R%d\n", dst, src1, src2);
	} else {
		fprintf(out, "SUB.f R%d R%d R%d\n", dst, src1, src2);
	}
}

void emit_multiplication(FILE *out, int dst, int src1, int src2)
{
	if (registers->reg[dst].type == TYPE_INTEGER) {
		fprintf(out, "MUL.i R%d R%d R%d\n", dst, src1, src2);
	} else {
		fprintf(out, "MUL.f R%d R%d R%d\n", dst, src1, src2);
	}
}

void emit_division(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "DIV.f R%d R%d R%d\n", dst, src1, src2);
}

void emit_intdivision(FILE *out, int dst, int src1, int src2)
{
	fprintf(out, "DIV.i R%d R%d R%d\n", dst, src1, src2);
}

void emit_exponent(FILE *out, int dst, int src1, int src2)
{
	if (registers->reg[dst].type == TYPE_INTEGER) {
		fprintf(out, "EXP.i R%d R%d R%d\n", dst, src1, src2);
	} else {
		fprintf(out, "EXP.f R%d R%d R%d\n", dst, src1, src2);
	}
}

void free_translation()
{
	if (registers != NULL) {
		free_register(registers);
	}

	if (stack_registers != NULL) {
		free_stack(stack_registers);
	}

	if (stack_available_temporaries != NULL) {
		free_stack(stack_available_temporaries);
	}

	if (stack_used_temporaries != NULL) {
		free_stack(stack_used_temporaries);
	}
}
