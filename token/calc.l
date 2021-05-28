/* Include header for token types */
%{
	#include <stdlib.h>
	#include <string.h>

	#include <AST.h>
	#include <grammar/calc.tab.h>	// generated via flex -d

	extern int yylineno;	// line number
	extern int yyleng;	// length of token
	int pos = 1;		// token position
	int parser = 1; // use parser

	char *id = NULL;
%}

number		(0|[1-9][0-9]*)
decimal		({number}\.{number}*|\.{number}+)
scientific	({number}[Ee][\+\-]?{number}+|{decimal}[Ee][\+\-]?{number}+)
identifier	[A-Za-z][_0-9A-Za-z]*

%%

{number}			{yylval.ival = atol(yytext);return INTEGER;}
({decimal}|{scientific})	{yylval.fval = atof(yytext);return REALNUM;}
{identifier}			{
							if (parser) {
								id = (char *)malloc((strlen(yytext) + 1) * sizeof(char));
								yylval.id = id;
								if (id != NULL) {
									strcpy(yylval.id, yytext);
								}
							}
							return IDENTIFIER;
						}
\+				{++pos;return ADDITION;}
\-				{++pos;return SUBTRACTION;}
\*				{++pos;return MULTIPLICATION;}
\/\/				{pos=pos+2;return INTDIVISION;}
\/				{++pos;return DIVISION;}
\^				{++pos;return EXPONENT;}
">="				{pos=pos+2;return GTEQ;}
">"				{++pos;return GT;}
"<="				{pos=pos+2;return LTEQ;}
"<"				{++pos;return LT;}
"=="				{pos=pos+2;return EQ;}
"!="				{pos=pos+2;return NTEQ;}
"="				{++pos;return ASSIGNMENT;}
"("				{++pos;return LPARENTHESE;}
")"				{++pos;return RPARENTHESE;}
(\r\n|\n)+			{yylineno=yylineno+yyleng;pos=1;return NEWLINE;}
[ \t]+				{pos=pos+yyleng;} // Whitespace
.				{return INVALIDTOK;}

%%

// Input is exhausted
int yywrap(void)
{
	return 1;
}
