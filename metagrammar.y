/*
metagrammar.y -- bison parser for bison's metagrammar
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


%{
/*---------------------------------------------*/
/*BISON PARSER DEFINITION FOR BISON METAGRAMMAR*/
/*---------------------------------------------*/

#include <generation.h>

/*SHORTHAND MACROS*/
#define ERROR(M,L) error(BAD_INPUT,0,"%s: line %d: %s",input_grammar_file_path,L,M)
#define remove_quotes(M) M+=sizeof(char);M[yyleng-2]='\0'

/*WORKING VARIABLES*/
symbol_list_entry *s=NULL;
rule_list_entry *r=NULL;
rule_list_entry *r_list=NULL;
int r_count = 0;

/*GLOBAL VARIABLES DEFINED IN CALLING FUNCTION*/

/*POINTER TO SYMBOL TABLE TO BE CONSTRUCTED*/
extern symbol_list_entry *symbol_table;
/*ID OF STARTING SYMBOL WILL BE PLACED HERE*/
extern symbol_id starting_symbol;
/*NAME OF THE INPUT FILE, FOR ERROR REPORTING PURPOSES*/
extern char *input_grammar_file_path;

/*GLOBAL STREAM FOR MESSAGES*/
extern FILE *message_stream;

/*STANDARD GLOBAL VARIABLES DEFINED IN FLEX SCANNER*/
extern char *yytext;
extern int yyleng;
extern int yylineno;

%}

%define parse.error verbose
%expect 2
%locations


%start yfile
%token IDENTIFIER DOUBLE_QUOTED_LITERAL SINGLE_QUOTED_LITERAL
%token PART_SEPARATOR
%token START_DECL TOKEN_DECL 
%token MISCELLANEOUS_DECL MISCELLANEOUS_EMPTY_DECL MISCELLANEOUS_FILENAME_DECL
%token ERROR_RESERVED_TOKEN
%token SET_SYM_VAL

%%

yfile : 	declarations
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced declarations section\n");
		}		
		PART_SEPARATOR
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced first PART_SEPARATOR\n");
			/*INITIALIZE WORKING VARIABLE r*/
			r = initialize_new_rle();
		}
		grammar
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced grammar section\n");
		}
		rest_of_file
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced rest_of_file section...done!\n");
		};
	
rest_of_file :	/*empty*/
		{
			/*PROGRAM DOES NOT CARE ABOUT WHAT FOLLOWS THE GRAMMAR SECTION*/
			YYACCEPT;
		}
		| PART_SEPARATOR	
		{
			/*PROGRAM DOES NOT CARE ABOUT WHAT FOLLOWS THE GRAMMAR SECTION*/
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced second PART_SEPARATOR\n");
			YYACCEPT;
		}
		| error 
		{
			if(must_print_message(WARNING))
			{
				fprintf(message_stream, "warning: syntax error ignored since encountered after a valid grammar section\n");
			}
			return(0);
		};
		
declarations :	/*empty*/
		| misc_declaration declarations
		| token_declaration declarations
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "more declarations follow...\n");
		}
		| start_declaration declarations2
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "more declarations follow...\n");
		};

declarations2 :	/*empty*/
		| declarations2 misc_declaration
		| declarations2 token_declaration
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "more declarations follow...(no more start declarations)\n");
		};
		
start_declaration : START_DECL start_symbol optional_semicolon
		{
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced start declaration\n");
		};

start_symbol : 	IDENTIFIER
		{
			symbol_list_entry *s = NULL;

			s = get_symbol(symbol_table, $1);
			assert(s != NULL);
			
			set_symbol_type(s, NT);
			starting_symbol = s->id;
		};
		
token_declaration :	TOKEN_DECL token_list optional_semicolon
			{
				if(must_print_message(PARSER))
					fprintf(message_stream, "reduced token declaration\n");
			};

token_list : 	token_list token
		| token;

token:		IDENTIFIER
		{
			symbol_list_entry *s = NULL;

			/*SET SYMBOL TYPE TO LEXICAL AND CHECK FOR ERRORS*/

			assert($1 > 0);	

			s = get_symbol(symbol_table, $1);
			assert(s != NULL);

			if(is_UNDEFINED(s))
			{
				/*UNENCOUNTERED DECLARATION*/
				set_symbol_type(s, LEXICAL);
			}
			else if(is_NT(s) && starting_symbol == $1)
			{
				/*ATTEMPTING TO DECLARE STARTING SYMBOL AS A TOKEN*/
				ERROR("token already declared as starting symbol", @1.first_line);
			}
			else if(is_NT(s))
			{
				/*DECLARED MORE THAN ONCE (AS AN ALIAS FOR A LITERAL), THAT'S ALL RIGHT*/
				set_symbol_type(s, NT);
			}
			else if(is_LEXICAL(s))
			{
				/*DECLARED MORE THAN ONCE, THAT'S ALL RIGHT*/
				set_symbol_type(s, LEXICAL);
			}
			else
			{
				/*THIS MEANS IT'S A LITERAL. SCANNER ERROR*/
				assert(0);
			}
		}			
		| IDENTIFIER somehow_quoted_literal
		{
			symbol_list_entry *s = NULL, *l = NULL;
			rule_list_entry *rle = NULL;
			/*CONNECT LITERAL AND IT'S ALIAS WITH A RULE*/
			/*THE TOKEN IS AN NT SYMBOL WITH ONE RULE CONTAINING THE LITERAL*/

			assert($1 > 0);
			assert($2 > 0);

			s = get_symbol(symbol_table, $1);
			l = get_symbol(symbol_table, $2);

			assert(s != NULL);
			assert(l != NULL);

			if(is_UNDEFINED(s))
			{
				/*UNENCOUNTERED DECLARATION*/
				set_symbol_type(s, NT);
			}
			else if(is_NT(s) && starting_symbol == $1)
			{
				/*ATTEMPTING TO DECLARE STARTING SYMBOL AS A TOKEN*/
				ERROR("token already declared as starting symbol", @1.first_line);
			}
			else if(is_NT(s))
			{
				/*ALREADY DECLARED AS AN ALIAS FOR A LITERAL!*/
				ERROR("token already declared as an alias for a literal", @1.first_line);
			}
			else if(is_LEXICAL(s))
			{
				/*ALREADY DECLARED AS A TOKEN, NOT AN ALIAS*/
				set_symbol_type(s, NT);
			}
			else
			{
				/*THIS MEANS IT'S A LITERAL. SCANNER ERROR*/
				assert(0);
			}
			rle = initialize_new_rle();
			insert_symbol_rle(rle, $2);
			insert_rle(s, rle);
		}
		| somehow_quoted_literal
		{
			/*NOTHING TO BE DONE, CHECKS PERFORMED BY SCANNER*/
		}
		| ERROR_RESERVED_TOKEN
		{
			/*NOTHING TO BE DONE*/
		};

somehow_quoted_literal:	SINGLE_QUOTED_LITERAL
			{
				assert($1 > 0);
				$$ = $1;
			}
			| DOUBLE_QUOTED_LITERAL
			{
				assert($1 > 0);
				$$ = $1;
			};

misc_declaration :	MISCELLANEOUS_DECL optional_semicolon
			|MISCELLANEOUS_EMPTY_DECL optional_semicolon
			|MISCELLANEOUS_FILENAME_DECL optional_semicolon;

optional_semicolon :	/*EMPTY*/
			| ';';

grammar : 	grammar definition
		| definition;
		
definition :	result ':' rule_list ';'
		{
			s = get_symbol(symbol_table, $1);
			assert(s != NULL);
			
			if(must_print_message(PARSER))
				fprintf(message_stream, "reduced rule definition for symbol: %d (%s)\n", s->id, s->name);

			while(r_list != NULL)
			{	
				assert(s != NULL);

				insert_rle(s, r_list);
				r_list = r_list->next;
			}

			/*MUST SET STARTING SYMBOL IF NOT SET BY %start DECLARATION*/
			if(starting_symbol == 0)
				starting_symbol = $1;
		};

result : 	IDENTIFIER
		{
			symbol_list_entry *s=NULL;
			
			s=get_symbol(symbol_table, $1);
			assert(s != NULL);

			/*RESULT OF A RULE MUST BE A NON TERMINAL SYMBOL*/
			if(is_LEXICAL(s))
			{
				ERROR("attempting to assign rule to a token", @1.first_line);
			}
			set_symbol_type(s, NT);
			$$ = $1;
		};

rule_list :	rule_list '|' rule
		{
			/*PUSH THE RULE ON THE LIST AND CREATE A NEW BLANK ONE*/
			if($3 >= 0)
			{
				r_list = link_rle(r_list, r);
			}
			else
			{	
				if(must_print_message(PARSER))
					fprintf(message_stream, "found rule containing 'error' reserved keyword\n");
				clean_rle_list(r);
			}

			r = initialize_new_rle();
		}
		| rule
		{
			/*PUSH THE RULE ON THE LIST AND CREATE A NEW BLANK ONE*/
			if($1 >= 0)
			{
				r_list = link_rle(r_list, r);
			}
			else
			{	
				if(must_print_message(PARSER))
					fprintf(message_stream, "found rule containing 'error' reserved keyword\n");
				clean_rle_list(r);
			}

			r = initialize_new_rle();
		};
		
rule : 		/*empty*/
		{ 
			$$ = 0;
		}
		| component_list
		{
			assert($1 != 0);
			$$ = $1;
		}
        /*| SET_SYM_VAL {
            fprintf(message_stream, "found val %s\n", $1);
        }
        | component_list SET_SYM_VAL
        {
            fprintf(message_stream, "found val %s\n", yytext);
        }*/;

component_list : component_list component
		{
			/*INSERT SYMBOL FOUND IN RULE BEING CONSTRUCTED*/
			assert($2 != 0);

			if($2 > 0)
			{
				insert_symbol_rle(r, (symbol_id)$2);
			}
			$$ = $1 * $2;
			if($$ > 1)
				$$ = 1;
		}
        | component_list component SET_SYM_VAL 
        {
            fprintf(message_stream, "found val (2) %s\n", yytext);
        }
		| component
		{
			/*INSERT SYMBOL FOUND IN LIST IN RULE BEING CONSTRUCTED*/
			assert($1 != 0);

			if($1 > 0)
			{
				insert_symbol_rle(r, (symbol_id)$1);
			}
			$$ = $1;
			if($$ > 1)
				$$ = 1;
			
		}
        | component SET_SYM_VAL
        {
            fprintf(message_stream, "found val (3) %s\n", yytext);
        };

component :	IDENTIFIER
		{
			assert($1 > 0);
			$$ = $1;
		}
		| somehow_quoted_literal
		{
			assert($1 > 0);
			$$ = $1;
		}
		| ERROR_RESERVED_TOKEN
		{
			assert($1 < 0);
			$$ = $1;
		};

%%

int yyerror(YYLTYPE *locp, char const *s)
{
	fprintf(stderr, "%s: line %d: syntax error: found \"%s\", expecting: \"%s\"\n", input_grammar_file_path, locp->last_line, yytext, s);
	
	return 1;
}
