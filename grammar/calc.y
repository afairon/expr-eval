%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <math.h>

	#include <AST.h>
	#include <hashtable.h>
	#include <symbol.h>

	extern int yylex();
	void yyerror(const char *);

	extern char *yytext;
	extern int yylineno;
	extern int yyleng;
	extern int pos;
	int undefined_variable = 0;
	char err_msg[256];

	hashtable_t *symbols;
	Node *root = NULL;
%}

%code requires
{
	#include <AST.h>
}

%start start

%union
{
	Node *node;
	double fval;
	int ival;
	char *id;
}

%token <ival> INTEGER
%token <fval> REALNUM
%token <id> IDENTIFIER
%token ASSIGNMENT
%token ADDITION SUBTRACTION
%token MULTIPLICATION DIVISION INTDIVISION
%token EXPONENT
%token GT GTEQ LT LTEQ EQ NTEQ
%token LPARENTHESE RPARENTHESE
%token NEWLINE
%token INVALIDTOK

%type <node> line stmt assignment expr term term2 term3 unary primary

%left ADDITION MULTIPLICATION DIVISION INTDIVISION
%right SUBTRACTION EXPONENT
%right ASSIGNMENT

%destructor {free_node(&$$);} <node>

%%

start		:	line				{root = $1;}
       		;

line		:	stmt				{$$ = $1;}
			|	line NEWLINE stmt	{$$ = $1;push_back($1, $3);}
			;

stmt		:	%empty				{$$ = NULL;}
      		|	assignment			{$$ = $1;}
      		|	expr				{$$ = $1;}
		;

assignment	:	IDENTIFIER ASSIGNMENT expr	{$$ = create_id($1, $3);}
	   	;

expr		:	expr GT term			{$$ = create_op(GT, 2, $1, $3);}
      		|	expr GTEQ term			{$$ = create_op(GTEQ, 2, $1, $3);}
      		|	expr LT term			{$$ = create_op(LT, 2, $1, $3);}
      		|	expr LTEQ term			{$$ = create_op(LTEQ, 2, $1, $3);}
      		|	expr EQ term			{$$ = create_op(EQ, 2, $1, $3);}
      		|	expr NTEQ term			{$$ = create_op(NTEQ, 2, $1, $3);}
      		|	term				{$$ = $1;}
      		;

term		:	term ADDITION term2		{$$ = create_op(ADDITION, 2, $1, $3);}
      		|	term SUBTRACTION term2		{$$ = create_op(SUBTRACTION, 2, $1, $3);}
      		|	term2				{$$ = $1;}
		;

term2		:	term2 MULTIPLICATION term3	{$$ = create_op(MULTIPLICATION, 2, $1, $3);}
       		|	term2 DIVISION term3		{$$ = create_op(DIVISION, 2, $1, $3);}
		|	term2 INTDIVISION term3		{$$ = create_op(INTDIVISION, 2, $1, $3);}
       		|	term3				{$$ = $1;}
		;

term3		:	unary EXPONENT term3		{$$ = create_op(EXPONENT, 2, $1, $3);}
       		|	unary				{$$ = $1;}
		;

unary		:	primary				{$$ = $1;}
       		|	ADDITION unary			{$$ = create_op(ADDITION, 1, $2);}
		|	SUBTRACTION unary		{$$ = create_op(SUBTRACTION, 1, $2);}
		;

primary		:	IDENTIFIER			{
										Node *value = symboltable_get(symbols, $1);
										if (value == NULL) {
											undefined_variable = 1;
											sprintf(err_msg, "undefined variable %s", $1);
											YYABORT;
										}
										$$ = create_id($1, (Node *)value);
										pos=pos+yyleng;
									}
	 	|	INTEGER				{$$ = create_const_int($1);pos=pos+yyleng;}
		|	REALNUM				{$$ = create_const_float($1);pos=pos+yyleng;}
		|	LPARENTHESE expr RPARENTHESE	{$$ = $2; if ($$ != NULL) {$$->N.operation.is_term = 1;}}
		;

%%
