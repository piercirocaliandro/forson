/*
print_tables.c -- prints formatted internal data structures
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


#include <generation.h>

extern FILE *message_stream, *output_stream;
extern symbol_list_entry *symbol_table;
extern int rc_values[];
extern symbol_id starting_symbol;
extern int verbosity;
extern char *rule_type_names[];


/*CYCLE THROUGH ALL SYMBOLS IN TABLE l AND PRINT THE CORRECT DESCRIPTIVE INFORMATION*/
void
print_symbol_list(symbol_list_entry *l)
{
	int i=0, symbol_count=0, old_verbosity=0;
	symbol_list_entry *sle=NULL;	
	
	assert(l != NULL);

	/*SUPPRESSES ALL MESSAGES TO AVOID MESSING UP TABLE PRINTING*/
	if(TABLE_STREAM == message_stream)
	{
		old_verbosity = verbosity; 
		verbosity = 0;
	}

	/*HEAD NODE STORES SYMBOL COUNT IN THE rulecount FIELD*/
	symbol_count = l->rulecount;

	fprintf(TABLE_STREAM, "Printing Symbol Table: %d symbols defined:\n", l->rulecount);
	if(symbol_count == 0)
	{
		fprintf(TABLE_STREAM, "EMPTY SYMBOL TABLE\n");
		return;
	}

	for(i=1; i<= symbol_count; i++)
	{
		char type[32];

		sle=get_symbol(l,i);
		assert(sle != NULL);

		/*FIND SYMBOL TYPE NAME AND PLACE IT IN type BUFFER*/
		if(is_NT(sle))		
			strcpy(type, "non-terminal");
		else if(is_LITERAL(sle))
			strcpy(type, "literal");
		else if(is_RANDOM_LEXICAL(sle))
			strcpy(type, "random lexical");
		else if(is_LEXICAL(sle))
			strcpy(type, "lexical");
		else
			strcpy(type, "-unknown-");
		if(i == starting_symbol)
		{
			char *str = " (STARTING SYMBOL)";
			strcat(type, str);
		}


		/*PRINT A LINE OF INFO FOR EVERY SYMBOL*/
		fprintf(TABLE_STREAM, "ID: %-6d ", sle->id);
		fprintf(TABLE_STREAM, "NAME: %-30s ", sle->name);
		fprintf(TABLE_STREAM, "TYPE: %-15s ", type);
		fprintf(TABLE_STREAM, "\n");


		/*IN CASE OF NON-TERMINALS, PRINT TRAILING RULE LIST TABLE*/
		if(is_NT(sle))
		{
			print_rule_list(sle);
		}
	}

	/*RESTORES ORIGINAL VERBOSITY LEVEL*/
	if(TABLE_STREAM == message_stream)
		verbosity = old_verbosity; 
}


/*PRINT ALL RULES OF A SINGLE NON-TERMINAL SYMBOL*/
void
print_rule_list(symbol_list_entry *l)
{
	int i;
	float f = 0.0, f_old = 0.0;
	rule_list_entry *rle = NULL;
	
	assert (l != NULL);
	assert (is_NT(l) == 1);

	/*SYMBOL IS NT BUT HAS NO RULES (ERROR CONDITION)*/
	if(l->rules == NULL)
	{	
		/*THIS SHOULD NOT HAPPEN*/
		abort();
	}

	for(i=1; i <= l->rulecount; i++)
	{
		rle = get_rle(l, i);

		assert(rle != NULL);
		assert(rle->type >= 0);
		assert(rle->type < NUMBER_OF_RULE_TYPES);

		/*PRINT A LINE OF INFO FOR EVERY RULE*/
		fprintf(TABLE_STREAM, "\t");
		fprintf(TABLE_STREAM, "rule: %-4d ", i);
		/*fprintf(TABLE_STREAM, "address: %-15d ", rle);*/
		/*fprintf(TABLE_STREAM, "lenth %3d ", rle->length);*/
		fprintf(TABLE_STREAM, "type: %-24s ", rule_type_names[rle->type]);

		/*CALCULATE AND PRINT PROBABILITY FOR EVERY RULE*/
		f = ((float)(rle->probability)/RAND_MAX)-f_old;
		f_old += f;
		fprintf(TABLE_STREAM, "probability: %10f :::: ", f);
		
		/*FILL THE REST OF THE LINE WITH INFO ON SYMBOLS IN RULE*/
		if (rle->length == 0)
		{
			fprintf(TABLE_STREAM, "-EMPTY STRING-");
		}
		else
		{
			int j;			

			for(j=0; j < rle->length; j++)
			{
				symbol_list_entry *s = NULL;

				s = get_symbol(symbol_table, extract_symbol_rle(rle,j));
				assert(s != NULL);

				if(is_LITERAL(s) == 1)
					fprintf(TABLE_STREAM, "\"%-s\" ", s->name);
				else
					fprintf(TABLE_STREAM, "%-s ", s->name);
			}
		}

		fprintf(TABLE_STREAM, "\n");
	}
}

/*PRINTS THE LEXICON STRUCTURE CONTENTS FOR A SINGLE TERMINAL SYMBOL*/
void
print_lexicon_table(symbol_list_entry *l)
{
	int i;

	assert(l != NULL);

	fprintf(TABLE_STREAM, "Printing Lexicon Table\n");

	for(i = 1; i <= l->rulecount; i++)
	{
		symbol_list_entry *current_symbol = NULL;

		current_symbol = get_symbol(l, i);
		assert(current_symbol != NULL);

		if(is_LEXICAL(current_symbol) == 1)
		{
			fprintf(TABLE_STREAM, "Symbol %s (ID: %d):\n", current_symbol->name, current_symbol->id);

			if(get_lexicon_numerosity(current_symbol) != 0)
			{
				lexicon_argz_structure *lazs = NULL;
				int index = 0, numerosity = 0;
				char *position = NULL;

				lazs = (lexicon_argz_structure *) current_symbol->rules;
				assert(lazs != NULL);

				numerosity = get_lexicon_numerosity(current_symbol);
				assert(numerosity > 0);

				while(index < numerosity)
				{
					position = argz_next(lazs->argz, lazs->argz_size, position);
					assert(position != NULL);

					fprintf(TABLE_STREAM, "%d:", index+1);
					fprintf(TABLE_STREAM, "\t%s\n", position);

					index++;
				}
			}
			else
			{
				fprintf(TABLE_STREAM, "<NO ENTRIES>\n");
			}
			
		}
	}
	
	fprintf(TABLE_STREAM, "\n");

}
