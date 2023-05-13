/*
lexicon.lex -- flex scanner for lexicon files
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

/*-----------------------------------------*/
/*FLEX SCANNER DEFINITION FOR LEXICON FILES*/
/*-----------------------------------------*/

#include <generation.h>

/*SHORTHAND MACROS*/
#define MESSAGE_WITH(M) fprintf(message_stream,"%s: line %d: %s: \"%s\"\n",input_lexicon_file_path,yy_lexiconlineno,M,yy_lexicontext)
#define MESSAGE_WITHOUT(M) fprintf(message_stream,"%s: %s\n",input_lexicon_file_path,M)
#define remove_quotes(M) M+=sizeof(char);M[yy_lexiconleng-2]='\0'; yy_lexiconleng-=2


/*GLOBALS DEFINED IN CALLING FUNCTION*/
extern char *input_lexicon_file_path;
extern symbol_id current_symbol_for_lexicon;
extern FILE *message_stream;
extern symbol_list_entry *symbol_table;

%}

%option prefix="yy_lexicon"
%option yylineno
%option noyywrap

%x in_association

%%

[a-zA-Z][[:alnum:]._]*	{
				symbol_list_entry *sle = NULL;

				if(must_print_message(L_SCANNER))
					MESSAGE_WITH("found IDENTIFIER, searching in symbol table");
				
				sle = get_symbol_by_name(symbol_table, yy_lexicontext);

				if(sle == NULL)
				{
					if(must_print_message(WARNING))
						fprintf(message_stream, "warning: %s: line %d: symbol undefined, lexicon will be discarded: \"%s\"\n", input_lexicon_file_path, yy_lexiconlineno, yy_lexicontext);
					current_symbol_for_lexicon = -1;
				}
				else if(is_LEXICAL(sle) == 0)
				{
					if(must_print_message(L_SCANNER))
						fprintf(message_stream, "warning: %s: symbol is not lexical, associated lexicon will be discarded: \"%s\"\n", input_lexicon_file_path, yy_lexicontext);
					current_symbol_for_lexicon = -1;
				}
				else
				{
					if(must_print_message(L_SCANNER))
						MESSAGE_WITHOUT("symbol found, waiting for lexicon...");
					current_symbol_for_lexicon = sle->id;
					assert(current_symbol_for_lexicon != 0);
				}
				BEGIN(in_association);
			}

<in_association>[a-zA-Z][[:alnum:]._]*	{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITH("found IDENTIFIER, expecting LEXICAL UNIT or SEMICOLON");
				return UNEXPECTED_IDENTIFIER;
			}

;			{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITHOUT("found SEMICOLON, expected IDENTIFIER");
				return UNEXPECTED_SEMICOLON;
			}

<in_association>;	{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITHOUT("found SEMICOLON, waiting for new IDENTIFIER");
				BEGIN(0);
			}

\"([^"\n]|\\\")+\"		{
				remove_quotes(yy_lexicontext);
				if(must_print_message(L_SCANNER))
					MESSAGE_WITH("found LEXICAL UNIT, expecting IDENTIFIER");
				return UNEXPECTED_LEXICAL_UNIT;
			}

<in_association>\"([^"\n]|\\\")+\"	{
				remove_quotes(yy_lexicontext);
				if(must_print_message(L_SCANNER))
					MESSAGE_WITH("found LEXICAL UNIT");
				
				return ASSOCIATED_LEXICAL_UNIT;
			}

<in_association><<EOF>>	{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITHOUT("found END OF FILE, expecting SEMICOLON");
				return UNEXPECTED_EOF;
			}

<<EOF>>			{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITHOUT("reached end of file, no errors");
				return END_OF_FILE;
			}

<*>\"\"			{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITHOUT("found empty string literal");
				return EMPTY_STRING_LITERAL;
			}

<*>[ \t\n]+		{
				/*EAT WHITESPACE*/
			}

.			{
				if(must_print_message(L_SCANNER))
					MESSAGE_WITH("found unexpected character");
				return UNEXPECTED_CHARACTER;
			}

%%
