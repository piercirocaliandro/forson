/*
generation.h -- type definitions and function declaration
Copyright 2005 Alfonso Tarantini
This file is part of Forson.

Forson is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Forson is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Forson; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


/* --------------- */
/* INCLUDE HEADERS */
/* --------------- */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <ctype.h>
#include <error.h>
#include <errno.h>
#include <limits.h>
#include <argz.h>

#include <lexicon_scanner_tokens.h>


/* --------- */
/* CONSTANTS */
/* --------- */

/*INTERNAL PARAMETER FOR 'RULE' ABSTRACT DATA TYPE*/
#define RULE_FRAGMENT_SIZE 32
#define GENERATION_THRESHOLD 3
#define STACK_DEFAULT_SIZE 4
#define PARSE_TREE_DEFAULT_CHILDREN_NUM 3

/*COSTANTS FOR DEFAULT PROGRAM BEHAVIOR*/
#define DEFAULT_VERBOSITY 0
#define MAX_VERBOSITY 6
#define DEFAULT_MINSIZE 0
#define DEFAULT_PRINT_TABLE_FLAG 0
#define TABLE_STREAM message_stream
#define DEFAULT_REPEAT 1
#define DEFAULT_STANDARD_OUTPUT_FLAG 0
#define DEFAULT_OUTPUT_PATH "o.out"
#define DEFAULT_SENTENCE_SEPARATOR "\n\n"
#define DEFAULT_PROBABILITY_INITIALIZATION 1
#define DEFAULT_NULL_PATH "/dev/null"
#define DEFAULT_MAX_RECURSION_DEPTH 10

/*DEFINING THE VERBOSITY POLICY AND THE SOURCES OF MESSAGES IN THE PROGRAM*/
#define VERB_POLICY {1,2,4,4,3,4,6,5,0}
#define NUMBER_OF_SOURCES 9

/*DEFINE THE DIFFERENT TYPES OF SYMBOLS IN SYMBOL TABLE*/
#define RC_VALUES {-2, -1, 0, INT_MIN, INT_MIN+1}

/*PARAMETERS FOR BLANK SPACE GENERATOR TUNING*/
#define MORE_BLANKS_PERCENTAGE 10
#define MAX_SPACES 12
#define NEWLINE_PROBABILITY_PERCENTAGE 15
#define TAB_PROBABILITY_PERCENTAGE 10

/*DEFINE TEXT STRINGS FOR RULE TYPE PRINTING*/
#define RULE_TYPE_NAMES {"unrecognized","empty-string","standard","terminal-only","recursive","left-recursive","right-recursive","multiple-recursive","copy","auto-copy","alias"}
#define NUMBER_OF_RULE_TYPES 11

/* ---------------- */
/* TYPE DEFINITIONS */
/* ---------------- */

typedef enum {CORRECT, SYNTAX_ERROR, UNEXPECTED_TOKEN, SEMANTIC_ERROR} parser_return_values;
typedef enum {MAIN, PARSER, G_SCANNER, L_SCANNER, CLEAN_MIN, CLEAN_MAX, LISTOPS, GENERATION, WARNING} source_type;
typedef enum {UNRECOGNIZED, EMPTY, STANDARD, TERMINAL, RECURSIVE, LEFT_RECURSIVE, RIGHT_RECURSIVE, MULTIPLE_RECURSIVE, COPY, AUTO_COPY, ALIAS} rule_type;
typedef enum {LEXICAL, LITERAL, NT, UNDEFINED, RANDOM_LEXICAL} symbol_type;
typedef enum {NORMAL, BAD_ARGUMENTS, BAD_INPUT, UNEXPECTED_ERROR} exit_codes;

/*UNIQUE IDENTIFIER FOR NON TERMINAL SYMBOLS*/
typedef unsigned long symbol_id;

/*IDENTIFIER FOR SINGLE ALTERNATIVES IN RULE DEFINITION  */
/*(UNIQUE FOR IN EVERY NON TERMINAL)                     */
typedef symbol_id rule_t;

/*TYPE FOR STACK IMPLEMENTATION*/
typedef struct STK
{
	symbol_id **buffer;
	int size;
	int stack_size;
} stack;

/*LIST TYPE FOR MULTIPLE RULE DEFINITION OF NON TERMINALS*/
typedef struct RBL
{
	struct RBL *next;
	rule_t *rule;
	short length;
	short visited;
	int probability;
	rule_type type;
} rule_list_entry;

/*LIST TYPE FOR NON TERMINAL SYMBOL TABLE*/
typedef struct SL
{
	/*POINTER TO NEXT SYMBOL IN LINKED LIST. NULL IF LAST*/
	struct SL *next;
	/*UNIQUE ID AND NAME*/
	symbol_id id;
	char *name;
	/*A NEGATIVE RULECOUNT INDICATES A TERMINAL SYMBOL*/
	/*IN THE HEAD NODE, IT IS USED AS A SYMBOL COUNTER*/
	int rulecount;
	/*IN THE HEAD NODE, IT STORES THE VALUE OF THE MAXIMUM */
	/*NUMBER OF RULES FOR A SYMBOL IN THE TABLE            */
	short visited;
	rule_list_entry *shortest;
	rule_list_entry *rules;
} symbol_list_entry;

/*TYPE FOR argz CONTAINER FOR LEXICON ELEMENTS READ FROM A LEXICAL INPUT FILE*/
/*POINTED TO BY THE rules FIELD IN LEXICAL SYMBOLS                           */
typedef struct LEX
{
	char *argz;
	size_t argz_size;
} lexicon_argz_structure;


/*PARSE TREE STRUCTURE*/
typedef struct PARSE_TREE{
	symbol_id sym;
	int num_children;
	int children_array_size;
	struct PARSE_TREE **children;
} parse_tree;

/*-------------------*/
/*FUNCTION DEFINITION*/
/*-------------------*/

/*GENERATION FUNCTIONS*/
rule_list_entry *get_random_rle(symbol_list_entry *sle);
rule_list_entry *get_terminal_rle(symbol_list_entry *sle);
rule_list_entry *choose(symbol_list_entry *sle, symbol_list_entry *symbol_table);
rule_list_entry *get_unvisited_rle(symbol_list_entry *sle);
rule_list_entry *get_with_deep_unvisited_rle(symbol_list_entry *sle, symbol_list_entry *symbol_table);
int rle_minimal_length(rule_list_entry *rle, symbol_list_entry *symbol_table);
int symbol_minimal_length(symbol_list_entry *sle, symbol_list_entry *symbol_table);
rule_list_entry *get_shortest_rle(symbol_list_entry *sle, symbol_list_entry *symbol_table);
void push_rule_on_stack(stack *st, rule_list_entry *rle, symbol_list_entry *symbol_table, parse_tree *tree);

void generate_terminal_text(symbol_list_entry *s);
void print_string(char *point);
char get_escaped_char(char c);
void generate_blank_text();

void grow_shortest(symbol_list_entry *rle, symbol_list_entry *symbol_table);
void grow(symbol_id starting_symbol, symbol_list_entry *symbol_table);
void purdom(symbol_id starting_symbol, symbol_list_entry *symbol_table);

/*DATA STRUCTURE CONSTRUCTION FUNCTIONS*/
void build_tables();
void check_grammar(symbol_list_entry *sle, symbol_id starting_symbol);
int check_error_only(symbol_list_entry *work_sle);
void normalize_rules(symbol_list_entry *sle);
int check_infinite_loops(symbol_list_entry *sle);
void do_lexicon_scanning();
void add_lexical_unit(symbol_id s_id);
rule_type determine_rule_type(symbol_id self_id, rule_list_entry *rle, symbol_list_entry *symbol_table);
void epure_symbol(symbol_list_entry *sle, symbol_list_entry *symbol_table);

/*EXPLICIT DECLARATION OF SCANNER AND PARSER AUTOMATICALLY GENERATED FUNCTIONS*/
int yylex();
int yy_lexiconlex();
int yyparse();
void yyrestart(FILE *new_file);
void yy_lexiconrestart(FILE *new_file);

/*SYMBOL LIST RELATED FUNCTIONS*/
symbol_list_entry *initialize_new_symbol_list();
symbol_id insert_symbol(symbol_list_entry *l, char* new_name, symbol_type t);
void remove_sle(symbol_list_entry *symbol_table, symbol_id id);
symbol_list_entry *get_symbol(symbol_list_entry *l, symbol_id id);
symbol_list_entry *get_symbol_by_name(symbol_list_entry *l, char *name);
void locate_symbol_in_sle(symbol_id id, symbol_list_entry *sle, int *loc);
void visit_symbol_list(	symbol_list_entry *l, symbol_list_entry *symbol_table, int visited_value);

/*SYMBOL TYPE SETTING AND DETERMINATION FUNCTIONS*/
void set_symbol_type(symbol_list_entry *l, symbol_type new_type);
int is_LITERAL(symbol_list_entry *l);
int is_LEXICAL(symbol_list_entry *l);
int is_NT(symbol_list_entry *l);
int is_UNDEFINED(symbol_list_entry *l);
int is_RANDOM_LEXICAL(symbol_list_entry *l);
int get_lexicon_numerosity(symbol_list_entry *l);
int increment_lexicon_numerosity(symbol_list_entry *l);

/*RLE TYPE DETERMINATION FUNCTIONS*/
int is_ONLY_TERMINALS(rule_list_entry *r);

/*RLE LIST RELATED FUNCTIONS*/
void insert_rle(symbol_list_entry *l, rule_list_entry *new_rule);
void remove_rle(symbol_list_entry *l, int pos);
rule_list_entry *get_rle(symbol_list_entry *l, int pos);
rule_list_entry *link_rle(rule_list_entry *l, rule_list_entry *new_rle);

/*RLE RELATED FUNCTIONS*/
void insert_symbol_rle(rule_list_entry *r, symbol_id sym);
int remove_symbol_rle(rule_list_entry *r, int pos);
symbol_id extract_symbol_rle(rule_list_entry *r, int pos);
void modify_symbol_rle(rule_list_entry *rle, int pos, symbol_id new_value);
rule_list_entry *initialize_new_rle();

/*RULE RELATED FUNCTIONS*/
symbol_id extract_symbol_rule(rule_t *rule, int pos);
void modify_symbol_rule(rule_t *rule, int pos, symbol_id new_value);
void insert_symbol_rule(rule_t *rule, symbol_id sym, int pos);
rule_t *remove_symbol_rule(rule_t *rule, int pos, int length);
rule_t *initialize_new_rule();

/*STACK RELATED FUNCTIONS*/
stack *initialize_new_stack();
symbol_id pop(stack *st);
int push(stack *st, symbol_id symb);
int get_size(stack *st);
int clean_stack(stack *st);

/*LEXICON ARGZ STRUCTURE RELATED FUNCTIONS*/
lexicon_argz_structure *initialize_new_lexicon_argz_structure();

/*MESSAGE PRINTING FUNCTIONS*/
void print_symbol_list(symbol_list_entry *l);
void print_rule_list(symbol_list_entry *l);
void print_lexicon_table(symbol_list_entry *l);
int must_print_message(source_type class);

/*UTILITY FUNCTIONS*/
void set_random_seed();
int read_number(char *string);
FILE *open_file_read(char *string);
FILE *open_file_write(char *string);
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t elem_sz);
void print_version();
void print_usage();

/*CLEAN UP FUNCTIONS*/
void clean_symbol_list(symbol_list_entry *l);
void clean_rle_list(rule_list_entry *l);
void clean_rule(rule_t *r);
void clean_up(void);

/*PARSE TREE FUNCTIONS*/
parse_tree *init_parse_tree(symbol_id sym);
parse_tree *init_empty_parse_tree(void);
int parse_tree_push_child(parse_tree *tree, symbol_id sym);
void parse_tree_clean(parse_tree *tree);
