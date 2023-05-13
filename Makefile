OBJS = main.o grow.o build_tables.o listops.o stack.o utilities.o print_tables.o metagrammar.yylex.o metagrammar.tab.o lexicon.yylex.o

CFLAGS += -I./include -I. -g

all : forson

forson : $(OBJS)
	gcc $(OBJS) -o forson

metagrammar.yylex.c : metagrammar.lex include/generation.h
	flex -ometagrammar.yylex.c metagrammar.lex

metagrammar.yylex.o : metagrammar.yylex.c metagrammar.tab.h include/generation.h
	gcc $(CFLAGS) -c metagrammar.yylex.c 

metagrammar.tab.c metagrammar.tab.h : metagrammar.y include/generation.h
	bison -d metagrammar.y

metagrammar.tab.o : metagrammar.tab.c include/generation.h
	gcc $(CFLAGS) -c metagrammar.tab.c

lexicon.yylex.c : lexicon.lex include/generation.h include/lexicon_scanner_tokens.h
	flex -olexicon.yylex.c lexicon.lex

lexicon.yylex.o : lexicon.yylex.c include/generation.h
	gcc $(CFLAGS) -c lexicon.yylex.c

main.o : main.c include/generation.h
	gcc $(CFLAGS) -c main.c	

grow.o : grow.c include/generation.h
	gcc $(CFLAGS) -c grow.c

listops.o : listops.c include/generation.h
	gcc $(CFLAGS) -c listops.c

stack.o : stack.c include/generation.h
	gcc $(CFLAGS) -c stack.c

build_tables.o: build_tables.c include/generation.h
	gcc $(CFLAGS) -c build_tables.c

utilities.o : utilities.c include/generation.h
	gcc $(CFLAGS) -c utilities.c

print_tables.o :print_tables.c include/generation.h
	gcc $(CFLAGS) -c print_tables.c 

clean : 
	rm -f gen $(OBJS) *.yylex.* *.tab.* forson
