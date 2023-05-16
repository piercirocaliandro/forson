/*
grow.c -- implementation of recursive descent and Purdom algorithms
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

extern FILE *output_stream, *message_stream;
extern short int no_spaces_flag;


/*NAVIGATES THE GRAMMAR TREE RECURSIVELY TO OBTAIN THE SHORTEST SENTENCE */
/*WHICH DERIVES FROM NON-TERMINAL SYMBOL sle                             */
/*CALLS TEXT GENERATION FUNCTIONS FOR TERMINALS AND BLANKS               */

/*THIS FUNCTION IS CURRENTLY NOT USED*/
void
grow_shortest(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	int i=0;
	symbol_list_entry *s = NULL;
	rule_list_entry *r = NULL;

	assert(sle != NULL);
	assert(is_NT(sle) == 1);	
	
	/*OPTIONAL MESSAGE PRINTING FOR EXECUTION TRACING*/
	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "called grow_shortest for: \"%s\" visited: %d\n", sle->name, sle->visited);
	}

	r = get_shortest_rle(sle, symbol_table);

	assert(r != NULL);
	r->visited++;
	
	for(i=0; i < r->length; i++)
	{
		s = get_symbol(symbol_table, extract_symbol_rle(r,i));
		assert(s != NULL);
		if(is_NT(s) == 1)
		{
			grow_shortest(s, symbol_table);
		}
		else
		{
			generate_terminal_text(s);

			if(no_spaces_flag == 0)
				generate_blank_text();
		}
	}

	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "returning from Grow for: \"%s\"\n", sle->name);
	}
}


/*IMPLEMENTATION OF THE GROW ALGORITHM. */
/*GENERATES A SINGLE SINTACTICALLY VALID SENTENCE OF THE TARGET GRAMMAR */
void
grow(symbol_id starting_symbol, symbol_list_entry *symbol_table)
{
	stack *st;
	symbol_id current = (symbol_id) 0;
	
	/*OPTIONAL MESSAGE PRINTING FOR EXECUTION TRACING*/
	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "called Grow for starting symbol\n");
	}

	assert(starting_symbol != (symbol_id) 0);

	st = initialize_new_stack();

	push(st, starting_symbol);
	current = pop(st);

	int added_rules = 0;
	while(current != 0)
	{	
		symbol_list_entry *sle = NULL;
		rule_list_entry *rle = NULL;

		/*OPTIONAL MESSAGE PRINTING FOR EXECUTION TRACING*/
		if(must_print_message(GENERATION))
		{
			fprintf(message_stream, "Stack size: %d, element popped: %d\n", get_size(st), (int) current);
		}

		sle = get_symbol(symbol_table, current);
		assert(sle != NULL);

		if(is_NT(sle) == 1)
		{
			sle->visited--;
			if(added_rules < GENERATION_THRESHOLD){
				rle = get_random_rle(sle);
				added_rules++;
			}
			else
				rle = get_terminal_rle(sle);
			
			assert(rle != NULL);
			push_rule_on_stack(st, rle, symbol_table);
			
		}
		else
		{
			generate_terminal_text(sle);

			if(no_spaces_flag == 0){
				generate_blank_text();
			}
		}

		current = pop(st);
	}
	printf("\nNumber of pushed rules: %d\n", added_rules);
	clean_stack(st);
}


/*PUSH ALL SYMBOLS IN RULE rle IN STACK st, FROM RIGHT TO LEFT*/
void
push_rule_on_stack(stack *st, rule_list_entry *rle, symbol_list_entry *symbol_table)
{
	int i;

	assert(st != NULL);
	assert(rle != NULL);

	for(i = rle->length-1; i >= 0; i--)
	{
		symbol_id s = (symbol_id) 0;
		symbol_list_entry *sle = NULL;

		s = extract_symbol_rle(rle,i);
		assert(s != (symbol_id) 0);

		sle = get_symbol(symbol_table, s);
		assert(sle != NULL);

		if(is_NT(sle) == 1)
		{
			sle->visited++;
		}

		push(st, s);
	}
}


/*IMPLEMENTATION OF THE PURDOM ALGORITHM                                */
/*GENERATES A SINGLE SINTACTICALLY VALID SENTENCE IN THE TARGET GRAMMAR */
/*IN A SET OF SENTENCES WHICH COVER ALL RULES OF THE TARGET GRAMMAR     */
void
purdom(symbol_id starting_symbol, symbol_list_entry *symbol_table)
{
	stack *st;
	symbol_id current = (symbol_id) 0;
	
	/*OPTIONAL MESSAGE PRINTING FOR EXECUTION TRACING*/
	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "called Purdom for starting symbol\n");
	}

	assert(starting_symbol != (symbol_id) 0);

	st = initialize_new_stack();

	push(st, starting_symbol);
	
	current = pop(st);
	while(current != 0)
	{	
		symbol_list_entry *sle = NULL;
		rule_list_entry *rle = NULL;

		/*OPTIONAL MESSAGE PRINTING FOR EXECUTION TRACING*/
		if(must_print_message(GENERATION))
		{
			fprintf(message_stream, "Stack size: %d, element popped: %d\n", get_size(st), (int) current);
		}

		sle = get_symbol(symbol_table, current);
		assert(sle != NULL);

		if(is_NT(sle) == 1)
		{
			rle = choose(sle, symbol_table);
			assert(rle != NULL);

			rle->visited++;
			sle->visited--;

			push_rule_on_stack(st, rle, symbol_table);
		}
		else
		{
			generate_terminal_text(sle);

			if(no_spaces_flag == 0)
				generate_blank_text();
		}

		current = pop(st);
	}

	clean_stack(st);
}



/*IMPLEMENTATION OF 'CHOOSE' ALGORITHM            */
/*RETURNS THE RULE TO BE USED, NEVER RETURNS NULL */
rule_list_entry 
*choose(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	rule_list_entry *r = NULL;
	
	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "called Choose for symbol: %s (%d)...\n", sle->name, sle->id);
	}

	assert(sle != NULL);
	assert(symbol_table != NULL);
	assert(is_NT(sle));

	/*IF THE SYMBOL HAS ONLY ONE RULE, RETURN IT */
	/*SMALL PERFORMANCE HACK                     */
	if(sle->rulecount == 1)
	{
		r = get_rle(sle, 1);
		assert(r != NULL);

		if(must_print_message(GENERATION))
		{
			fprintf(message_stream, "...Choose returning the only rule (address): %p\n", r);
		}
		return r;
	}

	/*TRY TO FIND AN UNVISITED RULE*/
	r = get_unvisited_rle(sle);

	/*IF IT HASN'T BEEN FOUND, TRY TO FIND A RULE               */
	/*WHICH EVENTUALLY PRODUCES A SYMBOL WITH AN UNVISITED RULE */
	if(r == NULL)
	{
		r = get_with_deep_unvisited_rle(sle, symbol_table);
	}

	/*IF IT HASN'T BEEN FOUND, USE THE RULE WHICH PRODUCES      */
	/*THE SHORTEST SENTENCE IN TERMS OF NUMBER OF LEXICAL UNITS */
	if(r == NULL)
	{
		r = get_shortest_rle(sle, symbol_table);
	}

	/*WE SURELY HAVE ASSIGNED r NOW, SO RETURN IT*/
	assert(r != NULL);
	if(must_print_message(GENERATION))
	{
		fprintf(message_stream, "...Choose returning rule (address): %p\n", r);
	}
	return r;
}


/*FINDS AN UNVISITED RLE FROM SYMBOL sle AND RETURN IT */
/*RETURNS NULL IF ALL RLE IN SYMBOL HAVE BEEN VISITED  */
rule_list_entry *
get_unvisited_rle(symbol_list_entry *sle)
{
	int i;
	
	assert(sle != NULL);
	assert(is_NT(sle) == 1);
	assert(sle->rulecount != 0);

	/*LOOK FOR THE FIRST UNVISITED RULE */
	for(i=1; i <= sle->rulecount; i++)
	{
		rule_list_entry *rle;
		
		rle = get_rle(sle, i);
		assert(rle != NULL);

		if(rle->visited == 0)
		{
			return rle;
		}
	}
	return NULL;
}


/*FINDS "rle" IN sle WHICH DERIVES A SYMBOL NOT IN THE STACK*/
/*WHICH HAS AN UNVISITED "rle"                              */
/*RETURNS NULL IF IT FAILS TO FIND ONE                      */
rule_list_entry *
get_with_deep_unvisited_rle(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	int j, save_visited_value;	

	assert(sle != NULL);
	assert(symbol_table != NULL);
	assert(is_NT(sle) != 0);

	/*MARK THE SYMBOL AS VISITED TO SIMULATE PRESENCE ON THE STACK    */
	/*THE OLD VALUE SHOULD BE KEPT AND REASSIGNED BEFORE RETURN. THIS */
	/*IS BECAUSE THE FUNCTION IS CALLED BY choose() WHICH IS CALLED   */
	/*BY purdom(), WHICH USES THE 'visited' FIELD AS A COUNTER        */
	save_visited_value = sle->visited;
	sle->visited = 1;

	/*FOR EVERY RULE IN SYMBOL sle ...*/
	for(j = 1; j <= sle->rulecount; j++)
	{
		int i;
		rule_list_entry *rle_to_check = NULL;
		
		rle_to_check = get_rle(sle, j);
		assert(rle_to_check != NULL);	

		/*FOR EVERY SYMBOL IN RULE rle_to_check...*/
		for(i=0; i < rle_to_check->length; i++)
		{
			symbol_id sle_to_check_id = 0;
			symbol_list_entry *sle_to_check = NULL;
			rule_list_entry *check_result = NULL;

			sle_to_check_id = extract_symbol_rle(rle_to_check, i);
			assert(sle_to_check_id != 0);
			sle_to_check = get_symbol(symbol_table, sle_to_check_id);
			assert(sle_to_check != NULL);

			/*WE DON'T CARE ABOUT TERMINAL SYMBOLS*/
			if(is_NT(sle_to_check) == 0)
			{
				continue;
			}
			/*NOR SYMBOLS ON THE STAACK*/
			if(sle_to_check->visited != 0)
			{
				continue;
			}
			/*SEE IF THE SYMBOL HAS AN UNVISITED RULE, IN CASE RETURN IT*/
			check_result = get_unvisited_rle(sle_to_check);
			if(check_result != NULL)
			{
				sle->visited = save_visited_value;
				return rle_to_check;
			}
		}

		/*SO sle DOES NOT HAVE RULES WHICH DERIVE SYMBOLS  */
		/*CONTAINING UNVISITED RULES.                      */
		/*WE SHOULD CHECK IF ANY OF THE NT SYMBOLS IN SOME */
		/*RULE OF Sle DO. RECRSION OCCURS                  */
		for(i=0; i < rle_to_check->length; i++)
		{
			symbol_id sle_to_check_id = 0;
			symbol_list_entry *sle_to_check = NULL;
			rule_list_entry *check_result = NULL;

			sle_to_check_id = extract_symbol_rle(rle_to_check, i);
			assert(sle_to_check_id != 0);
			sle_to_check = get_symbol(symbol_table, sle_to_check_id);
			assert(sle_to_check != NULL);

			if(is_NT(sle_to_check) == 0)
			{
				continue;
			}
			if(sle_to_check->visited != 0)
			{
				continue;
			}

			check_result = get_with_deep_unvisited_rle(sle_to_check, symbol_table);
			if(check_result != NULL)
			{
				sle->visited = save_visited_value;
				return rle_to_check;
			}
		}
	}

	/*WE HAD NO LUCK SO RETURN NULL*/
	sle->visited = save_visited_value;
	return NULL;
}


/*GETS A RANDOM RLE FROM SYMBOL sle*/
rule_list_entry *
get_random_rle(symbol_list_entry *sle)
{
	unsigned int rand_num = 0;
	int i = 0;

	assert(sle != NULL);
	assert(is_NT(sle) == 1);
	assert(sle->rulecount != 0);
	
	/*GET A RANDOM UNSIGNED INTEGER*/
	rand_num = (unsigned int) (random() % UINT_MAX);

	/*THE 'rle' LIST STRUCTURE OF sle CONTAINES THE NUMERICAL */
	/*REPARTITION FUNCTION OF THE PROBABILITY DISTRIBUTION OF */
	/*THE RULES                                               */
	for(i=1; i <= sle->rulecount; i++)
	{
		rule_list_entry *rle = NULL;

		rle = get_rle(sle, i);
		assert(rle != NULL);

		if(rand_num <= rle->probability)
		{
			return rle;
		}
	}
	/*THIS SHOULD NOT HAPPEN*/
	return NULL;
}

/*GETS A RULE THAT EXPANDS IN ATERMINAL SYMBOL*/
rule_list_entry *
get_terminal_rle(symbol_list_entry *sle){
	int i;

	assert(sle != NULL);
	assert(is_NT(sle) == 1);
	assert(sle->rulecount != 0);

	for(i = 1; i <= sle->rulecount; i++){
		rule_list_entry *rle = get_rle(sle, i);
		assert (rle != NULL);

		if (rle->type == TERMINAL){
			return rle;
		}
	}

	return NULL;
}


/*RETURNS THE MINIMAL LENGTH OF SENTENCES DERIVING FROM rle*/
/*RETURNS A NEGATIVE VALUE IF A SYMBOL IN rle IS FOUND TO BE ON THE STACK*/
/*THIS IS BECAUSE A RECURSIVE RULE SURELY DOES NOT DERIVE THE SORTEST SENTENCE*/
int
rle_minimal_length(rule_list_entry *rle, symbol_list_entry *symbol_table)
{
	int sum = 0, i;

	assert(rle != NULL);
	assert(symbol_table != NULL);

	/*FOR ALL SYMBOLS IN RULE rle...*/
	for(i = 0; i < rle->length; i++)
	{
		symbol_list_entry *s = NULL;
		symbol_id s_id = 0;

		s_id = extract_symbol_rle(rle, i);
		assert(s_id != 0);
		s = get_symbol(symbol_table, s_id);
		assert(s != NULL);

		if(s->visited == -1)
		{
			return -1;
		}

		/*SUM UP ALL THE MINIMAL LENGTHS OF SYMBOLS CONTAINED BY rle */
		sum += symbol_minimal_length(s, symbol_table);
	}
	return sum;
}


/*RETURNS THE MINIMAL LENGTH OF SENTENCES DERIVING FROM sle*/
int
symbol_minimal_length(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	int i;
	int shortest_value = INT_MAX, save_visited_value;

	assert(sle != NULL);
	assert(symbol_table != NULL);

	/*BASIC CASE OF RECURSION: TERMINAL SYMBOLS PRODUCE ONE LEXICAL ITEM */
	if(is_NT(sle) == 0)
	{
		return 1;
	}
	
	/*MARK sle TO SIMULATE 'STACK PRESENCE' CHECKING*/
	/*FUNCTION WILL UNMARK sle BEFORE RETURNING     */
	save_visited_value = sle->visited;
	sle->visited = -1;

	/*FOR EVERY RULE IN sle...*/
	for(i = 1; i <= sle->rulecount; i++)
	{
		rule_list_entry *r = NULL;
		int rml = INT_MAX;

		r = get_rle(sle, i);
		assert(r != NULL);

		/*GET IT'S MINIMAL LENGTH SENTENCE LENGTH...*/
		rml = rle_minimal_length(r, symbol_table);
		
		/*FIND THE MINIMUM POSITIVE VALUE AND RETURN IT*/
		if(rml >= 0)
		{
			if(rml < shortest_value)
			{
				shortest_value = rml;
			}
		}
		else
			continue;
	}

	sle->visited = save_visited_value;
	return shortest_value;
}


/*RETURNS THE "rle" IN sle WHICH DERIVES THE SHORTEST SENTENCE*/
rule_list_entry *
get_shortest_rle(symbol_list_entry *sle, symbol_list_entry *symbol_table)
{
	int i;
	int shortest_value = INT_MAX, save_visited_value;
	rule_list_entry *shortest_rle = NULL;

	assert(sle != NULL);
	assert(symbol_table != NULL);
	assert(is_NT(sle) != 0);

	/*IF 'SHORTEST' RULE ALREADY CHOSEN AND STORED, RETURN IT'S ADDRESS */
	/*(PERFORMANCE HACK)*/
	if(sle->shortest != NULL)
	{
		return sle->shortest;
	}

	/*MARK sle TO SIMULATE 'STACK PRESENCE' CHECKING */
	/*FUNCTION WILL UNMARK sle BEFORE RETURNING      */
	save_visited_value = sle->visited;
	sle->visited = -1;

	for(i = 1; i <= sle->rulecount; i++)
	{
		rule_list_entry *r = NULL;
		int rml = INT_MAX;

		r = get_rle(sle, i);
		assert(r != NULL);

		rml = rle_minimal_length(r, symbol_table);
		
		if(rml >= 0)
		{
			if(rml < shortest_value)
			{
				shortest_rle = r;
				shortest_value = rml;
			}
		}
		else
			continue;
	}
	assert(shortest_rle != NULL);

	/*STORE CALCULATED VALUE IN sle*/
	sle->shortest = shortest_rle;

	sle->visited = save_visited_value;
	return shortest_rle;
}


/*OUTPUT THE TEXT ASSOCIATED WITH A TERMINAL SYMBOL (LITERAL OR LEXICAL)*/
void
generate_terminal_text(symbol_list_entry *s)
{
	assert(s != NULL);

	/*LITERALS ARE ASSOCIATED WITH THEIR OWN NAME IN THE SYMBOL TABLE*/
	if (is_LITERAL(s))
	{
		print_string(s->name);
	}
	/*LEXICALS ARE ASSOCIATED WITH A RANDOM ELEMENT OF THEIR LEXICON argz STRUCTURE*/
	else if(is_LEXICAL(s))
	{
		int num = 0;

		num = get_lexicon_numerosity(s);

		if(num != 0)
		{
			int pos;
			char *point;

			lexicon_argz_structure *lazs = NULL;

			lazs = (lexicon_argz_structure *) s->rules;
			assert(lazs != NULL);

			pos = ((int)(random() % num));
			point = lazs->argz;
			/*NAVIGATE THE argz STRUCTURE TILL THE */
			/*RANDOMLY SELECTED ELEMENT IS FOUND   */
			while(pos-- > 0)
			{
				point = argz_next(lazs->argz, lazs->argz_size, point);
			}

			/*PASS THE POINTER TO PRINTING FUNCTION*/
			print_string(point);
		}

		/*IN CASE NO STRUCTURE IS DEFINED (OR STRUCTURE EMPTY), NAME IS USED*/
		else
			print_string(s->name);
	}
	else if(is_UNDEFINED(s))
	{
		/*UNDEFINED SYMBOL IS REACHABLE! ERROR!!*/
		assert(0);
		abort();
	}
}


/*OUTPUT STRING POINTED BY POINT (NULL TERMINATED), CONSIDERING ESCAPE SEQUENCES*/
void
print_string(char *point)
{
	char c;
	char *restart;
	int escape = 0;

	assert(point != NULL);

	c = *point;
	restart = point;

	while(c != '\0')
	{
		/*ESCAPING CODE FOR STANDARD C ESCAPE SEQUENCES*/
		if(escape == 1)
		{
			if(c == 'x')
			{
				long int ch;

				ch = strtol(point + sizeof(char), &restart, 16);

				putc((unsigned char) ch, output_stream);
				point = restart;
			}
			else if(isdigit(c))
			{
				long int ch;

				ch = strtol(point, &restart, 8);

				putc((unsigned char) ch, output_stream);
				point = restart;
			}
			else
			{
				putc(get_escaped_char(c), output_stream);
				escape = 0;
				point += sizeof(char);
			}
		}
		else
		{
			if(c == '\\')
			{
				escape = 1;
			}
			else
			{
				fputc(c, output_stream);
			}

			point += sizeof(char);
		}

		c = *point;
	}

	/*A TRAILING '\', ALONE, SHOULD BE PRINTED*/
	if(escape == 1)
	{
		fputc('\\', output_stream);
	}
}


/*RETURNS THE CORRECT CHAR FOR ESCAPE SEQUENCE "/c"*/
char
get_escaped_char(char c)
{
	assert(c != '\0');

	switch(c)
	{
		case '\'':
			return '\'';
		case '"' :
			return '"';
		case '\\':
			return '\\';
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 'a':
			return '\a';
		case 'f':
			return '\f';
		case 'b':
			return '\b';
		case 'r':
			return '\r';
		case 'v':
			return '\v';
		case '0':
			return '\0';
		default:
			return c;
	}
}


/*GENERATES RANDOM SPACES, TABS AND NEWLINES ACCORDING TO TUNABLE PARAMETERS*/
void
generate_blank_text()
{
	unsigned short choose = 0, longer = 0;

	longer = ((int)(random() % 100))+1;
	choose = ((int)(random() % 100))+1;

	if(longer <= MORE_BLANKS_PERCENTAGE)	
		generate_blank_text();

	if(choose <= NEWLINE_PROBABILITY_PERCENTAGE)
	{
		fputs("\n", output_stream);
	}
	else if(choose <= TAB_PROBABILITY_PERCENTAGE + NEWLINE_PROBABILITY_PERCENTAGE)
	{
		fputs("\t", output_stream);
	}
	else
	{
		unsigned short how_many;
		
		how_many = ((int)(random() % MAX_SPACES))+1;
		while(how_many-- > 0)
			fputs(" ", output_stream);
	}
}
