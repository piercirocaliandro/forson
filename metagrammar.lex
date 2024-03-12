/*
metagrammar.lex -- flex scanner for bison metagrammar
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

/*-----------------------------------------------*/
/*FLEX G_SCANNER DEFINITION FOR BISON METAGRAMMAR*/
/*-----------------------------------------------*/

#include <metagrammar.tab.h>
#include <generation.h>

/*SHORTHAND MACROS*/
#define MESSAGE_WITH(M) fprintf(message_stream, "%s: line %d: %s: %s\n",input_grammar_file_path,yylineno,M, yytext)
#define MESSAGE_WITHOUT(M) fprintf(message_stream, "%s: line %d: %s\n",input_grammar_file_path,yylineno,M)
#define ERROR(M) error(BAD_INPUT,0,"%s: line %d: %s: %s",input_grammar_file_path,yylineno,M,yytext)
#define remove_quotes(M) M+=sizeof(char);M[yyleng-2]='\0'; yyleng-=2

/*GLOBALS DEFINED IN CALLING FUNCTION*/

/*NAME OF THE INPUT FILE, FOR ERROR REPORTING PURPOSES*/
extern char *input_grammar_file_path;
/*SLE IN WHICH TO INSERT THE SYMBOLS FOUND DURING SCAN*/
extern symbol_list_entry *symbol_table;
/*GLOBAL STREAM FOR MESSAGES*/
extern FILE *message_stream;


/*WORKING VARIABLES*/

/*FOR NOT BEING FOOLED BY NESTED BRACKETS*/
int brack_nesting = 0, prologue = 0;

%}


%option noyywrap
%option yylineno

%x in_curly_brackets
%x in_double_quotes

%x in_comment
%x exiting_comment

%x in_prologue
%x exiting_prologue

%x in_inline_comment

%x in_misc_decl

VALID [[:alnum:]._*]

%%


^%start			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found START_DECL");
				return START_DECL;
			}

^("%token"|"%left"|"%right"|"%nonassoc")	{
				/*THESE BISON DECLARATIONS ARE ALL EQUAL TO */
				/*THE %token DECLARATION                    */
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found TOKEN_DECL");
				return TOKEN_DECL;
			}

^("%destructor"|"%union"|"%type")	{
				/*THESE DECLARATIONS DON'T MATTER TO FORSON     */
				/*BUT IDENTIFIERS CAN FOLLOW (MUST BE DISCARDED)*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found MISCELLANEOUS_DECL, entering start condition");
				BEGIN(in_misc_decl);
			}

<in_misc_decl>[\n]/%	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("...exiting start condition");
				yylineno--;
				yyless(0);
				BEGIN(0);
				return MISCELLANEOUS_DECL;
			}

<in_misc_decl>.|[\n]	{
				/*EAT ANYTHING IN MISCELLANEOUS DECLARATIONS*/
			}

^("%debug"|"%defines"|"%no-parser"|"%no-lines"|"%pure-parser"|"%token-table"|"%expect"|"%expect-rr"|"%pure_parser"|"%lex-param"|"%parse-param"|"%locations"|"%token_table")	{
				/*THESE DECLARATIONS DON'T MATTER TO FORSON*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found MISCELLANEOUS_EMPTY_DECL");
				return MISCELLANEOUS_EMPTY_DECL;
			}

^("%file-prefix"|"%name-prefix"|"%output")=\"[^\n"]*\"\n	{
				/*THESE DECLARATIONS DON'T MATTER BUT SPECIFY A STRING (MUST BE DISCARDED)*/
				/*REMOVE TRAILING NEWLINE*/
				yytext[yyleng-2] = '\0';
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found MISCELLANEOUS_FILENAME_DECL");
				return MISCELLANEOUS_FILENAME_DECL;
			}

("%prec")[^\n]+\n	{
				/*MYSTERIOUS UNDOCUMENTED DECLARATION         */
				/*WHICH APPEARS IN RULES!!!                   */
				/*FOUND IN PARSERS FOR THE gcc                */
				/*LOOKS LIKE PRECEDENCE SO FORSON DOESN'T CARE*/
			}

^%%[ \t]*\n			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found PART_SEPARATOR");
				return PART_SEPARATOR;
			}

error			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found ERROR_RESERVED_TOKEN");

				yylval = -1;

				return ERROR_RESERVED_TOKEN;
			}

[a-zA-Z._]{VALID}*	{
				symbol_list_entry *s = NULL;

				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found IDENTIFIER");

				/*ADD THE NEW IDENTIFIER IF MISSING. SET SEMANTIC VALUES*/
				s = get_symbol_by_name(symbol_table, yytext);
					
				if(s == NULL)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, UNDEFINED);
				}
				else if(is_LITERAL(s) == 1)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, UNDEFINED);
				}
				else
				{
					yylval = (int) s->id;
				}
				
				return IDENTIFIER;
			}

\"[^"\n]+\"		{
				symbol_list_entry *s = NULL;

				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found DOUBLE_QUOTED_LITERAL");

				remove_quotes(yytext);
				/*ADD THE NEW LITERAL IF MISSING. SET SEMANTIC VALUE*/
				s = get_symbol_by_name(symbol_table, yytext);
				if(s == NULL)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, LITERAL);
				}
				else if(is_LITERAL(s) == 0)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, LITERAL);
				}
				else
				{
					yylval = (int) s->id;
				}

				return DOUBLE_QUOTED_LITERAL;
			}

\'([^'\n]|\\n|\\t|\\\\|\\a|\\v|\\\?|\\b|\\f|\\r|\\\"|\\0|\\\')\'	{
				symbol_list_entry *s = NULL;

				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found SINGLE_QUOTED_LITERAL");

				remove_quotes(yytext);	

				/*ADD THE NEW LITERAL IF MISSING. SET SEMANTIC VALUE*/
				s = get_symbol_by_name(symbol_table, yytext);
				if(s == NULL)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, LITERAL);
				}
				else if(is_LITERAL(s) == 0)
				{
					yylval = (int) insert_symbol(symbol_table, yytext, LITERAL);
				}
				else
				{
					yylval = (int) s->id;
				}

				return SINGLE_QUOTED_LITERAL;
			}

[<]{VALID}*[>]		{
				/*EAT SEMANTIC VALUE TYPE IDENTIFIERS (FOUND IN TOKEN DECLARATIONS)*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found SEMANTIC VALUE TYPE IDENTIFIER");
			}

[[:digit:]]+		{
				/*EAT DECIMAL AND HEX NUMBERS*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found NUMBER");
			}

[[:digit:]a-f]+h	{
				/*EAT DECIMAL AND HEX NUMBERS*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITH("found HEX_NUMBER");
			}

;			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found SEMICOLON");
				return (unsigned char)';';
			}

:			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found COLON");
				return (unsigned char)':';
			}

[|]			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("found VERTICAL_BAR");
				return (unsigned char)'|';
			}

[{]			{
				brack_nesting++;
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("entering BRACKETS...");
				BEGIN(in_curly_brackets);
			}

<in_curly_brackets>\".*\"	/*JUST EAT THIS TO AVOID TO CONSIDER QUOTED BRACKETS*/

<in_curly_brackets>\'.\'	/*JUST EAT THIS TO AVOID TO CONSIDER QUOTED BRACKETS*/

<in_curly_brackets>[/][*]	{
				/*TO AVOID CONSIDERING BRACKETS IN COMMENTS*/
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("entering COMMENT in BRACKETS...");
				BEGIN(in_comment);
			}
<in_curly_brackets>[A-Z]+=.+[}] {
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT(yytext);
                brack_nesting--;
                BEGIN(0);
                return SET_SYM_VAL; 
    } 	/*EAT ANYTHING IN CURLY BRACKETS EXCEPT BRACKETS*/

<in_curly_brackets>[{]	{
				brack_nesting++;
			}
<in_curly_brackets><<EOF>>	{
				strcpy(yytext, "EOF");
				ERROR("expecting }, found");
			}
<in_curly_brackets>[}] 	{
				brack_nesting--;			
				if(brack_nesting == 0)
				{
					if(must_print_message(G_SCANNER))
						MESSAGE_WITHOUT("exiting BRACKETS");
					BEGIN(0);
				}
			}

"//"			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("in INLINE COMMENT...");
				BEGIN(in_inline_comment);
			}
<in_inline_comment>\n	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("out OF INLINE COMMENT");
				BEGIN(0);
			}
<in_inline_comment>.	/*EAT EVERYTHING ELSE*/

[/][*]			{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("entering COMMENT...");
				BEGIN(in_comment);
			}

<in_comment>[^*]	/*EAT ANYTHING IN COMMENT EXCEPT '*' */

<in_comment><<EOF>>	{
				strcpy(yytext, "EOF");
				ERROR("expecting */, found");
			}

<in_comment>[*]		{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("exiting COMMENT...");
				BEGIN(exiting_comment);
			}

<exiting_comment>[^/*]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("back in COMMENT...");
				BEGIN(in_comment);
			}

<exiting_comment>[*]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("exiting COMMENT...");
			}

<exiting_comment><<EOF>>	{
				strcpy(yytext, "EOF");
				yyleng = 4;
				ERROR("expecting */, found");
			}

<exiting_comment>[/]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("out of COMMENT");
				/*IF COMMENT WAS IN BRACKETS WE CAN'T */
				/*GET BACK TO INITIAL STATE (0)       */
				if(brack_nesting != 0)
				{
					BEGIN(in_curly_brackets);
				}
				else if(prologue != 0)
				{
					BEGIN(in_prologue);
				}
				else
				{
					BEGIN(0);
				}
			}

"%{"			{
				prologue++;
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("in PROLOGUE...");
				BEGIN(in_prologue);
			}

<in_prologue>[/][*]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("entering COMMENT in PROLOGUE...");
				BEGIN(in_comment);
			}
<in_prologue>[^%]	/*EAT ANYTHING IN PROLOGUE EXCEPT '%' */	

<in_prologue>[%]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("exiting PROLOGUE...");
				BEGIN(exiting_prologue);
			}

<exiting_prologue>[^}/]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("back in PROLOGUE...");
				BEGIN(in_prologue);
			}
<exiting_prologue>[/][*]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("entering COMMENT in PROLOGUE...");
				BEGIN(in_comment);
			}
<exiting_prologue>[}]	{
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("out of PROLOGUE");
				prologue--;
				BEGIN(0);
			}
<exiting_prologue><<EOF>>	{
				strcpy(yytext, "EOF");
				yyleng = 4;
				ERROR("expecting *}, found");
			}

[ \t\n]+				/*EAT WHITESPACE*/

\"[^"]*\n		{
				strcpy(yytext, "new line");
				ERROR("expecting \", found");
			}
\'[\n\t]\'		{
				strcpy(yytext, "tab or newline");
				ERROR("expecting character, found");
			}

\'[^']{2,}\'		{
				ERROR("found multi-character character literal");
			}

\"\"			{
				ERROR("found empty string literal");
			}

\'\'			{
				ERROR("found empty character literal");
			}

.			{
				ERROR("unrecognized character");
			}

<<EOF>>			{
				strcpy(yytext, "EOF");
				if(must_print_message(G_SCANNER))
					MESSAGE_WITHOUT("Reached EOF");
				return 0;
			}

%%
