#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <AST.h>
#include <grammar/calc.tab.h>

#define BUFFER_SIZE 32

extern FILE *yyin;
extern int yylex();
extern int yylex_destroy();
extern char *yytext;
extern int yylineno;
extern int pos;

FILE *out_err;

void yyerror(const char *s) { /* empty */ }

int main(int argc, char *argv[])
{
	int opt;
	char *input_file, *output_file = NULL;
	FILE *in, *out = NULL;

	int token, prevtoken;
	int literal_cap = 0, unknown_literal_cap = 0;
	int literal_length = 0, unknown_literal_length = 0;
	char *literal = NULL, *unknown_literal = NULL;

	// Read options from command line
	while ((opt = getopt(argc, argv, "ho:")) != -1) {
		switch (opt)
		{
			case 'o':
				output_file = optarg;
				break;
			case 'h':
			default:
				fprintf(stderr, "USAGE: %s [-o output] file\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	// Argument is placed before options
	if (optind >= argc) {
		fprintf(stderr, "USAGE: %s [-o output] file\n", argv[0]);
		fprintf(stderr, "Expected argument after options.\n");
		exit(EXIT_FAILURE);
	}

	input_file = argv[optind];

	in = fopen(input_file, "r");
	if (in == NULL) {
		fprintf(stderr, "Failed to open %s.\n", input_file);
		exit(EXIT_FAILURE);
	}

	yyin = in;

	// Setup output stream
	out = stdout;
	out_err = stderr;
	if (output_file != NULL) {
		out = fopen(output_file, "w");
		out_err = out;
	}

	if (out == NULL) {
		fprintf(stderr, "Failed to open %s.\n", output_file);
		fclose(in);
		exit(EXIT_FAILURE);
	}

	// Extract token
	token = yylex();
	while (token) {
		// Encountered unknown token
		if (token == INVALIDTOK) {
			// Encountered first invalid token
			if (unknown_literal == NULL) {
				unknown_literal_length = strlen(yytext);
				if (unknown_literal_length < BUFFER_SIZE) {
					unknown_literal_cap = BUFFER_SIZE;
				} else {
					unknown_literal_cap = BUFFER_SIZE * (unknown_literal_length / BUFFER_SIZE + 1);
				}
				unknown_literal = (char *)malloc(unknown_literal_cap * sizeof(char));
				strcpy(unknown_literal, yytext);
			} else if (unknown_literal_length == 0) {
				// Copy invalid token
				unknown_literal_length += strlen(yytext);
				if (unknown_literal_length >= unknown_literal_cap) {
					unknown_literal_cap = BUFFER_SIZE * (unknown_literal_length / BUFFER_SIZE + 1);
					unknown_literal = (char *)realloc(unknown_literal, unknown_literal_cap * sizeof(char));
				}
				strcpy(unknown_literal, yytext);
			} else {
				// Concatenate sequence of invalid tokens
				unknown_literal_length += strlen(yytext);
				if (unknown_literal_length >= unknown_literal_cap) {
					unknown_literal_cap = BUFFER_SIZE * (unknown_literal_length / BUFFER_SIZE + 1);
					unknown_literal = (char *)realloc(unknown_literal, unknown_literal_cap * sizeof(char));
				}
				strcat(unknown_literal, yytext);
			}
			token = yylex();
			continue;
		}

		// Valid token
		// Print the sequence of invalid tokens
		if (unknown_literal_length > 0) {
			fprintf(out, "%s/ERR ", unknown_literal);
			unknown_literal_length = 0;
		}

		// Encountered newline
		if (token == NEWLINE) {
			fprintf(out, "\n");
			token = yylex();
			continue;
		}

		// Save token.
		// Extract incomming token and check whether it's a valid or invalid token.
		// If it's invalid then create a buffer and store the token value to it.
		// Otherwise just print literal and token type.
		prevtoken = token;
		literal_length = strlen(yytext);
		if (literal == NULL) {
			if (literal_length < BUFFER_SIZE) {
				literal_cap = BUFFER_SIZE;
			} else {
				literal_cap = BUFFER_SIZE * (literal_length / BUFFER_SIZE + 1);
			}
			literal = (char *)malloc(literal_cap * sizeof(char));
		}
		if (literal_length >= literal_cap) {
			literal_cap = BUFFER_SIZE * (literal_length / BUFFER_SIZE + 1);
			literal = (char *)realloc(literal, literal_cap * sizeof(char));
		}
		strcpy(literal, yytext);
		token = yylex();
		if (token == INVALIDTOK) {
			unknown_literal_length = literal_length;
			if (unknown_literal == NULL) {
				if (unknown_literal_length < BUFFER_SIZE) {
					unknown_literal_cap = BUFFER_SIZE;
				} else {
					unknown_literal_cap = BUFFER_SIZE * (unknown_literal_length / BUFFER_SIZE + 1);
				}
				unknown_literal = (char *)malloc(unknown_literal_cap * sizeof(char));
			}
			if (unknown_literal_length >= unknown_literal_cap) {
				unknown_literal_cap = BUFFER_SIZE * (unknown_literal_length / BUFFER_SIZE + 1);
				unknown_literal = (char *)realloc(unknown_literal, unknown_literal_cap * sizeof(char));
			}
			strcpy(unknown_literal, literal);
			continue;
		}

		// Valid subsequent token
		fprintf(out, "%s/", literal);

		switch (prevtoken)
		{
			case INTEGER:
			case REALNUM:
				fprintf(out, "NUM ");
				break;
			case IDENTIFIER:
				fprintf(out, "VAR ");
				break;
			case ASSIGNMENT:
			case ADDITION:
			case SUBTRACTION:
			case MULTIPLICATION:
			case INTDIVISION:
			case DIVISION:
			case EXPONENT:
			case GTEQ:
			case GT:
			case LT:
			case LTEQ:
			case EQ:
			case NTEQ:
			case LPARENTHESE:
			case RPARENTHESE:
			{
				// Print "pow" instead of the sign "^"
				if (strcmp(literal, "^") == 0) {
					fprintf(out, "pow ");
					break;
				}
				if (strcmp(literal, "//") == 0) {
					fprintf(out, "intdiv ");
					break;
				}
				if (strcmp(literal, "/") == 0) {
					fprintf(out, "div ");
					break;
				}
				if (strcmp(literal, "(") == 0) {
					fprintf(out, "lparenthese ");
					break;
				}
				if (strcmp(literal, ")") == 0) {
					fprintf(out, "rparenthese ");
					break;
				}

				fprintf(out, "%s ", literal);
			}

			default:
				break;
		}
	}

	yylex_destroy();

	if (unknown_literal_length > 0) {
		fprintf(out, "%s/ERR ", unknown_literal);
	}

	if (unknown_literal != NULL) {
		free(unknown_literal);
	}

	if (literal != NULL) {
		free(literal);
	}

	// Close file descriptors
	if (out != stdout) {
		fclose(out);
	}
	fclose(in);

	return 0;
}
