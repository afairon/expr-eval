#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <AST.h>
#include <translate.h>
#include <hashtable.h>
#include <symbol.h>
#include <grammar/calc.tab.h>

extern int yyparse();
extern int yylex_destroy();
extern int yylineno;
extern int undefined_variable;

extern hashtable_t *symbols;
extern Node *root;

FILE *out = NULL, *out_err = NULL;

void expr(Node *n)
{
	Node *next = n, *temp;

	while (next != NULL) {
		temp = next;
		next = next->next;
		emit(out, temp);
	}
}

void yyerror(const char *s)
{
	fprintf(out_err, "\nERROR\n\n");
}

int main(int argc, char *argv[])
{
	int opt, err;
	char *input_file, *output_file = NULL;
	FILE *in = NULL;

	char *line = NULL;
	size_t len = 0;
	ssize_t nread;

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

	// Setup output stream
	out = stdout;
	out_err = stderr;
	if (output_file != NULL) {
		out = fopen(output_file, "w");
		out_err = out;
	}

	// Cannot open file
	if (out == NULL) {
		fprintf(stderr, "Failed to open %s.\n", output_file);
		fclose(in);
		exit(EXIT_FAILURE);
	}

	symbols = create_symboltable();

	// Read line by line
	while ((nread = getline(&line, &len, in)) != -1) {
		yy_scan_string(line);
		err = yyparse();
		if (undefined_variable) {
			yyerror(NULL);
		}
		if (!err && !undefined_variable) {
			expr(root);
		}
		free_node(&root);
		yylex_destroy();
		fprintf(out, "\n");
		undefined_variable = 0;
	}

	free(line);
	free_translation();
	free_symboltable(symbols);

	// Close file descriptors
	if (out != stdout) {
		fclose(out);
	}
	fclose(in);

	return 0;
}
