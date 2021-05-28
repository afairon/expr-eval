CC = gcc
YACC = bison
LEX = flex

CFLAGS = -Wall -I . -I include/
LDFLAGS = -lfl -lm
YFLAGS = -d -Wcounterexamples -Wempty-rule

PROGRAMS = build/scanner build/parser build/compiler

.PHONY: all
all: build obj $(PROGRAMS)

.PHONY: scanner
scanner: build obj build/scanner

.PHONY: parser
parser: build obj build/parser

.PHONY: compiler
compiler: build obj build/compiler

build/scanner: obj/scanner.o obj/AST.o obj/hashtable.o obj/symbol.o obj/calc.tab.o obj/lex.yy.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/parser: obj/parser.o obj/AST.o obj/hashtable.o obj/symbol.o obj/calc.tab.o obj/lex.yy.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/compiler: obj/compiler.o obj/AST.o obj/hashtable.o obj/register.o obj/stack.o obj/symbol.o obj/translate.o obj/calc.tab.o obj/lex.yy.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

obj/scanner.o: src/scanner.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/parser.o: src/parser.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/compiler.o: src/compiler.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/AST.o: src/AST.c include/AST.h grammar/calc.tab.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/hashtable.o: src/hashtable.c include/hashtable.h src/symbol.c include/symbol.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/register.o: src/register.c include/register.h src/AST.c include/AST.h grammar/calc.tab.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/stack.o: src/stack.c include/stack.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/symbol.o: src/symbol.c include/symbol.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/translate.o: src/translate.c include/translate.h src/AST.c include/AST.h grammar/calc.tab.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/calc.tab.o: grammar/calc.tab.c grammar/calc.tab.h
	$(CC) -c $(CFLAGS) -o $@ $<

obj/lex.yy.o: token/lex.yy.c
	$(CC) -c $(CFLAGS) -o $@ $<

token/lex.yy.c: token/calc.l grammar/calc.tab.h
	$(LEX) -o $@ $<

grammar/calc.tab.c: grammar/calc.y
	$(YACC) $(YFLAGS) -o $@ $<

grammar/calc.tab.h: grammar/calc.y
	$(YACC) $(YFLAGS) -o grammar/calc.tab.c $<

build:
	mkdir build

obj:
	mkdir obj

.PHONY:	clean
clean:
	rm -f $(PROGRAMS) obj/*.o token/lex.yy.* grammar/calc.tab.*
